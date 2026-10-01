/*
 * Pre-included into every source in compile_commands.json, for editors only
 * (clangd, clang-tidy): it maps the Watcom C/C++ dialect the decompilation
 * is written in onto what clang accepts. The real build uses Watcom and
 * never sees this file.
 */
#ifndef CLANGD_COMPAT_H
#define CLANGD_COMPAT_H

/* Watcom lays out every structure packed (-zp1). */
#pragma pack(1)

/* Memory models and calling-convention keywords clang does not know. */
#define __far
#define __near
#define __huge
#define __interrupt
#define __loadds
#define __saveregs
#define __export
#define __pascal
#define __fortran
#define __syscall
#define __based(segment)
#define __segment unsigned short

#endif
