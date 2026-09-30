.intel_syntax noprefix

// fltused: the XI entry that installs the floating-point printf and scanf
// support (__setEFGfmt) at priority 0x20. The module also defines the
// absolute symbol _fltused_, which modules that use floating point refer to
// so that the linker pulls it in; no unit here refers to it, so it is left
// out.

.section XI,"aw",@progbits
    .byte 0x00, 0x20
    .long "__setEFGfmt_"
