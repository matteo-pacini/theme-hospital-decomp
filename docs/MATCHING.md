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
- Compiler: Watcom C++ (`wpp386`) 10.5 or later; "register choice" near-misses
  that predate the `-ot` discovery were recompiled and most now match. 10.0a is ruled out (it loads
  signed chars with `movsx` where the game uses a dword load and `sar`); 10.5,
  10.5a and 10.6 generate identical code for everything tested. The project
  uses 10.6.
- Part of the C runtime was built with 10.0a: the printf and stdio modules
  (`fprtf`, `fputs`, `prtf`, `tzset`) match only under it, since 10.6 keeps
  a dead byte store and a `setne` mask that 10.0a does not. A
  `config/cflags.txt` entry selects it as `wcc386@10.0a`. Everything else
  in the runtime is as good or better under 10.6, and 11.0 is worse
  throughout.
- Flags: `-s -ot -d2 -zm` (see `config/cflags.txt`). Order matters: `-d2`
  turns optimisation off again, but the "optimise for time" preference set by
  the earlier `-ot` survives and decides register allocation (the value of a
  store in `eax`, a scaled index built with `lea` into a fresh register).
  With `-d2` alone the allocator prefers the opposite registers. `-d2` both disables optimisation
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
- `tools/libunit.py SOURCE LIBRARY MODULE`: checks a Watcom library module
  against a unit (see Watcom library modules).
- `tools/setfn.py 0xADDR NAME` / `--remove`: add, rename or merge one entry
  in `config/functions.txt`, re-sizing only its neighbours.
- `tools/variants.py src/FILE 0xADDR`: searches source variants of one
  function (commutative operand swaps, declaration order, `x op= e` forms,
  and with `--kinds ...,types` local integer types) with a beam search
  scored by objdiff, and prints the best as a diff. It rediscovers, for
  example, the fixes for `item_upgraded` and `can_i_get_to` from their
  earlier source; review `types` results, which can change behaviour.

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
| list walk comparing `[p+next]`, then reloading it twice (no CSE) | pointer to volatile: `volatile T *p` (a volatile struct field instead changes store order) |
| `test eax,eax; je` then `mov ebx,eax` for a zero store | the source copies the tested variable (`last = rw` inside `if (rw == 0)`); `a = X; b = a;` vs `b = X; a = b;` picks which variable gets the load |
| `and di,0xfffc` | plain `x & ~3` narrowed on a register with no byte form; separate `x += 3; x &= ~3;` gives a byte `and` on a spilled slot |
| `-d2 -otx` loop that compares memory (`cmp [x],0`) at its head, but loads then tests (`mov r,[x]; test r,r`) inside | `for (;;) { if (cnt > 0) {...} else {...} }` rather than `while (cnt > 0)` plus a tail; `while (c)` and tests after a join compare memory |
| separate epilogues for early `return list[0]` versus one shared | nested `if/else` assigning a result, then a single `return` |
| `lea r,[g*2]; imul r,g` | `p->g * 2 * p->g` (a `SQR(g) * 2` macro does not give it) |
| `-oneatx` network init taking a `void *` | a typed pointer cast into its own local, declared before `ret` |
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
| expression result in `eax` for `mem < expr` | write the comparison with memory on the left (`f35 < a + b`) |
| single `jcc` to the epilogue for two conditions | nested `if (a) { if (b) return; }` |
| word-width `or` on a `u16` global | `*(volatile u16 *)&g \|= 0x40` (plain source narrows to a byte `or`) |
| loop tail jumping back to a decrement | the decrement is in the condition: `while (--q > start)` |
| `ror eax,13` | `#pragma aux` inline code: `u32 ror13(u32); #pragma aux ror13 = "ror eax,13" parm [eax] value [eax];` (`_lrotr` gives `ror eax,cl`) |
| `cmp X; jle; X = K; tmp = X; ... else tmp = 0` | discarded clamp `pct > 100 ? pct = 100 : 0;` |
| or-chain whose true result jumps to the `else` block | `if (!(a == 1 \|\| a == 2 ...)) {...} else {...}` |
| local stored directly from both arms | `if`/`else` assignment (a `?:` goes through a temporary) |
| pointer loaded before the `u16` operand | `x + p->field` with `x` a `u16` (operand order decides which is loaded first) |
| extra temp slot for a returned `?:` | `if (a > b) return a; return b;` |
| switch value computed without a named local | `switch ((u32)(mask & -mask))` |
| value computed, stored, then compared | assignment in the condition: `if ((amount = expr) < lim)` |
| `je` to a `return` placed after the loop tail | `if (--rows != 0) { ...; goto L; } return 1;` |
| array index chosen by a condition, no local | `?:` in the subscript: `a[st == 4 ? 1 : st]--` |
| range checks grouped within an `&&` chain | left-nested grouping `(((a && b) && c) && d)` |
| `jmp` over the tail when one arm has a `goto` | `if`/`else` wrapping the whole tail |
| multiplier global preloaded or not | operand position of a divide (`x / 25` first or second) |

Operand order of commutative operators decides the result register
(`a + (b << 7)` and `(b << 7) + a` differ; `pos + D[i]` vs `D[i] + pos`).
Evaluation order follows the code generator (Open Watcom `tree.c`,
`TNBinary`/`TNCompare`): the left operand is evaluated first if and only
if its expression tree has at least as many nodes as the right one's. A
plain global is a leaf plus a conversion; a struct field or array element
at a nonzero offset is a bigger tree (`POINTS(PLUS(leaf, c))`), offset 0
behaves like a plain global, and every cast adds a node. So when a global is
loaded before or after the "wrong" operand, it is often really a struct
field (or the reverse), or the grouping is wrong:

- Many `D_` globals are fields of the demo's structs, which the data map
  had shrunk; `memset` sizes in initialisation code give the real sizes.
  Decide field versus plain global from the evaluation order.
- Layout-pointer tables are indexed as arrays of elements
  (`resvars.f40[i + K].x`), not through invented structs; give per-file
  pointer types through a union of pointer members, since casts add nodes.
- The map index is `x + (y << 7)` (tile expressions too); `y` is evaluated
  first.
- `?:` in a subscript makes the index evaluate first.
- `if (u16 local)` instead of `!= 0` changes register allocation across the
  whole function.
- `!x--` on a `u16` field or element gives `mov ax; dec; test ax,ax`; on a
  plain global, `dec` then `cmp word ptr, 0xffff`.
- A jump that lands one statement later than expected means that statement
  belongs inside the `if`.
- objdiff's alignment can hide an encoding difference; compare raw bytes and
  lengths when only jumps differ.

Adding a table to `config/symbols.txt` renames references to its interior in
every file. Check that no other source refers to an address it covers by its
raw name, and re-run `tools/check.py` on all files afterwards.

Stack frame layout (traced in the 10.6 compiler itself, `binw/wpp386.exe`,
and confirmed by predicting layouts that then matched). `-d2` turns
optimisation off (`-od` gives identical code), so every function is
generated block by block and its code is flushed in chunks: after the
parameters, at every control-flow statement (`if`, loop, label, `goto`,
`case`), at a switch's selection and at `return`. At the start of each
chunk the compiler takes the names created since the previous chunk
(newest first: parameters, `.return`, function-scope locals in declaration
order, then compiler, `?:`, switch and conversion temporaries and
inner-block locals as their block opens), adds `LastTemp` (the first name
of the previous chunk's sorted list that is still live: a named local, or a
temporary used across blocks), and sorts them once by declared size,
ascending (`char[2]` before `int` before `char[5]`), with an unstable shell
sort (gap n/2, then gap/2+1, down to bubble passes at gap 1). Slots are
handed out from `ebp` downwards in sorted order, but only up to `LastTemp`;
names sorted after it get their slot at their first use in the code (in
instruction order; operands last to first, then the result), in that chunk
or a later one. Names that never get a slot still take part in the sort.

This one rule covers what looked like two layouts: with a 4-byte parameter
as `LastTemp` it sorts last, so almost the whole frame is size-sorted; with a
`u8`/`u16` parameter, or a 1-byte compiler temporary left over, it sorts
first, so the frame follows first use. The levers are declaration order
(the order of the list), the block a local is declared in (a block opened
after a chunk boundary adds its locals to that chunk, as its oldest names)
and where that block opens.

Observed consequences:

- 4-byte locals are placed in reverse declaration order; with a 4-byte first
  parameter the whole frame (parameter spills included) is reversed, with a
  `u16` first parameter it is in forward order (the first-use case above).
- A 2-byte local gets a slot before the return-value slot; scope it in an
  inner block to place it after.
- The return-value slot comes after the parameters; a local that must sit
  below it can be declared inside an inner block
  (`else { T x; ... }`).
- Arrays and structs are allocated after all scalars of their block, nested
  blocks included; to place a scalar below an array, declare it in a block
  entered after the array's block.
- Locals of an inner block are allocated when codegen reaches the block, so
  they land below compiler temporaries created earlier; declaring a buffer
  inside the `if` that uses it moves it down.
- Slot assignment is positional for same-typed locals when no `?:`
  temporaries are involved: it depends on the sequence of declared types,
  not on names or how variables are used. Swapping two same-typed
  declarations swaps their slots, so read the target-to-ours slot pairs off
  `tools/check.py -v` and reorder declarations to match (this also settles
  `u8` locals).
- `switch (p->field)` without a local copy puts its first byte temporary
  among the locals and the second after them.
- In `-ox` functions spill-slot order need not follow declaration order
  (all permutations can tie); it moves with expression shape instead: an
  assignment inside a condition (`(c = *p) == K`) swapped two slots in
  `__scnf`.
- A `void *` parameter used through casts, instead of a `char *` local
  copy, moves the callee-saved copy of the parameter before the first call.

- Reading our own frame slots: compile with `-hw` in place of `-hd` and
  scan the OMF's local-symbol records (length, 0x11, 0x10/0x11/0x12 for a
  1/2/4-byte BP offset, type index, name) to get each local's slot; pair
  them with `check.py -v`'s slot pairs to read the target slot per name.
  Block-scope locals take their slot from the declaring block. The demo's
  debug dump (`orig/demo-1997-03/HOSPITAL.debug.txt`) also has line tables
  for library modules, which show block structure.

- Registers shifting around an indirect call can come from the function
  pointer parameter's calling convention: a `__cdecl` pointer clobbers
  eax/ecx/edx at the call, which moves parameter loads after it
  (`AIL_API_quick_startup`).

## Known unsolved

- Register allocation is global to the function even at `-d2`: a change
  in one statement can move register choice in another while its own code
  stays identical. When one statement's registers will not budge, vary the
  others (operand order, `x op= e` versus `x = x op (e)`, grouping).
- A local stored with dword instructions may still be `u16`/`s16`: `-d2`
  stores 16-bit locals as dwords. Reads through `movsx word`, 16-bit
  registers, or a bare `mov cl` shift count give it away; the declared type
  also moves the local's frame slot (it sorts with the 2-byte names).
- With evaluation order fixed by tree size, the textual order of a
  commutative operator still decides which operand is the destination
  register and which is folded as a memory operand.
- `jmp then; jmp skip` stubs after an or-test come from the empty-then form
  `if (!A && !B) {} else {...}`; `for (;;) { if (c) {...} else break; }`
  gives a loop whose exits go through `jmp` stubs.
- Miles (`-s -oxat -4`, confirmed by a sweep of options, their order and
  10.0a): a value computed in eax and then copied to its home register
  (`mov edx,eax; mov eax,edx` after a call in `ailsr`, the `abs` in
  `set_sequence_tempo`/`_volume`, `branch_index`) is not reproduced by
  any source form tried.
- `netint` inlining: the original inlines `CompleteMultiPlayerExchange`
  (needs `-oe=400` here) but not `LbNetwork_Init(u32)` into its wrappers
  (inlined from `-oe=250`), so no single `-oe` reproduces both; declaring
  the exchange halves `inline` instead drops their out-of-line copies,
  which the original keeps. The two unnamed `LbNetwork_Init` wrappers at
  0xbf91c/0xbfb78 wait on this, and with them building the module
  without `-zm`.

## Assembly stand-ins

Functions that were C in the original but that no available compiler
reproduces are matched from assembly generated by `tools/asm2src.py`. They
count as matched, but not as decompiled source; keep this list complete.
`tools/standin.py SOURCE 0xADDR` makes one: the C keeps its `FUNCTION`
marker and is still compiled, and `<source>.standin.s` supplies the
function's code in the object `tools/cc.py` produces (in place, when the
module is built into one segment and the sizes agree, so jumps between
neighbours stay as the original linker resolved them). The first below
predates the tool and uses a separate file instead.

- `_Scale10V` (0xfcad8), `src/lib/clib/scale10v.s`: FDIV checks without
  `pushfd`/`popfd` (C in `scale.c`).
- `__FreeDPMIBlocks_` (0xebdb8), `RationalAlloc_` (0xebfd7) and
  `__ExpandDGROUP_` (0xec158), `src/lib/clib/grownear.standin.s`: Watcom's
  own code (identical in the 10.5 and 10.6 `CLIB3R`); the module as a whole
  is in neither library (see Watcom library modules).

## Watcom library modules

Most of the runtime is Watcom's own library code (`CLIB3R.LIB`,
`MATH387R.LIB`, `PLIB3R.LIB`), mostly 10.6's but with some 10.5 modules
(`prtf`). The flake downloads the `LIB386` of both CDs from WinWorld's
archive and provides them as `$WATCOM_LIBS_10_6` and `$WATCOM_LIBS_10_5`. A
unit whose C does not reproduce its module is built from the library
instead: its `config/cflags.txt` entry names compiler `lib` (`lib@10.5` for
the 10.5 CD), the library under `LIB386` and the module, and `tools/cc.py`
takes that
module's object (and relocates its calls to its own functions, which the
library resolved). The C stays as documentation. They count as matched, not
as decompiled; the list is the `lib` block at the top of `config/cflags.txt`.

`tools/libunit.py SOURCE LIBRARY MODULE [VERSION]` checks a module against
the EXE before switching a unit: code bytes, where each relocation lands,
and the config objdiff then needs, namely the library's names for functions
and globals and the module's own segments that it refers to
section-relative (`config/sections.txt`). Library modules are one segment,
so `tools/split.py` lays their units out as built without `-zm`.

No CD's library (10.0a, 10.5, 10.6) has the EXE's `grownear` module as a
whole: its `__AdjustAmount` rounds to 8 bytes where 10.6's rounds to 4 (the
module's other functions do match 10.5's and 10.6's), so it stays C, with
stand-ins for the three functions its C does not reproduce.

## Libraries

Compiler and flags per source file come from `config/cflags.txt`.

- **Miles Sound System** (`ail*.c`, `memfile.c`; RAD Game Tools): C,
  `wcc386 -s -ox`, public functions declared `__cdecl` (symbols `_AIL_...`,
  arguments on the stack). Matches exactly with every 10.x compiler. Sources
  go in `src/lib/ail/`, shared declarations in `include/ail.h`.
- **Bullfrog LIBRARY7** (`g*.cpp`, `s*.cpp`, ...; `src/lib/bf1/`): optimised
  C++ built with debug info, `wpp386 -s -d2 -otx`. `-d2` together with an
  explicit `-o` gives the leaf frame without `mov esp,ebp`, the parameter
  spills and the dead loads of this code (no `a`: aliasing is not relaxed).
  Bullfrog's sound and network libraries (`src/lib/bf1/opt/`) have no frames:
  `-s -oneatx -4 -xs` (`-xs` for the network code's exception frames). The
  network library was probably built without `-zm` (one function jumps into
  another's epilogue). Game code stays plain `-d2`: every `-o` set scores far
  worse there.
- Jump tables: these libraries place a switch's table at the head of the
  function's segment, before its entry point; the splitter does the same.
- **Assembly modules** (`*.asm` in the demo's debug info: `gspr`, `reqspr`,
  `spr_scl`, `tri_f`, `arctan`, `sqroot`, `box_clip`, `scr_copy`,
  `rnc_1fm`, Miles' `aila`/`ailssa`): kept as GNU as Intel-syntax source in
  `src/asm/` and `src/lib/ail/`, generated by `tools/asm2src.py` with labels,
  symbolic relocations and the original assembler's encodings (`{load}` forms,
  or bytes where as cannot express them). They match exactly.
- **Bullfrog network library** (`sp.cpp`, `netint*.cpp`, `dosser.cpp`, ...):
  optimised C++ classes, flags not yet established.
- **Watcom C runtime** (`src/lib/clib/`): the runtime's own assembly modules
  (387 emulator, `*386`, `chipd32`, FP exception hooks) go through
  `tools/asm2src.py` like the game's. For the C modules:
  - The library does not seem to have been built with `-zm`: some functions
    fall through into, or jump into the epilogue of, the next one. Give such
    a module the pseudo-flag `-zm-` in `config/cflags.txt`; it is then
    compiled into one segment and compared as one section, so its source
    must implement every function in the module's address range.
  - FP support routines that keep a frame pointer need `-of` (see
    `config/cflags.txt`); `qsort` and `stat` were built for size.
  - Character comparisons need explicit `(int)` casts: `(int)*p == 'x'`;
    ctype tests `((int)(unsigned char)(_IsTable[c] & m) != 0)`.
  - Callbacks into user code save `es`/`fs`/`gs`: declare the function
    type with `#pragma aux ... modify [es fs gs]`.
  - `#pragma aux abort aborts` drops the return and tail jump after it.
  - Stack-argument FP functions: `#pragma aux f parm routine []`.
  - Other flags, per module in `config/cflags.txt`: `-fpc` (scanf's FP
    call), `-of` for the FP formatting routines. `_Scale10V` was compiled
    with the Pentium FDIV check (`-fpd`), which 10.6 wraps in
    `pushfd`/`popfd` and the original compiler did not; writing the check
    out as `#pragma aux` inline code does not help either (any inline code
    makes `-of` add an `ebp` frame, and without `-of`, which `_Scale` needs,
    the result is stored too early). It is an assembly stand-in (below).
  - Tables read with a `cs:` override are `__based(__segname("_CODE"))`.
- **Modules built without `-zm`**: without `-zm`, Watcom places a
  tail-called function's entry directly after its caller but leaves the
  body at its source position, and merges empty functions into a
  neighbour's `ret`; source order decides the layout. The serial driver
  (`dosser.cpp`), Miles' CD code (`ailcd.c`) and many clib modules are
  built this way.
- **Graphics library display state** (`_lbDisplay`, `TbDisplayStruct`,
  0x78 bytes): mouse positions and button states are `volatile`; the
  retail code reads its 16-bit fields with `movsx word ptr`, never the
  `sar` trick.
- **Bullfrog network code** (`sp`, `serialsp`, `modemsp`, `ipxsp`,
  `netint*`): `-s -oneatx -xs` without `-4`. Watcom inlines small functions
  defined later in the same module, and binding a function to another
  symbol with `#pragma aux f "sym"` stops that: give it its real C++ or
  member name instead. `__stdcall` member functions still mangle as
  ordinary `W?` names. C functions whose caller pops stack arguments:
  `#pragma aux f parm caller []`. A compiler-generated destructor, vtable
  or type signature is emitted in the module that defines the class's
  constructors; function-local statics are relocated as
  `W?name$:.0$:?Fn$n(..)..`, which needs a `config/symbols.txt` entry.
  Further idioms of this code (established on `ipxsp`, `netint`,
  `modemsp`, `serialsp`):
  - Code repeated across callers is an inlined member, not a macro:
    `IPXServiceProvider::BatchProcess` (with real function-local statics
    `static u32 t = clock();`, which give the `$Wbi00$` guard byte),
    `GetNextEmptyReceiveBuffer`, `AddToReceiveBuffer`. `AddToReceiveBuffer`,
    defined after its callers, is only inlined at any `-oe` once declared
    `inline`; the original still emits it out of line.
  - `for` loops are rotated (jump to the test at the bottom), `while`
    loops are tested at the top: pick the loop kind from the layout.
  - A local copy of a pointer (`data = p->data;`, a block-scoped cast
    pointer per `case`) gets its own register or slot; the compiler does
    not merge it back into the original.
  - The `AddPlayer` flags in `AddToReceiveBuffer` and
    `EnumerateAndAddPlayers` are set with `x = 0; if (a == b) x = 1;`;
    `a == b` as the argument gives `sete` (and, in `AddToReceiveBuffer`,
    an `ebp` frame). Other functions do use `sete`.
  - Early `return`s produce separate epilogues; a single `ret` variable
    with nested `if`s produces jumps to one epilogue. `if (!x) A else B`
    and `if (x) B else A` order the blocks differently.
  - `netint` needs `-oe=400` for `LbNetwork_Exchange` to inline
    `LbNetwork_StartExchange`/`LbNetwork_CompleteExchange` and those to
    inline their multi-player halves; `HostDataCollection`/`Broadcast`
    must then be kept out of line with `#pragma aux`.
- **Bullfrog sound code** (`bf1/opt`, `bf2` sound modules): `SampleInfo`'s
  volume and fade fields are `volatile`, since the fade timer callback
  writes them; on a volatile, write `++x`/`--x` (`x++` leaves a dead load).
  `-oneatx` includes `-oa`, so an apparent local copy of a global may be
  the global cached in a register (a dead reload before each `ret` gives it
  away). Which byte register a store picks can depend on the stored value
  (`g = 0` versus `g = field`). `init_sou.cpp` is built with `-4`.
- **Miles idioms**: an I/O configuration loop's `offset` is the compiler's
  strength-reduced `i * sizeof(IO_PARMS)` (write the multiply) and its
  `IO == NULL` test is really `i == 0`; `S16` fields compare as
  `(S32)x < 0`, or the compare narrows to a word; a dead `cmp mem,0` after
  a call is a no-op `if (p == NULL) p = NULL;`.
- **Optimised library code (`-ox`, `-oneatx`)**: follow the Open Watcom
  runtime source's statement structure. Declaration order changes register
  choice but not spill slots. `shl i,2; add base` instead of a scaled
  address is a repeated array expression the compiler shared
  (`if (a[i] && f) g(a[i])`); `mov al,1`/`inc eax` stores after `x = 0` are
  `++x`; an early `return` versus `if`/`else` changes which epilogue is
  shared; `total += *sz; sz++` and `total += *sz++` load in different
  orders; cleanup code written inline at the first failure, with later
  failures jumping to it, reproduces cross-jumped tails. Bullfrog's
  Smacker wrapper is built with `-5`, since `-4` inserts loop alignment
  padding the original does not have.
- **Bullfrog library idioms (L9)**: Bullfrog's `ASSERT` is
  `do { if (!x) { if (fn_000d0848(..)) int3(); } } while (0);`, and the
  loop makes `-d2 -otx` load the tested operands into registers. A nested
  `if (A || B) { if (g) ... }` loads `g` into a register where
  `if ((A || B) && g)` compares it in memory. In the sound modules the
  0/1 byte registers rotate across the whole function, so writing an
  unrelated test as `x == 0` instead of `!x` shifts them. Network and
  serial code calls its small member functions (`ClearDtr`, `FreeDosMem`,
  the DOS memory helpers) and lets the compiler inline them: hand-written
  copies of their bodies compile differently. `dosser` uses the real
  `_disable`/`_enable` intrinsics (10.6's `_enable` emits `sti; cld`). A
  helper inlined into an earlier function and also kept out of line is a
  plain, non-`inline` definition at its source position.
- **Inlining (`-oe=N`)** counts expression-tree nodes in the front end,
  and explicit casts count: Smacker's `PutWidths` is 20 nodes plain and 25
  with its casts, which keeps it out of line at `-oe=25` as in the
  original. Other runtime idioms (R8): duplicate a cleanup tail instead of
  sharing it through `goto` to get a jump into another branch's call
  sequence (`__brk`); separate `x += c; x &= ~m;` statements give byte-wide
  masks where one expression gives `and reg,imm32`; write array loops with
  an index (`a[i]`) and let the compiler strength-reduce them into
  pointer loops; `x = C; if (flags & K) x = C;` is the source of a dead
  `test` plus a preloaded constant.
- **Bullfrog library idioms (L10)**: the compiler emits a deleting
  destructor (`test cl,4` for `delete[]`, `test cl,2` for `delete`) only
  for a class that is not abstract, so every virtual a class overrides
  must be declared. `ServiceProvider` code calls its inline message
  decoders rather than repeating them. A `TbMessageType` local compared
  with enumerators gives `cmp byte`; a `u8` gives an int compare. netint
  `Create`/`Join` put the success path first and the failure print in the
  `else`, with one final `return`; `delete p;` needs no null test. In
  loops, write offsets like `buf + (block << 9)` inside the loop body and
  let the optimiser strength-reduce them. `serialsp.cpp` is built with
  `-oe=40`: its small `Parser` accessors inline, `SendAck` does not.
