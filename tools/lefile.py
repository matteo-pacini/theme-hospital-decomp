"""Reader for the Linear Executable (LE) image inside HOSPITAL.EXE.

Only what the decomp tooling needs: object table, loaded object bytes and
internal fixups resolved to linear addresses. Addresses are the linear
addresses the LE header assigns (code object 1 is based at 0x10000).
"""

import struct
from dataclasses import dataclass

# Fixup source types (low nibble of the source byte).
SRC_OFFSET16 = 0x05
SRC_OFFSET32 = 0x07
SRC_SELF_REL32 = 0x08
SRC_SELECTOR16 = 0x02


@dataclass
class LEObject:
    index: int  # 1-based, as used by fixup records
    base: int
    vsize: int
    flags: int
    data: bytes  # loaded bytes, zero-padded to vsize

    @property
    def is_code(self):
        return bool(self.flags & 0x4)

    def contains(self, addr):
        return self.base <= addr < self.base + self.vsize


@dataclass
class Fixup:
    site: int  # linear address of the patched field
    kind: int  # SRC_* constant
    target: int | None  # linear address, None for selector fixups
    target_obj: int


class LEFile:
    def __init__(self, path):
        self.raw = open(path, "rb").read()
        d = self.raw
        le = struct.unpack_from("<I", d, 0x3C)[0]
        if d[le : le + 2] != b"LE":
            raise ValueError(f"{path}: no LE header")
        self._le = le
        hdr = lambda off: struct.unpack_from("<I", d, le + off)[0]
        page_size = hdr(0x28)
        last_page_size = hdr(0x2C)
        n_pages = hdr(0x14)
        data_pages = hdr(0x80)
        self.entry_obj = hdr(0x18)
        self.entry_eip = hdr(0x1C)

        self.objects = []
        for i in range(hdr(0x44)):
            vsize, base, flags, page_index, n_obj_pages, _ = struct.unpack_from(
                "<6I", d, le + hdr(0x40) + i * 24
            )
            buf = bytearray(vsize)
            for p in range(n_obj_pages):
                page = page_index - 1 + p
                size = last_page_size if page == n_pages - 1 else page_size
                src = data_pages + page * page_size
                chunk = d[src : src + size][: max(0, vsize - p * page_size)]
                buf[p * page_size : p * page_size + len(chunk)] = chunk
            self.objects.append(LEObject(i + 1, base, vsize, flags, bytes(buf)))

        self.fixups = {}  # site -> Fixup
        fpt = le + hdr(0x68)
        frt = le + hdr(0x6C)
        for obj in self.objects:
            n_obj_pages = (obj.vsize + page_size - 1) // page_size
            first_page = struct.unpack_from("<6I", d, le + hdr(0x40) + (obj.index - 1) * 24)[3] - 1
            for p in range(n_obj_pages):
                page = first_page + p
                if page >= n_pages:
                    break
                start, end = struct.unpack_from("<2I", d, fpt + page * 4)
                self._parse_page_fixups(d, frt + start, frt + end, obj.base + p * page_size)

    def _parse_page_fixups(self, d, i, end, page_addr):
        while i < end:
            src, flags = d[i], d[i + 1]
            i += 2
            if src & 0x20:
                raise NotImplementedError("fixup source lists")
            if flags & 0x03:
                raise NotImplementedError(f"non-internal fixup flags {flags:#x}")
            site_off = struct.unpack_from("<h", d, i)[0]
            i += 2
            if flags & 0x40:
                obj = struct.unpack_from("<H", d, i)[0]
                i += 2
            else:
                obj = d[i]
                i += 1
            kind = src & 0x0F
            target = None
            if kind != SRC_SELECTOR16:
                if flags & 0x10:
                    target = struct.unpack_from("<I", d, i)[0]
                    i += 4
                else:
                    target = struct.unpack_from("<H", d, i)[0]
                    i += 2
                target += self.objects[obj - 1].base
            if flags & 0x04:
                i += 4 if flags & 0x20 else 2
            site = page_addr + site_off
            # A field straddling a page boundary is recorded on both pages.
            self.fixups[site] = Fixup(site, kind, target, obj)

    def object_at(self, addr):
        for obj in self.objects:
            if obj.contains(addr):
                return obj
        return None

    def read(self, addr, size):
        obj = self.object_at(addr)
        off = addr - obj.base
        return obj.data[off : off + size]

    @property
    def code(self):
        return self.objects[0]
