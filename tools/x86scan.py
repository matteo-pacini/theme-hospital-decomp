"""Instruction-level scanning of LE code, aware of data dwords embedded in code.

Watcom places switch jump tables inside functions; every table entry carries an
LE fixup, so any fixup site that is not an operand of the instruction being
decoded is treated as a 4-byte data word.
"""

from dataclasses import dataclass

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

_md = Cs(CS_ARCH_X86, CS_MODE_32)

TERMINAL = {"ret", "retf", "iretd", "jmp"}

# Filler emitted between 4-byte aligned functions: nop, mov eax,eax,
# lea eax,[eax+0], and zero bytes.
PADDING = (b"\x8d\x40\x00", b"\x8b\xc0", b"\x90", b"\x00")


def is_padding(data):
    i = 0
    while i < len(data):
        for p in PADDING:
            if data.startswith(p, i):
                i += len(p)
                break
        else:
            return False
    return True


@dataclass
class Item:
    addr: int
    size: int
    mnemonic: str | None  # None for an embedded data dword
    branch_target: int | None = None  # rel32 call/jmp/jcc destination
    branch_field: int | None = None  # address of the rel32 field

    @property
    def end(self):
        return self.addr + self.size

    @property
    def terminal(self):
        return self.mnemonic in TERMINAL


def scan(le, start, end):
    """Decode [start, end) of the code object into Items."""
    code = le.code
    items = []
    a = start
    while a < end:
        if a in le.fixups:
            items.append(Item(a, 4, None))
            a += 4
            continue
        off = a - code.base
        ins = next(_md.disasm(code.data[off : off + 16], a), None)
        if ins is None:
            items.append(Item(a, 1, "(bad)"))
            a += 1
            continue
        item = Item(a, ins.size, ins.mnemonic)
        b = ins.bytes
        rel32 = b[0] in (0xE8, 0xE9) or (b[0] == 0x0F and 0x80 <= b[1] <= 0x8F)
        if rel32 and ins.size >= 5 and not any(f in le.fixups for f in range(a, a + ins.size)):
            item.branch_target = int(ins.op_str, 16)
            item.branch_field = a + ins.size - 4
        items.append(item)
        a += ins.size
    return items


def trimmed_end(le, start, next_start):
    """End of the function at `start`, excluding alignment padding before `next_start`.

    Padding is a run of PADDING filler following a terminal instruction.
    Returns next_start if no such tail exists.
    """
    data = le.code.data
    base = le.code.base
    items = scan(le, start, next_start)
    for it in items:
        if not it.terminal:
            continue
        tail = data[it.end - base : next_start - base]
        if is_padding(tail):
            return it.end
    return next_start
