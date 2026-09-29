"""Per-function relocation lists of an ELF32 i386 relocatable object.

objdiff compares at most one relocation per x86 instruction, so an
instruction such as `mov [A], offset B` matches whatever B is. Comparing
these lists closes that gap.
"""

import struct

SHT_SYMTAB, SHT_REL = 2, 9
STT_FUNC, STT_SECTION = 2, 3
STB_LOCAL = 0
R_386_PC32 = 2


def _sections(b):
    shoff = struct.unpack_from("<I", b, 0x20)[0]
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", b, 0x2E)
    secs = [struct.unpack_from("<10I", b, shoff + i * shentsize) for i in range(shnum)]
    strtab = secs[shstrndx]
    name = lambda off: b[strtab[4] + off : b.index(b"\0", strtab[4] + off)].decode()
    return secs, [name(s[0]) for s in secs]


def function_relocs(path):
    """{function symbol: [(offset in function, type, target, addend)]}."""
    b = open(path, "rb").read()
    secs, sec_names = _sections(b)
    symtab_i = next(i for i, s in enumerate(secs) if s[1] == SHT_SYMTAB)
    symtab = secs[symtab_i]
    strtab = secs[symtab[6]]
    symname = lambda off: b[strtab[4] + off : b.index(b"\0", strtab[4] + off)].decode()

    syms = []
    for i in range(symtab[5] // 16):
        name, value, size, info, _, shndx = struct.unpack_from("<IIIBBH", b, symtab[4] + i * 16)
        kind = info & 0xF
        label = sec_names[shndx] if kind == STT_SECTION else symname(name)
        syms.append((label, value, size, kind, shndx, info >> 4))

    funcs = {label: (shndx, value, size) for label, value, size, kind, shndx, _ in syms if kind == STT_FUNC}
    starts = {(shndx, value): name for name, (shndx, value, _) in funcs.items()}
    shared = {shndx for shndx, _ in starts if sum(s == shndx for s, _ in starts) > 1}
    relocs = {name: [] for name in funcs}
    for rel in secs:
        if rel[1] != SHT_REL or rel[6] != symtab_i:
            continue
        target_sec = rel[7]
        data_off = secs[target_sec][4]
        for j in range(rel[5] // 8):
            off, info = struct.unpack_from("<II", b, rel[4] + j * 8)
            sym = syms[info >> 8]
            addend = struct.unpack_from("<i", b, data_off + off)[0]
            label, value, _, kind, shndx, bind = sym
            target = label
            if kind == STT_SECTION or (bind == STB_LOCAL and shndx == target_sec):
                # A reference into the function's own section (a jump table
                # or label), identified by its offset in the section; in a
                # section shared by several functions, by its offset from the
                # function instead, since a size difference in an earlier one
                # would shift every later offset.
                target = "<local>"
                addend += value
                # A call within a section shared by several functions (a
                # module built without -zm) lands on a function's entry;
                # the target names it after that function.
                if info & 0xFF == R_386_PC32:
                    callee = starts.get((shndx, addend + 4))
                    if callee:
                        target, addend = callee, -4
            for name, (shndx, value, size) in funcs.items():
                if shndx == target_sec and value <= off < value + size:
                    local_addend = addend - value if target == "<local>" and shndx in shared else addend
                    relocs[name].append((off - value, info & 0xFF, target, local_addend))
    return {name: sorted(r) for name, r in relocs.items()}
