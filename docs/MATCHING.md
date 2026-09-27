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
| `movsx r,word ptr [g]` on a global | the global is `volatile` (plain `s16` gives `mov r,[g-2]; sar r,16`) |
| reloc at `g-2`/`g-3` from the `sar` trick | give `config/symbols.txt` a symbol starting there and access `g` as a member of it |
| `mov al,[eax+T]; and eax,0xff; test eax,eax` | explicit `T[i] != 0` / `== 0` (a plain truth test gives `cmp byte ptr`) |
| `jne next; jmp fail` inside an `&&` chain | parenthesised sub-chain: `X && (P && E) && Q` |
| outer condition jumping past the whole `if` | nested `if (A) { if (B && C) ... }` |
| jump into a trampoline landing in an outer loop | `goto label` at the end of the outer loop body (`break`/`continue` add an extra `jmp`) |
| jump table whose gaps point at `default` | list the gap values as explicit `case`s next to `default:` |
| varargs call with `mov eax,N; push eax` rather than `push N` | fixed parameter declared `u8` |
| `rep movsd` from a global into a local | struct copy from an extern struct (`Message msg = D_00129004;`) |
| signed `jle` on two `u8` operands | `(int)a > (int)b` (without casts: byte compare, `jbe`) |

Stack frame layout is not fully understood. Observed so far:

- 4-byte locals are placed in reverse declaration order; with a 4-byte first
  parameter the whole frame (parameter spills included) is reversed, with a
  `u16` first parameter it is in forward order.
- The return-value slot comes after the parameters; a local that must sit
  below it can be declared inside an inner block
  (`else { T x; ... }`).
- `u8` locals follow neither declaration nor first-use order (unsolved).

## Known unsolved

- Register choice when combining an index with a pointer or table: the
  original computes the value in `eax` and the address in `edx`
  (`imul eax,eax,K; mov edx,[p]; add eax,edx`, `imul edx,[i],6; mov
  [edx+T],eax`), we get the reverse. It is common (about a third of the
  non-matching functions in 0x10000-0x14000) and depends on earlier code in
  the function: the same expression comes out right in some places. Watcom
  10.5, 10.5a, 10.6 and 11.0 and ~40 flag combinations all behave the same.
  Open Watcom 1.x is untested.
- Ordering of `u8` locals in the frame.
