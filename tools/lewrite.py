#!/usr/bin/env python3
"""Write the executable's code from the units and compare the file with the
original byte for byte.

The code object comes from tools/relink.py's placement of every unit at its
original address. Gaps between functions are filled as the original linker
filled them: with the no-op filler sequence for the gap's length between two
segments of one object, with zeros between objects (and throughout the
objects that were zero-padded). A relocated field holds the target's offset
within its object, as LE pages do. The code pages' fixup records are
generated from the units' relocations in the order the original linker
processed them, packed per page into 512-byte blocks written newest first.

Still taken from the original: the 16-bit code and data objects' pages and
fixup records, and the header and loader tables around them.

Usage: tools/lewrite.py [OUTPUT]    (default build/hospital.exe; after `ninja`)
"""

import collections
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
import relink  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, ROOT  # noqa: E402

# The no-op sequences the original linker filled a gap of each length with.
FILLER = {n: bytes.fromhex(h) for n, h in {
    1: "90", 2: "8bc0", 3: "8d4000", 4: "8d442000", 5: "8d40008bc9",
    6: "8d8000000000", 7: "8d800000000090", 8: "8d80000000008bd2",
    9: "8d80000000008d5200", 10: "8d80000000008d542200",
    11: "8d80000000008d52008bdb", 12: "8d80000000008d9200000000",
    13: "8d80000000008d920000000090", 14: "8d80000000008d92000000008bc0",
    15: "8d80000000008d92000000008d4000"}.items()}
BLOCK = 512  # the linker's fixup record block size


def fixup_record(page_offset, target_obj, target_off):
    """An internal 32-bit offset fixup, as the original encodes it."""
    flags = 0x10 if target_off > 0xFFFF else 0
    tail = struct.pack("<I", target_off) if flags else struct.pack("<H", target_off)
    return bytes([0x07, flags]) + struct.pack("<h", page_offset) + bytes([target_obj]) + tail


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


def main(output=os.path.join(ROOT, "build", "hospital.exe")):
    le = LEFile(EXE)
    result = {}
    relink.main(result=result)
    code = le.code
    image, owner = bytearray(result["image"]), result["owner"]

    # Gaps between placed bytes.
    i, unfilled = 0, []
    while i < code.vsize:
        if owner[i] is not None:
            i += 1
            continue
        j = i
        while j < code.vsize and owner[j] is None:
            j += 1
        before, after = owner[i - 1] if i else None, owner[j] if j < code.vsize else None
        n = j - i
        if before is not None and before == after and before not in result["zero_padded"] and n in FILLER:
            image[i:j] = FILLER[n]
        elif n >= 16:
            unfilled.append((code.base + i, n))
        i = j

    # Code fixups: raw values and records.
    d = le.raw
    lh = struct.unpack_from("<I", d, 0x3C)[0]
    hdr = lambda off: struct.unpack_from("<I", d, lh + off)[0]
    psz, npages = hdr(0x28), hdr(0x14)
    per_page = collections.defaultdict(list)
    for site, target in result["code_fixups"]:
        # An object's end (an end-of-segment label) belongs to it too.
        tobj = next(o for o in le.objects if o.base <= target <= o.base + o.vsize)
        off = target - tobj.base
        struct.pack_into("<I", image, site - code.base, off)
        rel = site - code.base
        for pg in sorted({rel // psz, (rel + 3) // psz}):
            if pg * psz <= rel + 3 and rel < (pg + 1) * psz:
                per_page[pg].append((rel - pg * psz, fixup_record(rel - pg * psz, tobj.index, off)))

    fpt, frt = lh + hdr(0x68), lh + hdr(0x6C)
    code_pages = (code.vsize + psz - 1) // psz
    records, page_table = bytearray(), [0]
    for pg in range(npages):
        if pg < code_pages:
            records += page_records(per_page[pg])
        else:
            a, b = struct.unpack_from("<2I", d, fpt + pg * 4)
            records += d[frt + a : frt + b]
        page_table.append(len(records))

    out = bytearray(d)
    orig_records_end = lh + hdr(0x70)  # the import module table follows the records
    if len(records) != orig_records_end - frt:
        print(f"fixup record table {len(records):#x} bytes, original {orig_records_end - frt:#x}: layout would shift")
    else:
        out[frt : frt + len(records)] = records
        struct.pack_into(f"<{npages + 1}I", out, fpt, *page_table)
    data_pages = hdr(0x80)
    for pg in range(code_pages):
        chunk = image[pg * psz : (pg + 1) * psz]
        out[data_pages + pg * psz : data_pages + pg * psz + len(chunk)] = chunk

    open(output, "wb").write(out)
    diff = [k for k in range(len(d)) if out[k] != d[k]]
    print(f"wrote {output}: {len(out)} bytes, {len(diff)} differ from the original")
    areas = collections.Counter()
    for k in diff:
        if k < lh:
            areas["DOS stub"] += 1
        elif k < fpt:
            areas["LE header and tables"] += 1
        elif k < frt:
            areas["fixup page table"] += 1
        elif k < orig_records_end:
            areas["fixup records"] += 1
        elif k < data_pages:
            areas["loader tables"] += 1
        elif k < data_pages + code_pages * psz:
            areas["code pages"] += 1
        else:
            areas["other pages"] += 1
    for a, n in areas.items():
        print(f"  {a}: {n}")
    for a, n in unfilled[:10]:
        print(f"  unfilled gap {a:#x} +{n:#x}")
    bad_pages = sorted({(k - fpt) // 4 for k in diff if fpt <= k < frt})
    rec_pages = [pg for pg in range(npages) if page_table[pg + 1] - page_table[pg] != struct.unpack_from("<2I", d, fpt + pg * 4)[1] - struct.unpack_from("<2I", d, fpt + pg * 4)[0]]
    print(f"  pages whose fixup records differ in size {len(rec_pages)}")
    return not diff


if __name__ == "__main__":
    sys.exit(0 if main(*sys.argv[1:]) else 1)
