#!/usr/bin/env python3
"""Convert a Watcom 32-bit OMF object into an ELF32 i386 relocatable object.

Covers what wpp386/wcc386 emit, including the records objconv drops or
rejects: local publics and externals (static functions, local initialiser
templates) and COMDATs (vtables, inline functions). Debug segments are
dropped. Every segment and every COMDAT becomes its own section; public
symbols in code sections are typed FUNC and sized up to the next symbol in
the same section, as objdiff needs. COMDATs with a selection criterion
(pick-any copies of generated functions, vtables and type signatures,
which every module using them emits) become weak symbols, so a link keeps
one copy as the Watcom linker does. References into a segment rather than
to a symbol become section-symbol relocations, as the assembler emits them.

Usage: tools/omf2elf.py INPUT.obj OUTPUT.o
"""

import os
import struct
import sys

ALIGN = {0: 1, 1: 1, 2: 2, 3: 16, 4: 256, 5: 4, 6: 4096}
DEBUG_CLASSES = {"DWARF", "DEBSYM", "DEBTYP"}
LOC_OFFSET32 = (9, 13)
# Watcom's code alignment fillers: nop, mov eax,eax (both encodings),
# lea eax,[eax+0].
PADDING = (b"\x90", b"\x8b\xc0", b"\x89\xc0", b"\x8d\x40\x00")

SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB, SHT_NOBITS, SHT_REL = 1, 2, 3, 8, 9
SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR = 1, 2, 4
STB_LOCAL, STB_GLOBAL, STB_WEAK = 0, 1, 2
STT_NOTYPE, STT_OBJECT, STT_FUNC, STT_SECTION = 0, 1, 2, 3
R_386_32, R_386_PC32 = 1, 2


class Reader:
    def __init__(self, data, big):
        self.d, self.p, self.big = data, 0, big

    def more(self):
        return self.p < len(self.d)

    def u8(self):
        self.p += 1
        return self.d[self.p - 1]

    def u16(self):
        self.p += 2
        return struct.unpack_from("<H", self.d, self.p - 2)[0]

    def u32(self):
        self.p += 4
        return struct.unpack_from("<I", self.d, self.p - 4)[0]

    def off(self):
        """Offset or length field: 32-bit in the odd-numbered record variants."""
        return self.u32() if self.big else self.u16()

    def index(self):
        b = self.u8()
        return ((b & 0x7F) << 8) | self.u8() if b & 0x80 else b

    def name(self):
        n = self.u8()
        self.p += n
        return self.d[self.p - n : self.p].decode("latin-1")

    def rest(self):
        r, self.p = self.d[self.p :], len(self.d)
        return r


class Section:
    def __init__(self, name, cls, align):
        self.name, self.cls, self.align = name, cls, align
        self.data = bytearray()
        self.length = 0
        self.relocs = []  # (offset, type, target), target = ("sec", Section) | ("sym", name)
        self.nobits = cls in ("BSS", "STACK")
        self.code = cls.endswith("CODE")
        self.comdat = False

    def write(self, off, data):
        if len(self.data) < off + len(data):
            self.data.extend(bytes(off + len(data) - len(self.data)))
        self.data[off : off + len(data)] = data


def lidata(r):
    """Expand one LIDATA iterated data block."""
    repeat = r.off()
    blocks = r.u16()
    if blocks == 0:
        body = r.d[r.p + 1 : r.p + 1 + r.d[r.p]]
        r.p += 1 + len(body)
    else:
        body = b"".join(lidata(r) for _ in range(blocks))
    return body * repeat


def convert(obj):
    lnames = [None]
    segments = [None]  # SEGDEF index -> Section, or None when dropped
    groups = [None]
    externs = [None]  # EXTDEF/LEXTDEF/CEXTDEF index -> (name, local)
    symbols = {}  # name -> (Section, offset, local)
    comdats = {}  # name -> Section
    # COMDATs the linker may take from any module (-> None), and weak or
    # lazy externals (-> (the default the linker uses when nothing defines
    # them, the COMENT class)).
    weak = {}
    sections = []
    target_thread = [None] * 4
    last = None  # (Section, offset) of the last LEDATA/COMDAT, for FIXUPP
    backpatches = []
    easy_omf = False  # Phar Lap Easy OMF-386: every record has 32-bit fields

    p = 0
    while p < len(obj):
        rtype, length = obj[p], struct.unpack_from("<H", obj, p + 1)[0]
        r = Reader(obj[p + 3 : p + 3 + length - 1], rtype & 1 or easy_omf)
        p += 3 + length
        kind = rtype & ~1
        if kind == 0x88 and obj[p - length + 1] == 0xAA:  # COMENT class
            easy_omf = True  # the assembler's objects (e.g. 386inite)
        elif kind == 0x88 and obj[p - length + 1] in (0xA8, 0xA9):  # weak, lazy externals
            cls, r.p = obj[p - length + 1], 2
            while r.more():
                name, default = r.index(), r.index()
                weak[externs[name][0]] = (externs[default][0], cls)
        elif kind in (0x80, 0x88, 0x94, 0xC4, 0xCC, 0xCE):
            pass  # THEADR, COMENT, LINNUM, LINSYM, VERNUM, VENDEXT
        elif kind in (0x96, 0xCA):  # LNAMES, LLNAMES
            while r.more():
                lnames.append(r.name())
        elif kind == 0x98:  # SEGDEF
            acbp = r.u8()
            if acbp >> 5 == 0:
                raise NotImplementedError("absolute segment")
            length = r.off()
            name, cls = lnames[r.index()], lnames[r.index()]
            if cls in DEBUG_CLASSES or name.startswith((".debug", "$$")):
                segments.append(None)
            else:
                sec = Section(name, cls, ALIGN[acbp >> 5])
                sec.length = length
                segments.append(sec)
                sections.append(sec)
        elif kind == 0x9A:  # GRPDEF
            groups.append(lnames[r.index()])
        elif kind in (0x8C, 0xB4):  # EXTDEF, LEXTDEF
            while r.more():
                externs.append((r.name(), kind == 0xB4))
                r.index()
        elif kind == 0xBC:  # CEXTDEF
            while r.more():
                externs.append((lnames[r.index()], False))
                r.index()
        elif kind in (0x90, 0xB6):  # PUBDEF, LPUBDEF
            r.index()
            seg = r.index()
            if seg == 0:
                raise NotImplementedError("PUBDEF with frame number")
            while r.more():
                name, off = r.name(), r.off()
                r.index()
                if segments[seg] is not None:
                    symbols[name] = (segments[seg], off, kind == 0xB6)
        elif kind in (0xA0, 0xA2):  # LEDATA, LIDATA
            sec, off = segments[r.index()], r.off()
            data = r.rest() if kind == 0xA0 else b"".join(lidata(r) for _ in iter(r.more, False))
            if sec is not None:
                sec.write(off, data)
            last = (sec, off, kind)
        elif kind == 0xC2:  # COMDAT
            flags, attrs, align = r.u8(), r.u8(), r.u8()
            off = r.off()
            r.index()
            seg = None
            if attrs & 0xF == 0:
                r.index()
                seg = r.index()
                if seg == 0:
                    raise NotImplementedError("COMDAT with frame number")
            name = lnames[r.index()]
            if flags & 2:
                raise NotImplementedError("iterated COMDAT")
            sec = comdats.get(name)
            if sec is None:
                base = segments[seg] if seg else None
                if base is not None:
                    sec = Section(base.name, base.cls, ALIGN[align] if align else base.align)
                elif attrs & 0xF in (1, 3):
                    sec = Section("_TEXT", "CODE", ALIGN.get(align, 1))
                else:
                    sec = Section("_DATA", "DATA", ALIGN.get(align, 1))
                sec.comdat = True
                comdats[name] = sec
                sections.append(sec)
                symbols[name] = (sec, 0, bool(flags & 4))
                if attrs & 0xF0:
                    weak[name] = None
            sec.write(off, r.rest())
            last = (sec, off, kind)
        elif kind == 0x9C:  # FIXUPP
            while r.more():
                b = r.u8()
                if not b & 0x80:  # THREAD
                    method = (b >> 2) & 7
                    if b & 0x40:
                        if method < 3:
                            r.index()
                    else:
                        target_thread[b & 3] = (method & 3, r.index())
                    continue
                locat = (b << 8) | r.u8()
                relative, loc, where = not locat & 0x4000, (locat >> 10) & 0xF, locat & 0x3FF
                if easy_omf and loc == 5:
                    loc = 9  # Easy OMF's 32-bit offset
                fixdat = r.u8()
                if fixdat & 0x80:
                    pass
                elif (fixdat >> 4) & 7 < 3:
                    r.index()
                if fixdat & 0x08:
                    method, datum = target_thread[fixdat & 3]
                    method |= fixdat & 4
                else:
                    method, datum = fixdat & 7, r.index()
                disp = 0 if fixdat & 4 else r.off()
                sec, base, rkind = last
                if sec is None:
                    continue  # debug segment
                if rkind == 0xA2:
                    raise NotImplementedError("fixup on LIDATA")
                if loc not in LOC_OFFSET32:
                    raise NotImplementedError(f"fixup location type {loc} in {sec.name}")
                if method & 3 == 0:
                    tsec = segments[datum]
                    if tsec is None:
                        continue
                    target = ("sec", tsec)
                elif method & 3 == 2:
                    target = ("sym", externs[datum][0])
                else:
                    raise NotImplementedError(f"fixup target method {method}")
                at = base + where
                addend = struct.unpack_from("<i", sec.data, at)[0] + disp - (4 if relative else 0)
                struct.pack_into("<i", sec.data, at, addend)
                sec.relocs.append((at, R_386_PC32 if relative else R_386_32, target))
        elif kind == 0xB2:  # BAKPAT
            sec, loc = segments[r.index()], r.u8()
            while r.more():
                backpatches.append((sec, loc, r.off(), r.off()))
        elif kind == 0x8A:  # MODEND
            break
        else:
            raise NotImplementedError(f"OMF record {rtype:#04x}")

    for sec, loc, off, value in backpatches:
        if sec is not None:
            fmt = {0: "<b", 1: "<h", 2: "<i"}[loc]
            struct.pack_into(fmt, sec.data, off, struct.unpack_from(fmt, sec.data, off)[0] + value)
    return sections, symbols, externs, weak


def write_elf(sections, symbols, externs, weak, path):
    shstr, strtab = bytearray(b"\0"), bytearray(b"\0")

    def add(tab, s):
        off = len(tab)
        tab.extend(s.encode("latin-1") + b"\0")
        return off

    # Section header order: null, content sections, their REL sections, symtab, strtab, shstrtab.
    index = {id(s): i + 1 for i, s in enumerate(sections)}

    syms = [(0, 0, 0, 0, 0)]  # (name offset, value, size, info, shndx)
    sym_index = {}
    for s in sections:
        sym_index[id(s)] = len(syms)
        syms.append((0, 0, 0, (STB_LOCAL << 4) | STT_SECTION, index[id(s)]))

    def sized(name):
        sec, off, _ = symbols[name]
        ends = [o for n, (s, o, _) in symbols.items() if s is sec and o > off]
        if not ends and sec.comdat and sec.code:
            # Alignment padding at the end of a function's COMDAT is not part
            # of it, as the executable's sizes have it.
            for pad in PADDING:
                end = len(sec.data) - len(pad)
                if sec.data.endswith(pad) and (sec.data[end - 1] == 0xC3 or sec.data[end - 3] == 0xC2):
                    return end - off
        # A BSS segment has no data records: its end is the SEGDEF length.
        return min(ends, default=max(len(sec.data), sec.length)) - off

    ordered = sorted(symbols.items(), key=lambda kv: (not kv[1][2], index[id(kv[1][0])], kv[1][1]))
    first_global = None
    for name, (sec, off, local) in ordered:
        if not local and first_global is None:
            first_global = len(syms)
        kind = STT_FUNC if sec.code else STT_OBJECT
        bind = STB_LOCAL if local else STB_WEAK if name in weak else STB_GLOBAL
        sym_index[name] = len(syms)
        syms.append((add(strtab, name), off, sized(name), (bind << 4) | kind, index[id(sec)]))
    if first_global is None:
        first_global = len(syms)
    for name, _ in externs[1:]:
        if name not in sym_index:
            sym_index[name] = len(syms)
            syms.append((add(strtab, name), 0, 0, (STB_GLOBAL << 4) | STT_NOTYPE, 0))

    rel_secs = [s for s in sections if s.relocs]
    nsec = 1 + len(sections) + len(rel_secs) + 3
    symtab_i = nsec - 3

    out = bytearray(52)
    headers = [(0,) * 10]
    for s in sections:
        while len(out) % 16:
            out.append(0)
        flags = SHF_ALLOC | (SHF_EXECINSTR if s.code else 0 if s.cls == "CONST" else SHF_WRITE)
        stype = SHT_NOBITS if s.nobits else SHT_PROGBITS
        headers.append((add(shstr, s.name), stype, flags, 0, len(out), len(s.data), 0, 0, s.align, 0))
        if not s.nobits:
            out += s.data
    for s in rel_secs:
        while len(out) % 4:
            out.append(0)
        start = len(out)
        for at, rtype, target in sorted(s.relocs):
            key = id(target[1]) if target[0] == "sec" else target[1]
            out += struct.pack("<II", at, (sym_index[key] << 8) | rtype)
        headers.append((add(shstr, ".rel" + s.name), SHT_REL, 0, 0, start, len(out) - start, symtab_i, index[id(s)], 4, 8))
    while len(out) % 4:
        out.append(0)
    start = len(out)
    for sym in syms:
        name, value, size, info, shndx = sym
        out += struct.pack("<IIIBBH", name, value, size, info, 0, shndx)
    headers.append((add(shstr, ".symtab"), SHT_SYMTAB, 0, 0, start, len(out) - start, symtab_i + 1, first_global, 4, 16))
    headers.append((add(shstr, ".strtab"), SHT_STRTAB, 0, 0, len(out), len(strtab), 0, 0, 1, 0))
    out += strtab
    shstr_name = add(shstr, ".shstrtab")
    headers.append((shstr_name, SHT_STRTAB, 0, 0, len(out), len(shstr), 0, 0, 1, 0))
    out += shstr
    while len(out) % 4:
        out.append(0)
    shoff = len(out)
    for h in headers:
        out += struct.pack("<10I", *h)

    ident = b"\x7fELF\x01\x01\x01" + bytes(9)
    struct.pack_into("<16sHHIIIIIHHHHHH", out, 0, ident, 1, 3, 1, 0, 0, shoff, 0, 52, 0, 0, 40, nsec, nsec - 1)
    open(path, "wb").write(out)


def write_omf(sections, symbols, externs, weak, path):
    """Write the object model back as a 32-bit OMF object for Watcom's
    linker, the inverse of convert: COMDAT sections become COMDAT records
    (pick-any for `weak` names), so the linker keeps one copy of each."""
    out = bytearray()

    def record(kind, body):
        rec = bytes([kind]) + struct.pack("<H", len(body) + 1) + bytes(body)
        out.extend(rec + bytes([-sum(rec) & 0xFF]))

    def index(i):
        return bytes([i]) if i < 0x80 else bytes([0x80 | i >> 8, i & 0xFF])

    def name(s):
        b = s.encode("latin-1")
        return bytes([len(b)]) + b

    lnames = {}

    def lname(s):
        if s not in lnames:
            lnames[s] = len(lnames) + 1
            record(0x96, name(s))
        return lnames[s]

    record(0x80, name(os.path.basename(path)))
    for s in ("", "CODE", "DATA", "BSS", "FLAT", "DGROUP"):
        lname(s)

    segs = {}  # id(section) -> SEGDEF index, for the plain (non-COMDAT) sections
    for sec in sections:
        if sec.comdat:
            continue
        cls = sec.cls if sec.cls in ("CODE", "DATA", "BSS") else ("CODE" if sec.code else "DATA")
        length = max(len(sec.data), sec.length)
        acbp = {1: 1, 2: 2, 4: 5, 16: 3, 256: 4}.get(sec.align, 3) << 5 | 2 << 2 | 1
        record(0x99, bytes([acbp]) + struct.pack("<I", length) + index(lname(sec.name)) + index(lname(cls)) + index(1))
        segs[id(sec)] = len(segs) + 1
    code_seg = next((segs[id(s)] for s in sections if not s.comdat and s.code), None)
    data_seg = next((segs[id(s)] for s in sections if not s.comdat and not s.code), None)
    dgroup = [segs[id(s)] for s in sections if not s.comdat and not s.code]
    record(0x9A, index(lnames["DGROUP"]) + b"".join(b"\xff" + index(i) for i in dgroup))
    record(0x9A, index(lnames["FLAT"]))
    DGROUP, FLAT = 1, 2

    # Names the fixups reach through EXTDEF: externals and COMDAT symbols;
    # symbols in plain segments are reached segment-relative.
    comdat_of = {id(s): n for n, (s, o, _) in symbols.items() if s.comdat and o == 0}
    ext = {}

    def extern(n):
        if n not in ext:
            ext[n] = len(ext) + 1
            record(0x8C, name(n) + index(0))
        return ext[n]

    def fixups(sec, lo, hi):
        body = bytearray()
        for at, rtype, target in sec.relocs:
            if not lo <= at < hi:
                continue
            addend = struct.unpack_from("<i", sec.data, at)[0] + (4 if rtype == R_386_PC32 else 0)
            if target[0] == "sym" and target[1] in symbols and not symbols[target[1]][0].comdat:
                tsec, toff, _ = symbols[target[1]]
                method, datum, addend = 0, segs[id(tsec)], addend + toff
            elif target[0] == "sym":
                method, datum = 2, extern(target[1])
            elif target[1].comdat:
                method, datum = 2, extern(comdat_of[id(target[1])])
            else:
                method, datum = 0, segs[id(target[1])]
            locat = 0x8000 | (0x4000 if rtype == R_386_32 else 0) | 9 << 10 | (at - lo)
            # Frame F1 (group FLAT), no displacement: the field holds the addend.
            body += struct.pack(">H", locat) + bytes([0x10 | 4 | method]) + index(FLAT) + index(datum)
            patched[at] = addend
        return body

    for sec in sections:
        patched = {}
        data = bytearray(sec.data)
        lo = 0
        while lo < len(data) and not sec.nobits:
            hi = min(lo + 1024, len(data))
            # Never split a relocated field across two records.
            while any(lo < at < hi < at + 4 for at, _, _ in sec.relocs):
                hi -= 1
            fx = fixups(sec, lo, hi)
            chunk = bytearray(data[lo:hi])
            for at, value in patched.items():
                if lo <= at < hi:
                    struct.pack_into("<i", chunk, at - lo, value)
            if sec.comdat:
                cname = comdat_of[id(sec)]
                local = symbols[cname][2]
                flags = (1 if lo else 0) | (4 if local else 0)
                attrs = 0x10 if cname in weak else 0x00  # pick-any; explicit allocation
                base_seg = code_seg if sec.code else data_seg
                grp = 0 if sec.code else DGROUP
                record(0xC3, bytes([flags, attrs, 0]) + struct.pack("<I", lo) + index(0)
                       + index(grp) + index(base_seg) + index(lname(cname)) + chunk)
            else:
                record(0xA1, index(segs[id(sec)]) + struct.pack("<I", lo) + chunk)
            if fx:
                record(0x9D, fx)
            lo = hi
    for name_, default in weak.items():
        if default is not None and name_ in ext:
            record(0x88, bytes([0x80, default[1]]) + index(ext[name_]) + index(extern(default[0])))
    for local in (False, True):
        for sec in sections:
            if sec.comdat:
                continue
            names = [(n, o) for n, (s, o, l) in symbols.items() if s is sec and l == local and not n.endswith(".c")]
            if names:
                grp = FLAT if sec.code else DGROUP
                record(0xB7 if local else 0x91, index(grp) + index(segs[id(sec)])
                       + b"".join(name(n) + struct.pack("<I", o) + index(0) for n, o in names))
    record(0x8B, b"\x00")
    open(path, "wb").write(out)


def library_module(path, name):
    """The OMF object of the module `name` (its THEADR name) in a library."""
    b = open(path, "rb").read()
    page = struct.unpack_from("<H", b, 1)[0] + 3
    p = page
    while p < len(b) and b[p] != 0xF1:  # library end record
        start, module = p, None
        while True:
            kind, length = b[p], struct.unpack_from("<H", b, p + 1)[0]
            if kind == 0x80 and module is None:  # THEADR
                module = b[p + 4 : p + 4 + b[p + 3]].decode("latin-1")
            p += 3 + length
            if kind in (0x8A, 0x8B):  # MODEND
                break
        if module == name:
            return b[start:p]
        p = -(-p // page) * page
    raise KeyError(f"{path}: no module {name}")


def main(src, dst):
    write_elf(*convert(open(src, "rb").read()), dst)


if __name__ == "__main__":
    main(*sys.argv[1:])
