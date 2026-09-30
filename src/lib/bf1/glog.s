.intel_syntax noprefix

// glog.cpp (LbLog*): only its string literals are in the EXE. Nothing calls
// the module's functions, and the linker dropped their segments but kept
// the shared CONST segment.
.section CONST,"a",@progbits
glog_strings:
    .asciz "w"
    .asciz "a"
    .asciz "\n"
    .asciz "CREATED"
    .asciz "APPENDED"
    .asciz "LOG %s"
    .asciz "  @ %02d:%02d:%02d"
    .asciz " "
    .asciz "  @ "
    .asciz " %s%d-%02d-%d"
    .asciz "\n\n"
    .asciz "%02d-%02d-%d "
    .asciz "%02d:%02d:%02d "
