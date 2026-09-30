#!/usr/bin/env python3
"""Write the executable from the units and compare the file with the
original byte for byte.

The code object comes from tools/relink.py's placement of every unit at its
original address. Gaps between functions are filled as the original linker
filled them: with the no-op filler sequence for the gap's length between two
segments of one object, with zeros between objects (and throughout the
objects that were zero-padded, and around COMDATs, placed like objects).
The data object comes from the units' data sections where relink.py places
them. A relocated field holds the target's offset within its object, as LE
pages do. Each page's fixup records are
generated in the order the original linker processed them (objects in link
order, each object's fixups in FIXUPP record order), packed into 512-byte
blocks written newest first. The LE header, object table, page map, name
tables, fixup page table and import tables are computed from the objects so
built.

Still taken from the original: the DOS/4GW stub; the 16-bit code object (no
unit has a CODE16 segment) and its two fixups; the data no unit places (most
globals are still extern at their addresses, and runtime tables) with the
fixups inside it, and the record order of the one data page whose order
depends on it (DATA_PAGES_IN_ORIGINAL_ORDER); and the sizes of what lies
past the last unit's data (DATA_VSIZE, OBJECT4_VSIZE).

Usage: tools/lewrite.py [OUTPUT]    (default build/hospital.exe; after `ninja`)
"""

import bisect
import collections
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
import relink  # noqa: E402
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402

# The no-op sequences the original linker filled a gap of each length with.
FILLER = {n: bytes.fromhex(h) for n, h in {
    1: "90", 2: "8bc0", 3: "8d4000", 4: "8d442000", 5: "8d40008bc9",
    6: "8d8000000000", 7: "8d800000000090", 8: "8d80000000008bd2",
    9: "8d80000000008d5200", 10: "8d80000000008d542200",
    11: "8d80000000008d52008bdb", 12: "8d80000000008d9200000000",
    13: "8d80000000008d920000000090", 14: "8d80000000008d92000000008bc0",
    15: "8d80000000008d92000000008d4000"}.items()}
BLOCK = 512  # the linker's fixup record block size
PAGE = 0x1000
OBJECT_ALIGN = 0x10000  # each object starts at the next 64K boundary
FILE_ALIGN = 0x200  # the pages start at the next 512-byte boundary
HEADER_SIZE = 0xC4
MODULE_NAME = b"MAIN"
# Object flags: readable 0x1, writable 0x2, executable 0x4, preload 0x40,
# 16:16 alias 0x1000, 32-bit 0x2000.
CODE_FLAGS, CODE16_FLAGS, DATA_FLAGS = 0x2045, 0x1045, 0x2043
SRC_OFFSET16 = 0x05

# Sizes no unit determines: DGROUP's end (the stack, after the last unit's
# BSS) and the fourth object, which no unit places anything in.
DATA_VSIZE = 0x79F30
OBJECT4_VSIZE = 0x1004
# Data pages whose fixups were processed in an order no unit gives: the last
# page's init/fini tables and runtime tables come from runtime modules whose
# data no unit places, so their link order is unknown. Their records keep the
# original's order.
DATA_PAGES_IN_ORIGINAL_ORDER = {0x33}


def fixup_record(page_offset, target_obj, target_off, kind=SRC_OFFSET32):
    """An internal offset fixup, as the original encodes it."""
    flags = 0x10 if target_off > 0xFFFF else 0
    tail = struct.pack("<I", target_off) if flags else struct.pack("<H", target_off)
    return bytes([kind, flags]) + struct.pack("<h", page_offset) + bytes([target_obj]) + tail


def page_records(entries):
    """A page's fixup records from (page offset, record) in processing order."""
    blocks, cur, size = [], [], 0
    for _, rec in entries:
        if size + len(rec) > BLOCK:
            blocks.append(cur)
            cur, size = [], 0
        cur.append(rec)
        size += len(rec)
    if cur:
        blocks.append(cur)
    return b"".join(r for blk in reversed(blocks) for r in blk)


def original_order(entries, sites, page_addr):
    """A page's records in the file order of the original's `sites` (linear
    addresses); records it lacks follow in page offset order."""
    by_offset = dict(entries)
    out = [by_offset.pop(s - page_addr) for s in sites if s - page_addr in by_offset]
    return b"".join(out + [by_offset[k] for k in sorted(by_offset)])


def add_fixup(per_page, rel, tobj, off, kind=SRC_OFFSET32):
    """Record a fixup at object offset `rel` on each page its field touches."""
    width = 2 if kind == SRC_OFFSET16 else 4
    for pg in sorted({rel // PAGE, (rel + width - 1) // PAGE}):
        per_page[pg].append((rel - pg * PAGE, fixup_record(rel - pg * PAGE, tobj, off, kind)))


def relink_all():
    """relink.main's result, with the data sections it placed and its symbol
    lookup ('data_placed', 'symbol_address')."""
    result, check = {}, relink.check_data

    def keep(le, placed, symbol_address, problems, notes):
        check(le, placed, symbol_address, problems, notes)
        result.update(data_placed=placed, symbol_address=symbol_address)

    relink.check_data = keep
    try:
        relink.main(result=result)
    finally:
        relink.check_data = check
    return result


def code_image(result, vsize, base, proj):
    """The code object's bytes and the gaps (address offset, size) left
    unfilled. `base` is the object's address; `proj` gives the functions and
    which of them are COMDATs."""
    image, owner = bytearray(result["image"][:vsize]), result["owner"]
    starts = sorted(f.addr for f in proj.functions)
    comdats = {a for u in proj.units for a in u.comdats}
    i, unfilled = 0, []
    while i < vsize:
        if owner[i] is not None:
            i += 1
            continue
        j = i
        while j < vsize and owner[j] is None:
            j += 1
        before, after = owner[i - 1] if i else None, owner[j] if j < vsize else None
        n = j - i
        # The linker placed a COMDAT like a separate object.
        comdat = base + j in comdats or starts[bisect.bisect(starts, base + i - 1) - 1] in comdats
        if before is not None and before == after and before not in result["zero_padded"] and not comdat and n in FILLER:
            image[i:j] = FILLER[n]
        elif n >= 16:
            unfilled.append((i, n))
        i = j
    return image, unfilled


def data_image(result, data, vsize):
    """The data object's bytes from the units' sections, which bytes they
    own, and their fixups {offset: (order, target object index, target
    offset)}, where order is the unit's place in link order and the fixup's
    place in its FIXUPP records; of a field several units place (a COMDAT
    vtable), the first unit's. `data` is the built data object; target
    objects are looked up in result['objects']."""
    image, owner, fixups, unknown = bytearray(vsize), [False] * vsize, {}, 0
    placed, symbol_address = list(result["data_placed"]), result["symbol_address"]
    # relink.py places a unit's data that only its own data refers to while
    # checking; that section's base is in the unit's `base` afterwards.
    seen = {id(p[1]) for p in placed}
    for u, sec, addr, syms, base, weak in placed:
        for s, _, _ in syms.values():
            if not s.code and id(s) in base and id(s) not in seen:
                seen.add(id(s))
                placed.append((u, s, base[id(s)], syms, base, weak))
    rank = {u.name: i for i, u in enumerate(sorted({p[0].name: p[0] for p in placed}.values(), key=lambda u: u.functions[0].addr))}
    for u, sec, addr, syms, base, weak in placed:
        size = max(len(sec.data), sec.length)
        at0 = addr - data.base
        image[at0 : at0 + len(sec.data)] = sec.data
        owner[at0 : at0 + size] = [True] * size
        for n, (at, _, target) in enumerate(sec.relocs):
            addend = struct.unpack_from("<i", sec.data, at)[0]
            if target[0] == "sec":
                t = base.get(id(target[1]))
            elif target[1] in syms and id(syms[target[1]][0]) in base:
                t = base[id(syms[target[1]][0])] + syms[target[1]][1]
            else:
                t = symbol_address(target[1])
                if t is None and isinstance(weak.get(target[1]), tuple):
                    t = symbol_address(weak[target[1]][0])  # the weak default
            if t is None:
                unknown += 1
                continue
            value = (t + addend) & 0xFFFFFFFF
            tobj = next(o for o in result["objects"] if o.base <= value <= o.base + o.vsize)
            struct.pack_into("<I", image, at0 + at, value - tobj.base)
            order = (rank[u.name], sec.reloc_records.get(at, 1 << 30), n)
            if at0 + at not in fixups or order < fixups[at0 + at][0]:
                fixups[at0 + at] = (order, tobj.index, value - tobj.base)
    return image, owner, fixups, unknown


class Obj:
    """A built object: its bytes (the initialised part) and virtual size."""

    def __init__(self, index, base, vsize, flags, image):
        self.index, self.base, self.vsize, self.flags, self.image = index, base, vsize, flags, image

    @property
    def pages(self):
        return (len(self.image) + PAGE - 1) // PAGE


def next_base(obj):
    return (obj.base + obj.vsize + OBJECT_ALIGN - 1) // OBJECT_ALIGN * OBJECT_ALIGN


def main(output=os.path.join(ROOT, "build", "hospital.exe")):
    le, proj = LEFile(EXE), Project()
    d = le.raw
    result = relink_all()

    # Objects. Every unit is placed at its original address, so the code
    # object ends where the last unit's code does.
    code_vsize = max(i for i, o in enumerate(result["owner"]) if o is not None) + 1
    # CODE16: the interrupt handlers dosipx and dosser install; no unit has
    # this segment, so its bytes are the original's.
    code16_bytes = le.objects[1].data
    objects = [Obj(1, le.code.base, code_vsize, CODE_FLAGS, b"")]
    objects.append(Obj(2, next_base(objects[0]), len(code16_bytes), CODE16_FLAGS, code16_bytes))
    objects.append(Obj(3, next_base(objects[1]), DATA_VSIZE, DATA_FLAGS, b""))
    objects.append(Obj(4, next_base(objects[2]), OBJECT4_VSIZE, DATA_FLAGS, b""))
    code, code16, data, _ = objects
    result["objects"] = objects

    # Code object and its fixups.
    image, unfilled = code_image(result, code.vsize, code.base, proj)
    code_pages = collections.defaultdict(list)
    for site, target in result["code_fixups"]:
        # An object's end (an end-of-segment label) belongs to it too.
        tobj = next(o for o in objects if o.base <= target <= o.base + o.vsize)
        rel = site - code.base
        struct.pack_into("<I", image, rel, target - tobj.base)
        add_fixup(code_pages, rel, tobj.index, target - tobj.base)
    code.image = bytes(image)

    # CODE16's fixups (16-bit offsets within itself), from the original.
    code16_pages = collections.defaultdict(list)
    orig_code16 = le.objects[1]
    for site, fx in sorted(le.fixups.items()):
        if orig_code16.contains(site):
            tbase = le.objects[fx.target_obj - 1].base
            add_fixup(code16_pages, site - orig_code16.base, fx.target_obj, fx.target - tbase, fx.kind)

    # Data object: the units' sections over the bytes no unit places.
    dimage, downer, dfixups, unknown = data_image(result, data, DATA_VSIZE)
    orig_data = le.objects[2]
    lh = struct.unpack_from("<I", d, 0x3C)[0]
    orig_table = lh + struct.unpack_from("<I", d, lh + 0x40)[0]
    _, _, _, orig_first, orig_pages, _ = struct.unpack_from("<6I", d, orig_table + 2 * 24)
    orig_init = PAGE * (orig_pages - 1) + struct.unpack_from("<I", d, lh + 0x2C)[0]
    # Bytes no unit places are the original's, up to the end of its
    # initialised data (the last segment there is no unit's either), and so
    # are the fixups among them.
    unowned = [i for i in range(orig_init) if not downer[i] and orig_data.data[i]]
    for i in unowned:
        dimage[i] = orig_data.data[i]
    unowned_fixups = [(site - orig_data.base, fx) for site, fx in le.fixups.items()
                      if orig_data.contains(site) and not downer[site - orig_data.base]]
    # Data no unit places was processed with the unit placed before it.
    owned_at = sorted(dfixups)
    for rel, fx in unowned_fixups:
        k = bisect.bisect(owned_at, rel)
        order = dfixups[owned_at[k - 1]][0] + (rel,) if k else (-1, rel)
        dfixups[rel] = (order, fx.target_obj, fx.target - le.objects[fx.target_obj - 1].base)
    init = max(orig_init, max((i + 1 for i in range(len(downer)) if downer[i] and dimage[i]), default=0))
    data.image = bytes(dimage[:init])
    data_pages = collections.defaultdict(list)
    for rel, (_, tobj, off) in sorted(dfixups.items(), key=lambda f: f[1][0]):
        add_fixup(data_pages, rel, tobj, off)

    # Page numbering, page map and fixup records.
    first = 1
    for o in objects:
        o.first_page, first = first, first + o.pages
    npages = first - 1
    records, page_table, origin = bytearray(), [0], []
    for o, per_page in ((code, code_pages), (code16, code16_pages), (data, data_pages)):
        for pg in range(o.pages):
            if o is data and pg in DATA_PAGES_IN_ORIGINAL_ORDER:
                records += original_order(per_page[pg], le.page_order[orig_first - 1 + pg], orig_data.base + pg * PAGE)
            else:
                records += page_records(per_page[pg])
            page_table.append(len(records))
            origin.append(o.index)
    last_page_size = len(data.image) - (data.pages - 1) * PAGE

    # Loader section: object table, page map, resident names, entry table;
    # then the fixup section: page table, records, import module and
    # procedure tables (none; the procedure table is one empty byte).
    obj_table = b"".join(struct.pack("<6I", o.vsize, o.base, o.flags, o.first_page, o.pages, 0) for o in objects)
    page_map = b"".join(struct.pack(">I", pg << 8) for pg in range(1, npages + 1))
    resident = bytes([len(MODULE_NAME)]) + MODULE_NAME + struct.pack("<H", 0) + b"\0"
    entry = b"\0"
    loader = obj_table + page_map + resident + entry
    fixup_section = struct.pack(f"<{npages + 1}I", *page_table) + records + b"\0"
    off_obj = HEADER_SIZE
    off_map = off_obj + len(obj_table)
    off_res = off_map + len(page_map)
    off_ent = off_res + len(resident)
    off_fpt = off_ent + len(entry)
    off_frt = off_fpt + 4 * (npages + 1)
    off_imp = off_fpt + len(fixup_section) - 1
    end = off_fpt + len(fixup_section)
    pages_at = (lh + end + FILE_ALIGN - 1) // FILE_ALIGN * FILE_ALIGN
    cstart = next(f.addr for f in proj.functions if f.name == "_cstart_")

    header = bytearray(HEADER_SIZE)
    header[0:4] = b"LE\0\0"
    struct.pack_into("<HHII", header, 0x8, 2, 1, 0, 0x200)  # 386, OS/2, version 0, flags
    struct.pack_into("<5I", header, 0x14, npages, code.index, cstart - code.base, data.index, data.vsize)
    struct.pack_into("<3I", header, 0x28, PAGE, last_page_size, len(fixup_section))
    struct.pack_into("<I", header, 0x38, end - off_obj)
    struct.pack_into("<4I", header, 0x40, off_obj, len(objects), off_map, 0)
    struct.pack_into("<3I", header, 0x50, off_res, 0, off_res)
    struct.pack_into("<I", header, 0x5C, off_ent)
    struct.pack_into("<5I", header, 0x68, off_fpt, off_frt, off_imp, 0, off_imp)
    struct.pack_into("<I", header, 0x80, pages_at)
    struct.pack_into("<I", header, 0x94, data.index)

    # The DOS/4GW stub is the original's: everything before the LE header,
    # whose offset its 0x3C field holds.
    stub = d[:lh]
    out = bytearray(stub + header + loader + fixup_section)
    out += bytes(pages_at - len(out))
    for o in (code, code16, data):
        out += o.image + bytes(o.pages * PAGE - len(o.image))
    del out[pages_at + (npages - 1) * PAGE + last_page_size :]

    open(output, "wb").write(out)
    diff = [k for k in range(max(len(out), len(d))) if k >= len(out) or k >= len(d) or out[k] != d[k]]
    print(f"wrote {output}: {len(out)} bytes (original {len(d)}), {len(diff)} differ")
    print(f"data placed by units {sum(downer):#x} bytes ({sum(downer[:init]):#x} initialised),"
          f" {len(dfixups) - len(unowned_fixups)} fixups"
          f" ({unknown} unit relocations unresolved)")
    print(f"data from the original: {len(unowned):#x} nonzero bytes of {init - sum(downer[:init]):#x} unowned,"
          f" {len(unowned_fixups)} fixups")
    rec_at = lh + off_frt
    areas = [("DOS stub", 0, lh), ("LE header", lh, lh + off_obj), ("object table", lh + off_obj, lh + off_map),
             ("page map", lh + off_map, lh + off_res), ("names and entry table", lh + off_res, lh + off_fpt),
             ("fixup page table", lh + off_fpt, rec_at)]
    names = {code.index: "code", code16.index: "CODE16", data.index: "data"}
    for pg, oi in enumerate(origin):
        areas.append((f"{names[oi]} fixup records", rec_at + page_table[pg], rec_at + page_table[pg + 1]))
    areas.append(("import tables and padding", rec_at + page_table[-1], pages_at))
    at = pages_at
    for o in (code, code16, data):
        areas.append((f"{names[o.index]} pages", at, at + o.pages * PAGE))
        at += o.pages * PAGE
    counts = collections.Counter({a: 0 for a, _, _ in areas})
    for k in diff:
        a = next((a for a, lo, hi in areas if lo <= k < hi), "past the end")
        counts[a] += 1
    for a, n in counts.items():
        print(f"  {a}: {n}")
    for a, n in unfilled[:10]:
        print(f"  unfilled gap {code.base + a:#x} +{n:#x}")
    return not diff


if __name__ == "__main__":
    sys.exit(0 if main(*sys.argv[1:]) else 1)
