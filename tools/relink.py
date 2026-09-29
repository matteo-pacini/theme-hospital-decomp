#!/usr/bin/env python3
"""Rebuild the EXE's code object from the units, at the original addresses,
and compare it with the original byte for byte.

Each unit's code sections (read as tools/link.py reads them) are placed
where their functions are in config/functions.txt; relocations are resolved
against original addresses: functions by name, globals by
config/symbols.txt, a unit's own data segments by config/sections.txt.
Absolute references are compared with the EXE's fixup targets, and every
other byte (relative branches included) with the EXE's raw bytes. Bytes of
the code object that no unit places are reported as gaps.

Usage: tools/relink.py [-v]    (after `ninja`)
"""

import collections
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
import omf2elf  # noqa: E402
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from link import unit_model  # noqa: E402
from project import EXE, Project  # noqa: E402

ADDRESS_NAME = re.compile(r"_[CD]_([0-9a-f]{8})")


def main(verbose=False):
    proj, le = Project(), LEFile(EXE)
    code = le.code
    fn_addr = {f.name: f.addr for f in proj.functions}
    data_addr = {s.name: s.addr for s in proj.data_symbols}

    def symbol_address(name):
        m = ADDRESS_NAME.fullmatch(name)
        if m:
            return int(m.group(1), 16)
        return fn_addr.get(name, data_addr.get(name))

    image = bytearray(code.vsize)
    owner = [None] * code.vsize  # unit name per placed byte
    placed = []  # (unit, section, address)
    problems = collections.Counter()
    notes = []

    for u in sorted(proj.units, key=lambda u: u.functions[0].addr):
        sections, symbols, externs, weak = unit_model(u)
        seg_addr = {}  # a unit's own data segments, by name
        for segment, addr, size in proj.unit_data.get(u.source, []) if u.source else []:
            seg_addr.setdefault(segment, addr)
        # A unit's own functions first: statics of the same name exist in
        # several units (radix_value in strtol and scnf).
        own = dict(fn_addr, **{f.name: f.addr for f in u.functions})
        base = {}
        for sec in sections:
            if not sec.code:
                if sec.name in seg_addr:
                    base[id(sec)] = seg_addr[sec.name]
                continue
            known = [own[n] - o for n, (s, o, _) in symbols.items() if s is sec and n in own and not n.endswith(".c")]
            if not known:
                continue  # compiled code a stand-in replaced, or nothing to place
            if len(set(known)) > 1:
                problems["section with inconsistent symbol addresses"] += 1
                notes.append(f"{u.name}: {sec.name} placed at {sorted(set(map(hex, known)))}")
            base[id(sec)] = known[0]
            placed.append((u, sec, known[0], symbols))

        for u_, sec, addr, syms in [p for p in placed if p[0] is u]:
            size = len(sec.data)
            if sec.comdat and owner[addr - code.base] is not None:
                continue  # an inline function another unit placed already
            for k in range(size):
                i = addr - code.base + k
                if 0 <= i < code.vsize:
                    if owner[i] is not None and owner[i] != u.name:
                        problems["bytes placed twice"] += 1
                    image[i] = sec.data[k]
                    owner[i] = u.name
            for at, rtype, target in sec.relocs:
                site = addr + at
                addend = struct.unpack_from("<i", sec.data, at)[0]
                if target[0] == "sec":
                    t = base.get(id(target[1]))
                else:
                    name = target[1]
                    if name in syms and id(syms[name][0]) in base:
                        t = base[id(syms[name][0])] + syms[name][1]
                    elif name in syms and not syms[name][0].code:
                        t = None  # the unit's own data: placed below
                    else:
                        t = symbol_address(name)
                if t is None and target[0] == "sec" or (t is None and name in syms):
                    # A data segment of the unit's own that config/sections.txt
                    # does not place: place it where this first reference
                    # lands; every later reference then checks it.
                    fx = le.fixups.get(site)
                    dsec = target[1] if target[0] == "sec" else syms[name][0]
                    off = 0 if target[0] == "sec" else syms[name][1]
                    if fx is not None and rtype == omf2elf.R_386_32 and not dsec.code:
                        base[id(dsec)] = fx.target - addend - off
                        problems["data segments placed from a reference"] += 1
                        t = base[id(dsec)] + off
                if t is None:
                    problems["relocation target unknown"] += 1
                    if verbose:
                        notes.append(f"{site:#x}: {target[0]} {getattr(target[1], 'name', target[1])} unknown")
                    continue
                value = t + addend - (site if rtype == omf2elf.R_386_PC32 else 0)
                struct.pack_into("<i", image, site - code.base, value)
                if rtype == omf2elf.R_386_32:
                    fx = le.fixups.get(site)
                    if fx is None or fx.kind != SRC_OFFSET32 or fx.target != (value & 0xFFFFFFFF):
                        problems["absolute reference differs"] += 1
                        notes.append(f"{site:#x} ({u.name}): ours {value & 0xFFFFFFFF:#x}, EXE {fx.target if fx else None!s}")
                    # The EXE's raw bytes at a fixup are not the target.
                    image[site - code.base : site - code.base + 4] = code.data[site - code.base : site - code.base + 4]

    diffs, gaps = [], []
    i = 0
    while i < code.vsize:
        if owner[i] is None:
            j = i
            while j < code.vsize and owner[j] is None:
                j += 1
            gaps.append((code.base + i, j - i))
            i = j
            continue
        if image[i] != code.data[i]:
            diffs.append(code.base + i)
        i += 1

    covered = sum(o is not None for o in owner)
    print(f"code object {code.vsize:#x} bytes, placed {covered:#x}, gaps {len(gaps)} ({sum(n for _, n in gaps):#x} bytes)")
    print(f"bytes differing {len(diffs)}")
    for k, v in problems.items():
        print(f"{k}: {v}")
    runs = []
    for a in diffs:
        if runs and a - runs[-1][1] <= 8:
            runs[-1][1] = a
        else:
            runs.append([a, a])
    for lo, hi in runs[: 40 if not verbose else None]:
        f = proj.function_at(lo)
        print(f"  differs {lo:#x}..{hi:#x}  {f.name if f else '(no function)'}")
    def filler(b):
        """Whether b is only alignment no-ops: nop, mov r,r, lea r,[r+0]."""
        i = 0
        while i < len(b):
            op = b[i]
            m = b[i + 1] if i + 1 < len(b) else None
            if op in (0x90, 0x00):
                i += 1
            elif op in (0x89, 0x8B) and m is not None and m >> 6 == 3 and (m >> 3) & 7 == m & 7:
                i += 2
            elif op == 0x8D and m is not None and m >> 6 in (1, 2):
                n = 1 if m >> 6 == 1 else 4
                sib = m & 7 == 4
                end = i + 2 + sib + n
                if b[i + 2 + sib : end] != bytes(n) or end > len(b):
                    return False
                i = end
            else:
                return False
        return True
    nonzero_gaps = [(a, n) for a, n in gaps if not (n < 16 and filler(bytes(code.data[a - code.base : a - code.base + n])))]
    print(f"gaps that are not alignment filler {len(nonzero_gaps)}")
    for a, n in nonzero_gaps[: 20 if not verbose else None]:
        print(f"  gap {a:#x} +{n:#x}: {code.data[a - code.base : a - code.base + min(n, 16)].hex()}")
    for n in notes[: 20 if not verbose else None]:
        print("  " + n)
    return not diffs and not nonzero_gaps and not problems


if __name__ == "__main__":
    sys.exit(0 if main("-v" in sys.argv[1:]) else 1)
