# Matching guide

What is known about how `HOSPITAL.EXE` was built, and the source idioms that
reproduce its code. Everything here was established by compiling test cases
and comparing against the original.

## Toolchain facts

- Compiler: Watcom C++ (`wpp386`) 10.5 or later. 10.0a is ruled out (it loads
  signed chars with `movsx` where the game uses a dword load and `sar`); 10.5,
  10.5a and 10.6 generate identical code for everything tested. The project
  uses 10.6.
- Flags: `-d2 -zm -s` (see `tools/cc.py`). `-d2` both disables optimisation
  and keeps dead loads such as the value of a discarded `i++`, which the game
  has. `-zm` puts each function in its own segment; the linker then aligns
  every function to 4 bytes, padding with `nop`, `mov eax,eax` or
  `lea eax,[eax+0]`.
- The game is C++: `wcc386` (C) generates different code for the same source.
- Default calling convention is Watcom register: arguments in `eax`, `edx`,
  `ebx`, `ecx`, then the stack (callee pops, `ret N`). Varargs functions take
  everything on the stack and the caller pops.
- Structs are packed to 1 byte (`struct {u32; u8; s8;}` is 6 bytes).
- Enums are the smallest signed type that holds their values (usually 1 byte).
- `char` is unsigned.

## Conventions

- Declare everything `extern "C"` so symbols do not depend on C++ mangling.
  Unnamed functions are `fn_XXXXXXXX` (symbol `fn_XXXXXXXX_`), unnamed data
  `D_XXXXXXXX` (symbol `_D_XXXXXXXX`), both named after their address.
- Put `// FUNCTION: 0xXXXXXXXX` on the line above each function definition.
- Reference string literals and other constant data through `extern char
  D_XXXXXXXX[]` rather than writing the literal: a literal would be emitted
  into this object's own `CONST` segment, and the reloc would not match.
- If code indexes into a table or struct, the table needs an entry in
  `config/symbols.txt` (`address symbol size`, hex) so that references to its
  interior are named `symbol+offset` in the target, as the compiler names
  them. `tools/fn.py` shows `_D_XXXXXXXX` names for interior addresses when
  the entry is missing.

## Tools

- `tools/fn.py 0xADDR`: disassembly with relocated operands named as source
  must name them, then rz-ghidra pseudo-C (treat the latter as a hint only).
- `tools/check.py src/FILE.cpp`: builds just that file's unit in a temporary
  directory and prints each function's match with differing instructions.
  Safe to run concurrently.

## Idioms

| Target code | Source |
| --- | --- |
| `xor eax,eax; mov al,[g]; cmp eax,2` | `u8`/`char` variable compared with a constant |
| `cmp byte ptr [g],2` | enum-typed variable compared with an enumerator |
| `cmp byte ptr [g],0; jne` | truth test of a `u8` (`if (g)`) |
| `mov eax,[x+2]; sar eax,0x18; test eax,eax; jl` | `s8` field at `x+5` tested `>= 0` |
| `mov ax,[..]; and eax,0xffff` | `u16` field promoted to `int` (e.g. varargs argument) |
| `mov al,[..]; and eax,0xff` | narrowing cast `(u8)` of a wider lvalue |
| `mov eax,[i]; inc [i]` in a loop header | `for (...; ...; i++)` (dead load kept by `-d2`) |
| `mov byte ptr [ebp-N],v; jmp epilogue` ... `mov al,[ebp-N]` | `return v;` (the slot is the compiler's return value, not a local) |
| unsigned `jb`/`jae` against a constant | unsigned variable or comparison |
| `if (a) X else Y` | Watcom keeps source order: the `then` block comes first, the jump goes to `else` |

Stack frame, from `ebp-4` downwards: spilled register parameters (the last
parameter at `ebp-4`), then the return-value slot if any, then locals in
declaration order.

## Known unsolved

- A store whose value is computed in `eax` with the index in `edx`
  (`imul edx,[i],6; mov [edx+T],eax`) instead of the reverse. It occurs three
  times in the game code (e.g. `fn_00010618`, `fn_00010760`); no source
  variant tried reproduces it.
