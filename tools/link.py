#!/usr/bin/env python3
"""Link every unit into a DOS/4G executable and report what does not resolve.

This tracks how close the sources are to linking; it is not yet a faithful
build of the game. Every unit is read into tools/omf2elf.py's object model:
compiled units from the compiler's own OMF object (or, with assembly
stand-ins, the one tools/cc.py writes after applying them), library units
from their library module, and assembly and auto units from their ELF
object. Each unit's static functions get a global alias, `unit::name`, so
the EXE's data can point at them; references into the middle of a function
(`_C_XXXXXXXX`) become the function's symbol plus an offset, and ones
outside every function are defined with their gap's bytes. The 16-bit
code object's assembly modules (config/sections.txt, segment CODE16) are
read from their ELF objects as 16-bit segments, and a name the code refers
to by an address inside one is defined there. The EXE's data and stack
objects become a generated data module that defines every other name the
code refers to at its original address and carries the data's own
relocations. Everything is written as OMF and linked in
address order with Open Watcom's wlink; unresolved and duplicate symbols
are written to build/link/report.txt.

Known gaps: data the compiler emits itself (initialiser tables, module
registration blocks, exception tables) is still copied from the EXE as well,
and the EXE's copy is the one the runtime walks; its pointers into a unit's
static data point at the EXE's copy; and Watcom 10.6's own wlink does not
run under wibo.

Usage: tools/link.py    (after `ninja`)
"""

import os
import re
import shutil
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
import omf2elf  # noqa: E402
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402
from standin import companion  # noqa: E402

OUT = os.path.join(ROOT, "build", "link")
INTERIOR = re.compile(r"_C_([0-9a-f]{8})")
TABLE_RECORDS = re.compile(r"^//\s*TABLE RECORDS:\s*(\S+)", re.M)


def unit_model(u):
    """The object model of a unit, from the object tools/link.py links."""
    base = os.path.join(ROOT, "build", "target" if u.auto else "base", u.name)
    if not u.auto:
        with_standins = os.path.exists(os.path.join(ROOT, companion(u.source)))
        omf = base + (".link.obj" if with_standins else ".obj")
        if os.path.exists(omf):
            return omf2elf.convert(open(omf, "rb").read())
    model = omf2elf.read_elf(base + ".o")
    if not u.auto and u.source.endswith(".s"):
        table_records(model, open(os.path.join(ROOT, u.source)).read())
    return model


def table_records(model, source):
    """Record order of an assembly unit (GNU as keeps none): its sections'
    relocations in section order, except in a table that a
    `// TABLE RECORDS: SYMBOL` line in the source names: the original
    assembler emitted each entry of such a table right after the routine
    it points at, so the linker processed an entry's fixup after that
    routine's own fixups."""
    tables = TABLE_RECORDS.findall(source)
    if not tables:
        return
    sections, symbols = model[0], model[1]
    rank = {id(sec): 2 * i for i, sec in enumerate(sections)}
    for sec in sections:
        sec.reloc_records = {at: rank[id(sec)] for at, _, _ in sec.relocs}
    for name in tables:
        table = symbols[name][0]
        for at, _, target in table.relocs:
            if target[0] == "sym" and target[1] in symbols:
                table.reloc_records[at] = rank[id(symbols[target[1]][0])] + 1


def alias(unit, name):
    return f"{unit.name}::{name}"


def data_module(proj, le, resolved, code_name, defined, gaps):
    """The EXE's non-code objects as an object model: `resolved` ({name:
    address}) defined at their addresses, and the data's own relocations.
    `gaps` ({name: address}) are places in the code object outside every
    function, carried with the bytes of their gap."""
    data, stack = le.objects[2], le.objects[3]
    sections, symbols = [], {}
    starts = sorted(f.addr for f in proj.functions)
    for lo in sorted({max((f.end for f in proj.functions if f.end <= a), default=le.code.base) for a in gaps.values()}):
        hi = min(a for a in starts + [le.code.base + le.code.vsize] if a > lo)
        sec = omf2elf.Section("_TEXT", "CODE", 1)
        sec.write(0, bytes(le.read(lo, hi - lo)))
        for n, a in gaps.items():
            if lo <= a < hi:
                symbols[n] = (sec, a - lo, False)
        sections.append((None, sec))
    gap_sections = [sec for _, sec in sections]
    sections = []
    for obj, name, cls in ((data, "_DATA", "DATA"), (stack, "STACK", "STACK")):
        sec = omf2elf.Section(name, cls, 16)
        if cls != "STACK":
            sec.write(0, bytes(le.read(obj.base, obj.vsize)))
        sec.length = obj.vsize
        sections.append((obj, sec))

    missing = 0
    for n, a in resolved.items():
        for obj, sec in sections:
            if obj.base <= a <= obj.base + obj.vsize:
                symbols[n] = (sec, a - obj.base, False)
                break

    for obj, sec in sections[:1]:
        for site, fx in le.fixups.items():
            if not obj.contains(site) or fx.kind != SRC_OFFSET32:
                continue
            at, t = site - obj.base, fx.target
            target = None
            if le.code.contains(t):
                f = proj.function_at(t)
                if f is not None and code_name(f) is not None:
                    target, addend = ("sym", code_name(f)), t - f.addr
            else:
                s = proj.data_symbol_at(t)
                if s is not None and s.name in defined:
                    # A global a unit now defines: its copy, not the EXE's.
                    target, addend = ("sym", s.name), t - s.addr
                else:
                    for tobj, tsec in sections:
                        if tobj.contains(t):
                            target, addend = ("sec", tsec), t - tobj.base
            if target is None:
                missing += 1
                continue
            struct.pack_into("<i", sec.data, at, addend)
            sec.relocs.append((at, omf2elf.R_386_32, target))
    return [sec for _, sec in sections] + gap_sections, symbols, [None], {}, missing


def main():
    proj = Project()
    le = LEFile(EXE)
    shutil.rmtree(OUT, ignore_errors=True)
    os.makedirs(os.path.join(OUT, "objs"))

    units = sorted(proj.units, key=lambda u: u.functions[0].addr)
    models = [unit_model(u) for u in units]
    unit_of = {f.addr: u for u in units for f in u.functions}
    defined = {n for m in models for n, (_, _, local) in m[1].items() if not local}
    local_names = {id(u): {n for n, (s, _, local) in m[1].items() if local and s.code} for u, m in zip(units, models)}

    def code_name(f):
        u = unit_of.get(f.addr)
        if f.name in defined:
            return f.name
        if u is not None and f.name in local_names[id(u)]:
            return alias(u, f.name)
        return None

    gaps = {}
    for u, (sections, symbols, externs, weak) in zip(units, models):
        for n in local_names[id(u)]:
            sec, off, _ = symbols[n]
            symbols[alias(u, n)] = (sec, off, False)
        for sec in sections:
            for i, (at, rtype, target) in enumerate(sec.relocs):
                m = target[0] == "sym" and target[1] not in symbols and INTERIOR.fullmatch(target[1])
                if not m:
                    continue
                addr = int(m.group(1), 16)
                f = proj.function_at(addr)
                name = code_name(f) if f is not None else None
                if name is None:
                    gaps[target[1]] = addr  # outside every function
                    continue
                addend = struct.unpack_from("<i", sec.data, at)[0] + addr - f.addr
                struct.pack_into("<i", sec.data, at, addend)
                sec.relocs[i] = (at, rtype, ("sym", name))

    needed = set()
    for sections, symbols, externs, weak in models:
        for sec in sections:
            needed.update(t[1] for _, _, t in sec.relocs if t[0] == "sym" and t[1] not in symbols)
    by_name = {s.name: s.addr for s in proj.data_symbols}
    resolved = {}
    for n in sorted(needed - defined):
        if n in by_name:
            resolved[n] = by_name[n]
        elif re.fullmatch(r"_D_[0-9a-f]{8}", n):
            resolved[n] = int(n[3:], 16)
    # The 16-bit code object's modules. A name the code refers to by its
    # address there (the C knows the handlers only as bounds to copy) is an
    # alias of the place in the module that covers it, or ends there.
    code16 = []
    for m in proj.code16:
        sections, symbols, externs, weak = omf2elf.read_elf(os.path.join(ROOT, "build", "base", m.name + ".o"))
        sec = next(s for s in sections if s.name == "CODE16")  # not the assembler's empty .text
        sec.use16 = True
        code16.append((m, ([sec], symbols, externs, weak)))
        for n, a in list(resolved.items()):
            if m.addr <= a < m.end or a == m.end and not any(o.addr == a for o in proj.code16):
                symbols[n] = (sec, a - m.addr, False)
                del resolved[n]
    *data, missing = data_module(proj, le, resolved, code_name, defined, gaps)

    files = []
    for k, model in enumerate(models + [model for _, model in code16] + [tuple(data)]):
        dst = os.path.join("objs", f"{k:03d}.obj")
        omf2elf.write_omf(*model, os.path.join(OUT, dst))
        files.append(dst)
    with open(os.path.join(OUT, "hosp.lnk"), "w") as f:
        f.write("system dos4g\noption nodefaultlibs\noption undefsok\noption start=_cstart_\n"
                "option map=hosp.map\nname hosp.exe\n")
        f.writelines(f"file {p}\n" for p in files)
    env = dict(os.environ, WATCOM=os.path.dirname(os.path.dirname(shutil.which("wlink"))))
    r = subprocess.run(["wlink", "@hosp.lnk"], cwd=OUT, env=env, capture_output=True, text=True)
    open(os.path.join(OUT, "link.out"), "w").write(r.stdout + r.stderr)

    unresolved = sorted({m.group(1) for m in re.finditer(r"undefined symbol (.+)", r.stdout)})
    duplicates = sorted({m.group(1) for m in re.finditer(r"redefinition of (\S+)", r.stdout)})
    with open(os.path.join(OUT, "report.txt"), "w") as f:
        f.write(f"objects {len(models) + len(code16) + 1} (auto units {sum(u.auto for u in units)})\n")
        f.write(f"data relocations without a target {missing}\n")
        f.write(f"code references outside every function {len(gaps)}\n")
        f.write(f"unresolved {len(unresolved)}\n" + "".join(f"  {n}\n" for n in unresolved))
        f.write(f"duplicate {len(duplicates)}\n" + "".join(f"  {n}\n" for n in duplicates))
    print(open(os.path.join(OUT, "report.txt")).read(), end="")


if __name__ == "__main__":
    main()
