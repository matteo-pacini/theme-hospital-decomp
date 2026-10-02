.intel_syntax noprefix

// __init_387_emulator: start the 387 emulator (jumps to its initialisation).
// FUNCTION: 0x000dc812

.section _TEXT,"ax",@progbits,unique,903186
.globl "__init_387_emulator"
.type "__init_387_emulator",@function
"__init_387_emulator":
    jmp "fn_000ef0a8_"
.size "__init_387_emulator",.-"__init_387_emulator"

// __fini_387_emulator: shut the 387 emulator down (jumps into its termination code).
// FUNCTION: 0x000dc817

.section _TEXT,"ax",@progbits,unique,903191
.globl "__fini_387_emulator"
.type "__fini_387_emulator",@function
"__fini_387_emulator":
    jmp "fn_000ef20d_"+0x20
.size "__fini_387_emulator",.-"__fini_387_emulator"
