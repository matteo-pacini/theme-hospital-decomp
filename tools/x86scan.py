"""Instruction-level scanning of LE code, aware of data dwords embedded in code.

Watcom places switch jump tables inside functions; every table entry carries an
LE fixup, so any fixup site that is not an operand of the instruction being
decoded is treated as a 4-byte data word.
"""

from dataclasses import dataclass

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_MEM, X86_OP_REG, X86_REG_EIZ

_md = Cs(CS_ARCH_X86, CS_MODE_32)

TERMINAL = {"ret", "retf", "iretd", "jmp"}

_nop_md = Cs(CS_ARCH_X86, CS_MODE_32)
_nop_md.detail = True


def _is_nop(ins):
    """True for instructions the linker and compiler use as alignment filler:
    nop, mov r,r, and lea r,[r+0] in any encoding."""
    if ins.mnemonic == "nop":
        return True
    ops = ins.operands
    if ins.mnemonic == "mov" and len(ops) == 2 and ops[0].type == ops[1].type == X86_OP_REG:
        return ops[0].reg == ops[1].reg
    if ins.mnemonic == "lea" and len(ops) == 2 and ops[1].type == X86_OP_MEM:
        mem = ops[1].mem
        return mem.base == ops[0].reg and mem.index in (0, X86_REG_EIZ) and mem.disp == 0
    return False


def is_padding(data):
    """True if data is alignment filler: zero bytes and no-op instructions."""
    i = 0
    while i < len(data):
        if data[i] == 0:
            i += 1
            continue
        ins = next(_nop_md.disasm(data[i : i + 15], 0), None)
        if ins is None or i + ins.size > len(data) or not _is_nop(ins):
            return False
        i += ins.size
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
