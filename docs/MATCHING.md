# Matching guide

What is known about how `HOSPITAL.EXE` was built, and the source idioms that
reproduce its code. Everything here was established by compiling test cases
and comparing against the original.

## Toolchain facts

- The 1997 demo build (optimised) is reproduced exactly by Watcom 10.5,
  10.5a and 10.6 with `-s -oneatx`: our retail source recompiled that way
  matched the demo byte for byte in 9 of 10 functions tested (11.0 matched
  2), which confirms both the compiler family and that the decompiled
  source is faithful to the original.
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

- Functions and globals the 1997 demo names (see README) use their original
  names and must mangle to exactly the symbol in `config/functions.txt` /
  `config/symbols.txt`: write the real C++ declaration, including top-level
  `const` on parameters (Watcom mangles it), `volatile`, and the original
  class/enum names. `tools/fn.py` prints the demangled declaration and
  `owdemangle` demangles any symbol. A wrong signature shows up in
  `tools/check.py` as "not found in compiled object".
- Some functions changed signature between the demo and retail (e.g.
  `open_requester` takes two arguments in retail, one in the demo), and a few
  demo globals are used differently. Such a function keeps an address name in
  source and is bound to the target's symbol with
  `#pragma aux fn_XXXXXXXX "W?..."` in `include/hosp.h`, so relocations still
  match.
- Everything else is `extern "C"` and named after its address: functions
  `fn_XXXXXXXX` (symbol `fn_XXXXXXXX_`), data `D_XXXXXXXX` (symbol
  `_D_XXXXXXXX`).
- One source file per original module (`src/<module>.cpp`, as named by the
  demo's debug info); shared types live in `include/hosp.h` (game) and
  `include/bflib.h` (Bullfrog library types, globals and functions).
- Never declare functions or data `static`, even where the original did:
  objconv drops Watcom's local-symbol records, so a static function would
  vanish from the compiled object. The mangled name does not change.
- Put `// FUNCTION: 0xXXXXXXXX` on the line above each function definition.
- Reference string literals and other constant data through `extern`
  declarations rather than writing the literal: a literal would be emitted
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
| reloc at `g-2`/`g-3` from the `sar` trick | nothing special: the splitter names such reads after the variable read (`g-2`), as the compiler does |
| `mov al,[eax+T]; and eax,0xff; test eax,eax` | explicit `T[i] != 0` / `== 0` (a plain truth test gives `cmp byte ptr`) |
| `jne next; jmp fail` inside an `&&` chain | parenthesised sub-chain: `X && (P && E) && Q` |
| outer condition jumping past the whole `if` | nested `if (A) { if (B && C) ... }` |
| jump into a trampoline landing in an outer loop | `goto label` at the end of the outer loop body (`break`/`continue` add an extra `jmp`) |
| jump table whose gaps point at `default` | list the gap values as explicit `case`s next to `default:` |
| varargs call with `mov eax,N; push eax` rather than `push N` | fixed parameter declared `u8` |
| `rep movsd` from a global into a local | struct copy from an extern struct (`Message msg = D_00129004;`) |
| signed `jle` on two `u8` operands | `(int)a > (int)b` (without casts: byte compare, `jbe`) |
| `u8`/`u16` field: `and`+`test` vs `cmp byte/word ptr ..,0` | `x == 0` vs `!x` / `if (x)` |
| lone `cmp eax,1` followed by unrelated code | empty `if (g == 1) {}` |
| `u16` local set/incremented with dword ops, read with `xor eax,eax; mov ax` | ordinary `u16` local |
| `switch` on `u8`: value copied to two byte temps, compare tree | `switch (u8)`; on `u16` with a jump table the temps are dwords |
| `mov eax,[base+k-1]; sar eax,0x18` | `(s8)w` of a `u16` field at offset `k` |
| `mov [tmp],eax; mov eax,[tmp]; push eax` around a call result | result assigned to a named local before the varargs call |
| 16-bit `imul ax,word ptr [..]` | `u16 = u16 * u16` |
| `jmp` back to a switch head | `goto` to a label on the `switch` |
| `&&`: false first condition via a separate `jmp end` stub | `a && b`; nested `if`s jump straight to the end |

| `test byte ptr [x+0x75],4`, `and byte ptr [x+0x74],0xef` | `u16` flags field: `flags & 0x400`, `flags &= ~0x10` (masks spanning both bytes stay 16-bit) |
| `mov al,[ebp-3]` (high byte of a parameter) | 2-byte union parameter, e.g. `union { u16 raw; struct { u8 frac; u8 tile; }; }` |
| `shl 5; sar 5` on a bitfield | `signed int x : 27` (plain `int` bitfields are unsigned) |
| `mov [tmp],edx; neg dword ptr [tmp]` | `x < 0 ? -(x) : (x)` (an ABS macro) |
| `inc eax; mov al,[eax]` | `*(u8 *)((u32)p + 1)` |
| `mov eax,[p]; add [p],2` on a `u8 *` | `(*(u16 **)&p)++` |
| `xor ah,ah; and eax,0xffff` on an argument | explicit `(u16)` cast of a `u8` |
| `test ax,ax` on a call result | bare truth test of a `u16` return |
| store, load, store, load, store | chained assignment `a = b = c = 0` |
| every `return 0` jumps to one block | `break` out of the loop, single `return 0;` after it |
| each `&&` test jumping straight to the false label | nested `if`s; `a && b && c && d` may need `(a && b) && (c && d)` |
| Watcom varargs prologue | `typedef char *va_list[1]` with a manual `va_start`/`va_end` (`ap[0] = 0`) |

| `[prev_global]>>16` read for a 16-bit global | `s32` alias of the preceding global bound with `#pragma aux`, written `(alias >> 16)`; a struct-member alias changes evaluation order |
| dword load and `sar` on a `u16` field | `(s16)p->field` |
| `if (!x) break;` in a switch case: one `je` to the switch end | as written; an if-block then `break` adds a `jmp` stub |
| `mov eax,[g]; xor eax,eax; mov [g],eax` | `g ^= g` |
| `A \|\| (B \|\| C)`: tail jumps straight to the then-block | explicit grouping; `(A\|\|B)\|\|C` emits `jne next; jmp then` |
| byte temporary for `c ? 1 : 0` | `c ? (u8)1 : (u8)0` (plain constants give a dword temporary) |
| switch compare tree grouping | consecutive case values merge into a range only when their labels are adjacent in source |
| nested `if (a) { if (b) { ...; break; } }` in a case | reproduces tests jumping straight to the next test; an else-if chain does not |
| frame 8 bytes off with word stores | check array sizes (`char buf[500]`, not `[0x200]`) |
| map index `movsx y; shl 7; movsx x; add` | `&map[x + (y << 7)]`; with `u8` coordinates `map[(s16)x + ((s16)y << 7)]` |
| u8 tile expression in 32-bit ops passed as `u16` | `(s16)(x + (y << 7))` (without the cast Watcom narrows to 16-bit arithmetic) |
| `add eax,dword ptr [ptrglobal]` | integer addition `(T *)(off + (u32)ptr)`; `(u8 *)ptr + off` loads the pointer first |
| `test dl,0xff` after a 32-bit subtraction | `!((a - b) & 0xff)` |
| `add byte ptr [x],0xf7` / `add dword ptr [x],-1` | `x += -9` / `--x` |
| loop increment after the body, no initial jump to the test | `for (;;) { ...; i++; }` |
| `ror eax,13` | `#pragma aux` inline code: `u32 ror13(u32); #pragma aux ror13 = "ror eax,13" parm [eax] value [eax];` (`_lrotr` gives `ror eax,cl`) |
| `cmp X; jle; X = K; tmp = X; ... else tmp = 0` | discarded clamp `pct > 100 ? pct = 100 : 0;` |
| or-chain whose true result jumps to the `else` block | `if (!(a == 1 \|\| a == 2 ...)) {...} else {...}` |
| local stored directly from both arms | `if`/`else` assignment (a `?:` goes through a temporary) |

C++ `inline` functions are unusable: with `-d2` Watcom emits them as COMDATs,
which objconv rejects.

Operand order of commutative operators decides the result register
(`a + (b << 7)` and `(b << 7) + a` differ; `pos + D[i]` vs `D[i] + pos`).

Adding a table to `config/symbols.txt` renames references to its interior in
every file. Check that no other source refers to an address it covers by its
raw name, and re-run `tools/check.py` on all files afterwards.

Stack frame layout is not fully understood. Observed so far:

- 4-byte locals are placed in reverse declaration order; with a 4-byte first
  parameter the whole frame (parameter spills included) is reversed, with a
  `u16` first parameter it is in forward order.
- The return-value slot comes after the parameters; a local that must sit
  below it can be declared inside an inner block
  (`else { T x; ... }`).
- Arrays and structs are allocated after all scalars of their block, nested
  blocks included; to place a scalar below an array, declare it in a block
  entered after the array's block.
- Locals of an inner block are allocated when codegen reaches the block, so
  they land below compiler temporaries created earlier; declaring a buffer
  inside the `if` that uses it moves it down.
- Slot assignment is positional: it depends on the sequence of declared
  types, not on names or how variables are used. Swapping two same-typed
  declarations swaps their slots, so read the target-to-ours slot pairs off
  `tools/check.py -v` and reorder declarations to match (this also settles
  `u8` locals). The return-value slot does not move this way.
- `switch (p->field)` without a local copy puts its first byte temporary
  among the locals and the second after them.

## Known unsolved

- Register choice when combining an index with a pointer or table: the
  original computes the value in `eax` and the address in `edx`
  (`imul eax,eax,K; mov edx,[p]; add eax,edx`, `imul edx,[i],6; mov
  [edx+T],eax`), we get the reverse. It is common (about a third of the
  non-matching functions in 0x10000-0x14000) and depends on earlier code in
  the function: the same expression comes out right in some places. Watcom
  10.5, 10.5a, 10.6 and 11.0 and ~40 flag combinations all behave the same.
  Register allocation works per basic block: a later statement in the same
  block changes earlier choices, and splitting the block (a label, `do {}
  while (0)`) can flip them, but no plausible source form found so far
  reproduces the original throughout. Open Watcom 1.9 is further off
  (7/34 matched in 0x10000-0x14000 with `-zp1`, versus 22/34 with 10.6),
  and the DOS-hosted 10.6 compiler (`binw`, run under DOSBox-X) generates the
  same code as the NT-hosted one.
  Scoring 14 register-choice functions against retail with about 70 extra
  flags on top of `-s -d2` (and `-od`/`-d1`/`-d3` bases) changed nothing
  for the better, so the difference is most likely a 10.x patch build we
  do not have.

  Minimal test case, `plot_bid_money` (0x68480): the original computes the
  scaled index into a fresh register, which leaves `eax` for the value;
  every available compiler shifts in place instead. Any `-o` flag produces
  the `lea`, but also drops the dead loads that `-d2` keeps:

      original:  lea edx,[eax*4]; mov eax,[price]; mov [edx+bids],eax
      ours:      shl eax,2;       mov edx,[price]; mov [eax+bids],edx
- Position of the return-value slot relative to locals in some functions.

## Libraries

Compiler and flags per source file come from `config/cflags.txt`.

- **Miles Sound System** (`ail*.c`, `memfile.c`; RAD Game Tools): C,
  `wcc386 -s -ox`, public functions declared `__cdecl` (symbols `_AIL_...`,
  arguments on the stack). Matches exactly with every 10.x compiler. Sources
  go in `src/lib/ail/`, shared declarations in `include/ail.h`.
- **Bullfrog LIBRARY7** (`g*.cpp`, `s*.cpp`, ...): optimised C++ whose
  leaf functions keep a frame (`push ebp; mov ebp,esp; ...; pop ebp`) but
  without the `mov esp,ebp` every available compiler emits with `-of+`, so it
  was likely built with an earlier Watcom than any we have.
- **Assembly modules** (`*.asm` in the demo's debug info: `gspr`, `reqspr`,
  `spr_scl`, `hvline`, `tri_f`, `arctan`, `sqroot`, `box_clip`, `scr_copy`,
  `rnc_1fm`, Miles' `aila`/`ailssa`): kept as GNU as Intel-syntax source in
  `src/asm/` and `src/lib/ail/`, generated by `tools/asm2src.py` with labels,
  symbolic relocations and the original assembler's encodings (`{load}` forms,
  or bytes where as cannot express them). They match exactly.
- **Bullfrog network library** (`sp.cpp`, `netint*.cpp`, `dosser.cpp`, ...):
  optimised C++ classes, flags not yet established.
