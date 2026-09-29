#!/usr/bin/env python3
"""Rebuild the EXE's code object from the units, at the original addresses,
and compare it with the original byte for byte.

Each unit's code sections (read as tools/link.py reads them) are placed
where their functions are in config/functions.txt; relocations are resolved
against original addresses: functions by name, globals by
config/symbols.txt, a unit's own data segments by config/sections.txt.
Absolute references are compared with the EXE's fixup targets, and every
other byte (relative branches included) with the EXE's raw bytes. Bytes of
the code object that no unit places are reported as gaps. The units' own
data is checked against the data object the same way. Finally, the gaps
between functions show where the original objects began and ended (filler
within one object, zeros between objects), which is compared with how the
functions are divided into source files.

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


def check_data(le, placed, symbol_address, problems, notes):
    """Overlay the units' data on the EXE's data object and compare."""
    data = le.objects[2]
    image, owner = bytearray(data.data), [None] * data.vsize
    # Relocated fields of every placed section: their raw bytes are addends.
    reloc_bytes = {addr - data.base + at + j for _, sec, addr, _, _, _ in placed for at, _, _ in sec.relocs for j in range(4)}
    late = []
    queue = list(placed)
    while queue:
        u, sec, addr, syms, base, weak = queue.pop(0)
        size = max(len(sec.data), sec.length)
        if not data.contains(addr) or not data.contains(addr + size - 1):
            problems["data placed outside the data object"] += 1
            notes.append(f"{u.name}: {sec.name} at {addr:#x}")
            continue
        body = bytes(sec.data) + bytes(size - len(sec.data))
        fields = {at + j for at, _, _ in sec.relocs for j in range(4)}
        for k in range(size):
            i = addr - data.base + k
            if owner[i] is not None and owner[i] != u.name and image[i] != body[k] and k not in fields and i not in reloc_bytes:
                problems["data bytes placed twice, differently"] += 1
                if problems["data bytes placed twice, differently"] <= 3:
                    notes.append(f"{addr + k:#x}: {u.name} {sec.name} over {owner[i]}")
            image[i], owner[i] = body[k], u.name
        for at, rtype, target in sec.relocs:
            site = addr + at
            addend = struct.unpack_from("<i", sec.data, at)[0]
            if target[0] == "sec":
                t = base.get(id(target[1]))
            elif target[1] in syms and id(syms[target[1]][0]) in base:
                t = base[id(syms[target[1]][0])] + syms[target[1]][1]
            else:
                t = symbol_address(target[1])
                if t is None and isinstance(weak.get(target[1]), tuple):
                    t = symbol_address(weak[target[1]][0])  # the weak default
            fx = le.fixups.get(site)
            if t is None and target[0] == "sym" and target[1] in syms and id(syms[target[1]][0]) in base:
                t = base[id(syms[target[1]][0])] + syms[target[1]][1]
            if t is None and target[0] == "sym" and target[1] in syms and fx is not None and id(syms[target[1]][0]) not in base:
                # The unit's own data that only its data refers to: place it here.
                tsec, toff, _ = syms[target[1]]
                base[id(tsec)] = fx.target - addend - toff
                late.append((u, tsec, base[id(tsec)], syms, base, weak))
                t = base[id(tsec)] + toff
            if t is None:
                problems["data relocation target unknown"] += 1
                notes.append(f"{site:#x} ({u.name} {sec.name}): target {getattr(target[1], 'name', target[1])} unknown")
            elif fx is None or fx.target != ((t + addend) & 0xFFFFFFFF):
                problems["data reference differs"] += 1
                notes.append(f"{site:#x} ({u.name} {sec.name}): ours {(t + addend) & 0xFFFFFFFF:#x}, EXE {fx.target if fx else None!s}")
            i = site - data.base
            image[i : i + 4] = data.data[i : i + 4]
        queue.extend(late)
        late.clear()
    diffs = [data.base + i for i in range(data.vsize) if owner[i] is not None and image[i] != data.data[i]]
    owned = sum(o is not None for o in owner)
    print(f"data object {data.vsize:#x} bytes, placed by units {owned:#x}, differing {len(diffs)}")
    runs = []
    for a in diffs:
        if runs and a - runs[-1][1] <= 8:
            runs[-1][1] = a
        else:
            runs.append([a, a])
    for lo, hi in runs[:20]:
        print(f"  data differs {lo:#x}..{hi:#x} ({owner[lo - data.base]})")


def module_boundaries(proj, le):
    """Gaps between functions that contradict the units: the original linker
    filled alignment gaps between one object's segments with no-op filler,
    and gaps between objects with zeros. Returns [(kind, gap address, gap
    size, function before, its unit, function after, its unit)] for filler
    between two units and zeros inside one, skipping units whose objects are
    zero-padded throughout (Smacker, some assembly and runtime modules)."""
    unit_of = {f.addr: u for u in proj.units for f in u.functions}
    fs = sorted(proj.functions, key=lambda f: f.addr)
    gaps = []
    for a, b in zip(fs, fs[1:]):
        n = b.addr - a.end
        if 0 < n < 16:
            gaps.append((a, b, n, not any(le.read(a.end, n))))
    inside = collections.defaultdict(list)
    for a, b, n, zero in gaps:
        if unit_of[a.addr] is unit_of[b.addr]:
            inside[unit_of[a.addr].name].append(zero)
    zero_padded = {u for u, z in inside.items() if all(z)}
    out = []
    for a, b, n, zero in gaps:
        ua, ub = unit_of[a.addr], unit_of[b.addr]
        if ua is ub and zero and ua.name not in zero_padded:
            out.append(("zeros inside a unit", a.end, n, a.name, ua.name, b.name, ub.name))
        elif ua is not ub and not zero:
            out.append(("filler between units", a.end, n, a.name, ua.name, b.name, ub.name))
    return out


def main(verbose=False):
    proj, le = Project(), LEFile(EXE)
    code = le.code
    fn_addr = {f.name: f.addr for f in proj.functions}
    data_addr = {s.name: s.addr for s in proj.data_symbols}
    # Names several modules' statics share (.mod_init, ...) place nothing.
    counts = collections.Counter(s.name for s in proj.data_symbols)
    unique_data = {n: a for n, a in data_addr.items() if counts[n] == 1}

    def symbol_address(name):
        m = ADDRESS_NAME.fullmatch(name)
        if m:
            return int(m.group(1), 16)
        return fn_addr.get(name, data_addr.get(name))

    data_placed, unplaced_data, pending = [], [], []
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
                if sec.name in seg_addr and not sec.comdat:
                    base[id(sec)] = seg_addr[sec.name]
                continue
            known = [own[n] - o for n, (s, o, _) in symbols.items() if s is sec and n in own and not n.endswith(".c")]
            # A section with only an address-named label (cstrt386's BEGTEXT).
            known += [int(m.group(1), 16) - o for n, (s, o, _) in symbols.items()
                      if s is sec and (m := ADDRESS_NAME.fullmatch(n))]
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

        # The unit's own data (strings, initialised and uninitialised statics,
        # vtables, exception tables), where a name, config/sections.txt or a
        # code reference places it.
        for sec in sections:
            if sec.code or not max(len(sec.data), sec.length):
                continue
            if id(sec) not in base:
                # Global names only: statics repeat across modules (.mod_init).
                named = [unique_data[n] - o for n, (s, o, local) in symbols.items() if s is sec and n in unique_data and not local]
                if named:
                    base[id(sec)] = named[0]
            if id(sec) in base:
                data_placed.append((u, sec, base[id(sec)], symbols, base, weak))
            else:
                pending.append((u, sec, symbols, base, weak))

    # Code addresses of every placed symbol, for data that points at code.
    code_syms = {}
    for u, sec, addr, syms in placed:
        for n, (s_, o, _) in syms.items():
            if s_ is sec:
                code_syms.setdefault(n, addr + o)
    # A data section no name places (a module's init table entry, which every
    # module names .mod_init): where the EXE points at the same code.
    by_target = collections.defaultdict(list)
    for site, fx in le.fixups.items():
        if le.objects[2].contains(site) and fx.kind == SRC_OFFSET32:
            by_target[fx.target].append(site)
    for u, sec, syms, base, weak in pending:
        own_code = {n: a + o for u2, sec2, a, syms2 in placed if u2 is u for n, (s2, o, _) in syms2.items() if s2 is sec2}
        sites = None
        for at, rtype, target in sec.relocs:
            name = target[1] if target[0] == "sym" else None
            if name is None:
                where = base.get(id(target[1])) if target[1].code else None
            else:
                where = own_code.get(name, code_syms.get(name))
            if where is not None:
                addend = struct.unpack_from("<i", sec.data, at)[0]
                here = {site - at for site in by_target.get(where + addend, [])}
                sites = here if sites is None else sites & here
        if sites and len(sites) == 1:
            base[id(sec)] = sites.pop()
            data_placed.append((u, sec, base[id(sec)], syms, base, weak))
        else:
            unplaced_data.append((u, sec, base))

    def any_address(name):
        a = symbol_address(name)
        if a is None:
            a = code_syms.get(name)
        if a is None:
            a = placed_data.get(name)
        return a

    placed_data = {}
    for u, sec, addr, syms, base, weak in data_placed:
        for n, (s_, o, _) in syms.items():
            if s_ is sec:
                placed_data.setdefault(n, addr + o)
    check_data(le, data_placed, any_address, problems, notes)
    unplaced_data = [f"{u.name}: {sec.name} {max(len(sec.data), sec.length):#x} bytes"
                     for u, sec, base in unplaced_data if id(sec) not in base]
    if unplaced_data:
        problems["data sections not placed"] = len(unplaced_data)

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
    for n in unplaced_data[: 20 if not verbose else None]:
        print("  data not placed: " + n)
    boundaries = module_boundaries(proj, le)
    print(f"module boundary contradictions {len(boundaries)}")
    for kind, at, n, fa, ua, fb, ub in boundaries[: 40 if not verbose else None]:
        print(f"  {kind}: {at:#x} +{n}  {fa} [{ua}] -> {fb} [{ub}]")
    return not diffs and not nonzero_gaps and not problems and not boundaries


if __name__ == "__main__":
    sys.exit(0 if main("-v" in sys.argv[1:]) else 1)
