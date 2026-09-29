#!/usr/bin/env python3
"""Link every unit into a DOS/4G executable and report what does not resolve.

This tracks how close the sources are to linking; it is not the build of the
game yet. Compiled units link from the compiler's own OMF objects (which keep
its weak references and COMDATs), or, with assembly stand-ins, from the OMF
tools/cc.py writes after applying them; assembly units and auto units
(build/target, no sources yet) are converted from ELF to OMF with objconv. The EXE's data, 16-bit code and stack objects
become a generated data module that defines every name the code refers to
at its original address and carries the data's own relocations. Open
Watcom's wlink then links everything, in address order, and the unresolved
symbols are written to build/link/report.txt.

Known gaps: data the compiler emits itself (initialiser tables, module
registration blocks, exception tables) is still copied from the EXE as
well; data pointers into globals now defined by compiled units point at the
EXE's copy instead; the 16-bit code object is not marked as a 16-bit
segment; and Watcom 10.6's own wlink does not run under wibo.

Usage: tools/link.py    (after `ninja`)
"""

import collections
import os
import re
import shutil
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
from elfrelocs import _sections  # noqa: E402
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402
from split import q  # noqa: E402
from standin import companion  # noqa: E402

OUT = os.path.join(ROOT, "build", "link")


def elf_symbols(path):
    """(defined globals, undefined globals) of an ELF object."""
    b = open(path, "rb").read()
    secs, _ = _sections(b)
    st = next(s for s in secs if s[1] == 2)
    strtab = secs[st[6]][4]
    defined, undefined = set(), set()
    for i in range(1, st[5] // 16):
        nm, _, _, info, _, shndx = struct.unpack_from("<IIIBBH", b, st[4] + i * 16)
        if info >> 4 == 0:
            continue
        name = b[strtab + nm : b.index(b"\0", strtab + nm)].decode("latin-1")
        (undefined if shndx == 0 else defined).add(name)
    return defined, undefined


def data_module(proj, le, resolved):
    """GNU as source for the EXE's non-code objects, defining `resolved`
    ({name: address}) and relocating the data's own fixups."""
    names_at = collections.defaultdict(list)
    for n, a in resolved.items():
        names_at[a].append(n)
    data, code16, stack = le.objects[2], le.objects[1], le.objects[3]

    def target(t):
        if le.code.contains(t):
            f = proj.function_at(t)
            return None if f is None else q(f.name) + (f"+{t - f.addr:#x}" if t != f.addr else "")
        for obj, label in ((data, ".LDATA"), (code16, ".LCODE16")):
            if obj.contains(t):
                return f"{label}+{t - obj.base:#x}"
        return None

    out, missing = [".intel_syntax noprefix"], 0
    for obj, sec, label in ((data, "_DATA", ".LDATA"), (code16, "CODE16", ".LCODE16"), (stack, "STACK", ".LSTACK")):
        flags = '"ax"' if sec == "CODE16" else '"aw"'
        out.append(f"\n.section {sec},{flags},{'@nobits' if sec == 'STACK' else '@progbits'}\n{label}:")
        pending = []

        def flush():
            if pending:
                out.append(f".skip {len(pending)}" if sec == "STACK" else ".byte " + ",".join(f"{x:#04x}" for x in pending))
                pending.clear()

        a, end = obj.base, obj.base + obj.vsize
        while a < end:
            for n in names_at.get(a, []):
                flush()
                out.append(f".globl {q(n)}\n{q(n)}:")
            fx = le.fixups.get(a)
            if fx is not None and fx.kind == SRC_OFFSET32 and sec != "STACK":
                flush()
                e = target(fx.target)
                missing += e is None
                out.append(f".long {e or 0}")
                a += 4
                continue
            pending.append(0 if sec == "STACK" else le.read(a, 1)[0])
            if len(pending) == 16:
                flush()
            a += 1
        flush()
        for n in names_at.get(end, []):
            out.append(f".globl {q(n)}\n{q(n)}:")
    return "\n".join(out) + "\n", missing


def main():
    proj = Project()
    le = LEFile(EXE)
    shutil.rmtree(OUT, ignore_errors=True)
    os.makedirs(os.path.join(OUT, "objs"))

    units = sorted(proj.units, key=lambda u: u.functions[0].addr)
    elfs = [os.path.join(ROOT, "build", "target" if u.auto else "base", u.name + ".o") for u in units]

    defined, undefined = collections.Counter(), set()
    for path in elfs:
        d, un = elf_symbols(path)
        defined.update(d)
        undefined |= un
    need = sorted(undefined - set(defined))
    by_name = {s.name: s.addr for s in proj.data_symbols}
    resolved = {}
    for n in need:
        if n in by_name:
            resolved[n] = by_name[n]
        elif re.fullmatch(r"_D_[0-9a-f]{8}", n):
            resolved[n] = int(n[3:], 16)

    source, missing = data_module(proj, le, resolved)
    data_s, data_o = os.path.join(OUT, "data.s"), os.path.join(OUT, "data.o")
    open(data_s, "w").write(source)
    subprocess.run(["as", "--32", "-o", data_o, data_s], check=True)

    files = []
    for k, (u, path) in enumerate(list(zip(units, elfs)) + [(None, data_o)]):
        dst = os.path.join("objs", f"{k:03d}.obj")
        omf = os.path.splitext(path)[0] + ".obj"
        if u is not None and not u.auto and os.path.exists(os.path.join(ROOT, companion(u.source))):
            omf = os.path.splitext(path)[0] + ".link.obj"  # with its stand-ins
        if u is not None and not u.auto and os.path.exists(omf):
            shutil.copy(omf, os.path.join(OUT, dst))
        else:
            subprocess.run(["objconv", "-fomf", path, os.path.join(OUT, dst)], check=True, capture_output=True)
        files.append(dst)
    with open(os.path.join(OUT, "hosp.lnk"), "w") as f:
        f.write("system dos4g\noption nodefaultlibs\noption undefsok\noption start=_cstart_\n"
                "option map=hosp.map\noption stack=0x1000\nname hosp.exe\n")
        f.writelines(f"file {p}\n" for p in files)
    env = dict(os.environ, WATCOM=os.path.dirname(os.path.dirname(shutil.which("wlink"))))
    r = subprocess.run(["wlink", "@hosp.lnk"], cwd=OUT, env=env, capture_output=True, text=True)
    open(os.path.join(OUT, "link.out"), "w").write(r.stdout + r.stderr)

    unresolved = sorted({m.group(1) for m in re.finditer(r"undefined symbol (.+)", r.stdout)})
    duplicates = sorted({m.group(1) for m in re.finditer(r"redefinition of (\S+)", r.stdout)})
    with open(os.path.join(OUT, "report.txt"), "w") as f:
        f.write(f"objects {len(elfs)} (auto units {sum(u.auto for u in units)})\n")
        f.write(f"data relocations without a target {missing}\n")
        f.write(f"unresolved {len(unresolved)}\n" + "".join(f"  {n}\n" for n in unresolved))
        f.write(f"duplicate {len(duplicates)}\n" + "".join(f"  {n}\n" for n in duplicates))
    print(open(os.path.join(OUT, "report.txt")).read(), end="")


if __name__ == "__main__":
    main()
