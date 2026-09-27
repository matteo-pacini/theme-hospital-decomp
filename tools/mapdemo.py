#!/usr/bin/env python3
"""Match retail functions to named functions of the March 1997 demo build.

The demo is optimised and the retail game is not, so code is compared by
structure only:

1. anchors: functions that are the only user of a string in both builds;
2. call graphs: callee lists of matched pairs are aligned, and unmatched
   callees between aligned ones are paired when both gaps have equal length;
3. link order: unmatched functions between two consecutive matched pairs
   are aligned in order (the builds link modules in the same order), scoring
   candidate pairs by the struct offsets and constants they use, which the
   optimiser leaves alone, and by the other evidence above.

Writes config/demo_map.txt: retail address, demo address, method, evidence
(confirmed / consistent / conflict), category (game / bflib / runtime),
source file, demo symbol.

Usage: tools/mapdemo.py
"""

import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(__file__))
import demosyms  # noqa: E402
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_EBP, X86_REG_ESP  # noqa: E402
from x86scan import scan  # noqa: E402

MIN_STRING = 4
MIN_SCORE = 0.3

# Retail game modules were built unoptimised; Bullfrog's libraries and the
# runtime were not. A demo function's category must agree with this.
UNOPTIMISED = re.compile(rb"[\x50-\x57]{1,7}\x89\xe5\x81\xec")

_md = Cs(CS_ARCH_X86, CS_MODE_32)
_md.detail = True


class Build:
    """Functions of one executable with the features used for matching."""

    def __init__(self, le, starts_sizes, categories=None):
        self.le = le
        self.category = categories or {}
        self.unoptimised = {
            a: bool(UNOPTIMISED.match(le.code.data, a - le.code.base)) for a, _ in starts_sizes
        }
        self.funcs = sorted(starts_sizes)  # [(addr, size)]
        self.addrs = [a for a, _ in self.funcs]
        self.strings = {}  # addr -> set of strings referenced
        self.calls = {}  # addr -> [callee addr] in code order
        self.consts = {}  # addr -> set of struct offsets and immediates
        for addr, size in self.funcs:
            strs, calls, consts = set(), [], set()
            for it in scan(le, addr, addr + size):
                if it.mnemonic == "call" and it.branch_target is not None:
                    calls.append(it.branch_target)
                if it.mnemonic and it.branch_target is None:
                    consts |= self.constants(it)
                for site in range(it.addr, it.end):
                    fx = le.fixups.get(site)
                    if fx is not None and fx.kind == SRC_OFFSET32 and not le.code.contains(fx.target):
                        s = self.string_at(fx.target)
                        if s:
                            strs.add(s)
            self.strings[addr] = strs
            self.calls[addr] = calls
            self.consts[addr] = consts

    def constants(self, it):
        """Displacements off non-frame registers and immediates of one
        instruction, skipping relocated fields and small values."""
        if any(a in self.le.fixups for a in range(it.addr, it.end)):
            return set()
        ins = next(_md.disasm(self.le.read(it.addr, it.size), it.addr), None)
        if ins is None:
            return set()
        out = set()
        for op in ins.operands:
            if op.type == X86_OP_MEM and op.mem.base not in (0, X86_REG_EBP, X86_REG_ESP) and op.mem.disp > 4:
                out.add(("disp", op.mem.disp))
            elif op.type == X86_OP_IMM and not 0 <= op.imm <= 8:
                out.add(("imm", op.imm & 0xFFFFFFFF))
        return out
    def string_at(self, addr):
        obj = self.le.object_at(addr)
        if obj is None:
            return None
        off = addr - obj.base
        end = obj.data.find(b"\0", off, off + 256)
        if end - off < MIN_STRING:
            return None
        s = obj.data[off:end]
        if all(32 <= c < 127 or c in (9, 10, 13) for c in s):
            return s
        return None


def unique_users(build):
    users = collections.defaultdict(set)
    for addr, strs in build.strings.items():
        for s in strs:
            users[s].add(addr)
    return {s: next(iter(u)) for s, u in users.items() if len(u) == 1}


def evidence(demo, retail, mapping, d, r):
    """'confirmed', 'consistent' or 'conflict' for pairing demo d with retail r."""
    if (demo.category[d] == "game") != retail.unoptimised[r]:
        return "conflict"
    sd, sr = demo.strings[d], retail.strings[r]
    if sd and sr and not sd & sr:
        return "conflict"
    mapped = [mapping[c] for c in dict.fromkeys(demo.calls[d]) if c in mapping]
    hits = sum(c in retail.calls[r] for c in mapped)
    if len(mapped) >= 2 and hits == 0:
        return "conflict"
    if (sd and sd == sr) or (len(mapped) >= 2 and hits == len(mapped)):
        return "confirmed"
    return "consistent"


def pair_gaps(left, right, mapping, rmapped, method, methods, demo, retail):
    """Pair the unmatched entries of two equally long gaps, in order, unless
    any resulting pair contradicts the evidence."""
    left = [x for x in left if x not in mapping]
    right = [x for x in right if x not in rmapped]
    if len(left) != len(right):
        return 0
    if any(evidence(demo, retail, mapping, d, r) == "conflict" for d, r in zip(left, right)):
        return 0
    n = 0
    for d, r in zip(left, right):
        mapping[d] = r
        rmapped[r] = d
        methods[d] = method
        n += 1
    return n


def align_calls(demo, retail, mapping, rmapped, methods):
    added = 0
    for d, r in list(mapping.items()):
        cd = demo.calls.get(d, [])
        cr = retail.calls.get(r, [])
        # Anchor positions where a demo callee is already mapped to the same
        # retail callee, taken in increasing order on both sides.
        anchors = [(-1, -1)]
        j0 = 0
        for i, c in enumerate(cd):
            if c in mapping:
                try:
                    j = cr.index(mapping[c], j0)
                except ValueError:
                    continue
                anchors.append((i, j))
                j0 = j + 1
        anchors.append((len(cd), len(cr)))
        for (i0, j0), (i1, j1) in zip(anchors, anchors[1:]):
            gd = [c for c in cd[i0 + 1 : i1] if c in demo.strings]
            gr = [c for c in cr[j0 + 1 : j1] if c in retail.strings]
            added += pair_gaps(list(dict.fromkeys(gd)), list(dict.fromkeys(gr)), mapping, rmapped, "calls", methods, demo, retail)
    return added


def similarity(demo, retail, mapping, d, r):
    if evidence(demo, retail, mapping, d, r) == "conflict":
        return None
    cd, cr = demo.consts[d], retail.consts[r]
    score = len(cd & cr) / len(cd | cr) if cd or cr else 0.5
    sd, sr = demo.strings[d], retail.strings[r]
    if sd and sr:
        score += len(sd & sr) / len(sd | sr)
    return score


def align_gap(demo, retail, mapping, gd, gr):
    """Order-preserving alignment of two gaps maximising total similarity;
    returns the pairs scoring at least MIN_SCORE."""
    n, m = len(gd), len(gr)
    if not n or not m or n * m > 40000:
        return []
    sim = [[similarity(demo, retail, mapping, d, r) for r in gr] for d in gd]
    best = [[0.0] * (m + 1) for _ in range(n + 1)]
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            take = sim[i][j]
            best[i][j] = max(
                best[i + 1][j],
                best[i][j + 1],
                (take + best[i + 1][j + 1]) if take is not None and take >= MIN_SCORE else -1,
            )
    pairs, i, j = [], 0, 0
    while i < n and j < m:
        take = sim[i][j]
        if take is not None and take >= MIN_SCORE and best[i][j] == take + best[i + 1][j + 1]:
            pairs.append((gd[i], gr[j]))
            i, j = i + 1, j + 1
        elif best[i][j] == best[i + 1][j]:
            i += 1
        else:
            j += 1
    return pairs


def align_order(demo, retail, mapping, rmapped, methods):
    pairs = sorted((r, d) for d, r in mapping.items())
    added = 0
    for (r0, d0), (r1, d1) in zip(pairs, pairs[1:]):
        if d1 <= d0:
            continue
        gd = [a for a in demo.addrs if d0 < a < d1 and a not in mapping]
        gr = [a for a in retail.addrs if r0 < a < r1 and a not in rmapped]
        if len(gd) == len(gr) and pair_gaps(gd, gr, mapping, rmapped, "order", methods, demo, retail):
            added += len(gd)
            continue
        for d, r in align_gap(demo, retail, mapping, gd, gr):
            mapping[d] = r
            rmapped[r] = d
            methods[d] = "align"
            added += 1
    return added


def main():
    proj = Project()
    retail = Build(LEFile(EXE), [(f.addr, f.size) for f in proj.functions])

    _, syms = demosyms.load()
    code = sorted((s for s in syms if s.code and 0x10000 <= s.addr < 0x100000), key=lambda s: s.addr)
    by_addr = {}
    for s in code:
        by_addr.setdefault(s.addr, s)
    starts = sorted(by_addr)
    demo_le = LEFile(demosyms.DEMO_EXE)
    end = demo_le.code.base + demo_le.code.vsize
    demo = Build(
        demo_le,
        [(a, (starts[i + 1] if i + 1 < len(starts) else end) - a) for i, a in enumerate(starts)],
        {a: s.category for a, s in by_addr.items()},
    )

    mapping, rmapped, methods = {}, {}, {}
    main_sym = next(s for s in code if s.name == "main_")
    main_retail = next(f.addr for f in proj.functions if f.addr == main_sym.addr)
    mapping[main_sym.addr] = main_retail
    rmapped[main_retail] = main_sym.addr
    methods[main_sym.addr] = "anchor"

    du, ru = unique_users(demo), unique_users(retail)
    votes = collections.defaultdict(collections.Counter)
    for s, d in du.items():
        if s in ru:
            votes[d][ru[s]] += 1
    for d, c in votes.items():
        (r, n), *rest = c.most_common()
        if (not rest or rest[0][1] < n) and r not in rmapped and evidence(demo, retail, mapping, d, r) != "conflict":
            mapping[d] = r
            rmapped[r] = d
            methods[d] = "strings"
    print(f"string anchors: {len(mapping)}", file=sys.stderr)

    while True:
        added = align_calls(demo, retail, mapping, rmapped, methods)
        print(f"call-graph pass: +{added}", file=sys.stderr)
        if not added:
            break
    for _ in range(3):
        added = align_order(demo, retail, mapping, rmapped, methods)
        print(f"link-order pass: +{added}", file=sys.stderr)
        while True:
            added = align_calls(demo, retail, mapping, rmapped, methods)
            print(f"call-graph pass: +{added}", file=sys.stderr)
            if not added:
                break

    out = os.path.join(ROOT, "config", "demo_map.txt")
    with open(out, "w") as f:
        f.write("# retail  demo  method  evidence  category  source  demo-symbol  (generated by tools/mapdemo.py)\n")
        for d, r in sorted(mapping.items(), key=lambda x: x[1]):
            s = by_addr[d]
            ev = evidence(demo, retail, mapping, d, r)
            f.write(f"{r:08x} {d:08x} {methods[d]:<7} {ev:<10} {s.category:<7} {s.source:<14} {s.name}\n")
    cats = collections.Counter(by_addr[d].category for d in mapping)
    print(f"mapped {len(mapping)} of {len(retail.funcs)} retail functions: {dict(cats)}", file=sys.stderr)


if __name__ == "__main__":
    main()
