#!/usr/bin/env python3
"""Mark global symbols in executable sections of an ELF32 object as FUNC, sized
up to the next global symbol in the same section (or the section end).

objconv carries no type or size over from OMF and emits every public as a
4-byte OBJECT, which objdiff would otherwise diff as data. Local symbols
(objconv's ?NoName labels for debug-info references) are left alone.

Usage: tools/fixsyms.py OBJECT.o   (rewritten in place)
"""

import struct
import sys

SHT_SYMTAB = 2
SHF_EXECINSTR = 0x4
STT_NOTYPE, STT_OBJECT, STT_FUNC = 0, 1, 2
STB_LOCAL = 0


def main(path):
    b = bytearray(open(path, "rb").read())
    shoff = struct.unpack_from("<I", b, 0x20)[0]
    shentsize, shnum = struct.unpack_from("<HH", b, 0x2E)
    sections = [struct.unpack_from("<10I", b, shoff + i * shentsize) for i in range(shnum)]
    symtab = next(s for s in sections if s[1] == SHT_SYMTAB)
    off, size, entsize = symtab[4], symtab[5], symtab[9]

    code_syms = []  # (section index, value, symbol entry offset, st_info)
    for i in range(size // entsize):
        o = off + i * entsize
        _, value, _, info, _, shndx = struct.unpack_from("<IIIBBH", b, o)
        if (
            info >> 4 != STB_LOCAL
            and 0 < shndx < shnum
            and sections[shndx][2] & SHF_EXECINSTR
            and info & 0xF in (STT_NOTYPE, STT_OBJECT)
        ):
            code_syms.append((shndx, value, o, info))

    for shndx, value, o, info in code_syms:
        following = [v for s, v, _, _ in code_syms if s == shndx and v > value]
        end = min(following, default=sections[shndx][5])
        struct.pack_into("<IB", b, o + 8, end - value, (info & 0xF0) | STT_FUNC)
    open(path, "wb").write(b)


if __name__ == "__main__":
    main(sys.argv[1])
