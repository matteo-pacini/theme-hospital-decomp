.intel_syntax noprefix

// The dispatch table's entries were each assembled after the routine they point at:
// TABLE RECORDS: ssa_dispatch_table

// Data, not a function: the mixer's dispatch table. Entries 0-127 are the DMA
// copy routines DC_n and entries 128-255 the merge routines M_n, indexed by
// format bits (see AILSSA_DMA_copy and AILSSA_merge); 0 where no routine
// exists.
// FUNCTION: 0x000f0da8

.section _TEXT,"ax",@progbits,unique,986536
.globl "ssa_dispatch_table"
.type "ssa_dispatch_table",@function
"ssa_dispatch_table":
    .long "_DC_0"
    .long "_DC_1"
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_16"
    .long "_DC_17"
    .long "_DC_18"
    .long "_DC_19"
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_24"
    .long "_DC_25"
    .long "_DC_26"
    .long "_DC_27"
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_32"
    .long "_DC_33"
    .long 0
    .long 0
    .long "_DC_36"
    .long "_DC_37"
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_48"
    .long "_DC_49"
    .long "_DC_50"
    .long "_DC_51"
    .long "_DC_52"
    .long "_DC_53"
    .long "_DC_54"
    .long "_DC_55"
    .long "_DC_56"
    .long "_DC_57"
    .long "_DC_58"
    .long "_DC_59"
    .long "_DC_60"
    .long "_DC_61"
    .long "_DC_62"
    .long "_DC_63"
    .long "_DC_64"
    .long "_DC_65"
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_80"
    .long "_DC_81"
    .long "_DC_82"
    .long "_DC_83"
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_88"
    .long "_DC_89"
    .long "_DC_90"
    .long "_DC_91"
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_96"
    .long "_DC_97"
    .long 0
    .long 0
    .long "_DC_100"
    .long "_DC_101"
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_DC_112"
    .long "_DC_113"
    .long "_DC_114"
    .long "_DC_115"
    .long "_DC_116"
    .long "_DC_117"
    .long "_DC_118"
    .long "_DC_119"
    .long "_DC_120"
    .long "_DC_121"
    .long "_DC_122"
    .long "_DC_123"
    .long "_DC_124"
    .long "_DC_125"
    .long "_DC_126"
    .long "_DC_127"
    .long "_M_0"
    .long "_M_1"
    .long 0
    .long "_M_3"
    .long 0
    .long 0
    .long 0
    .long "_M_7"
    .long "_M_8"
    .long "_M_9"
    .long 0
    .long "_M_11"
    .long 0
    .long 0
    .long 0
    .long "_M_15"
    .long "_M_16"
    .long "_M_17"
    .long 0
    .long "_M_19"
    .long 0
    .long 0
    .long 0
    .long "_M_23"
    .long "_M_24"
    .long "_M_25"
    .long 0
    .long "_M_27"
    .long 0
    .long 0
    .long 0
    .long "_M_31"
    .long "_M_32"
    .long "_M_33"
    .long 0
    .long "_M_35"
    .long 0
    .long 0
    .long 0
    .long "_M_39"
    .long "_M_40"
    .long "_M_41"
    .long 0
    .long "_M_43"
    .long 0
    .long 0
    .long 0
    .long "_M_47"
    .long "_M_48"
    .long "_M_49"
    .long 0
    .long "_M_51"
    .long 0
    .long 0
    .long 0
    .long "_M_55"
    .long "_M_56"
    .long "_M_57"
    .long 0
    .long "_M_59"
    .long 0
    .long 0
    .long 0
    .long "_M_63"
    .long "_M_64"
    .long "_M_65"
    .long "_M_66"
    .long "_M_67"
    .long 0
    .long 0
    .long "_M_70"
    .long "_M_71"
    .long "_M_72"
    .long "_M_73"
    .long "_M_74"
    .long "_M_75"
    .long 0
    .long 0
    .long "_M_78"
    .long "_M_79"
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_M_88"
    .long "_M_89"
    .long "_M_90"
    .long "_M_91"
    .long 0
    .long 0
    .long "_M_94"
    .long "_M_95"
    .long "_M_96"
    .long "_M_97"
    .long "_M_98"
    .long "_M_99"
    .long 0
    .long 0
    .long "_M_102"
    .long "_M_103"
    .long "_M_104"
    .long "_M_105"
    .long "_M_106"
    .long "_M_107"
    .long 0
    .long 0
    .long "_M_110"
    .long "_M_111"
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long "_M_120"
    .long "_M_121"
    .long "_M_122"
    .long "_M_123"
    .long 0
    .long 0
    .long "_M_126"
    .long "_M_127"
.size "ssa_dispatch_table",.-"ssa_dispatch_table"

// DMA copy routine 0: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit mono output at EDI, unclipped.
// FUNCTION: 0x000f11a8

.section _TEXT,"ax",@progbits,unique,987560
.globl "_DC_0"
.type "_DC_0",@function
"_DC_0":
    mov eax, dword ptr [esi]
    add esi, 4
    xor eax, 0x8000
    mov byte ptr [edi], ah
    inc edi
    dec ecx
    .byte 0x75; .byte 0xf0  # jne 0xf11a8
    ret
.size "_DC_0",.-"_DC_0"

// DMA copy routine 1: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit mono output at EDI, unclipped.
// FUNCTION: 0x000f11b9

.section _TEXT,"ax",@progbits,unique,987577
.globl "_DC_1"
.type "_DC_1",@function
"_DC_1":
    mov eax, dword ptr [esi]
    add esi, 4
    mov byte ptr [edi], ah
    inc edi
    dec ecx
    .byte 0x75; .byte 0xf5  # jne 0xf11b9
    ret
.size "_DC_1",.-"_DC_1"

// DMA copy routine 16: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, unclipped.
// FUNCTION: 0x000f11c5

.section _TEXT,"ax",@progbits,unique,987589
.globl "_DC_16"
.type "_DC_16",@function
"_DC_16":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    {load} mov dl, ah
    mov word ptr [edi], dx
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xe2  # jne 0xf11c5
    ret
.size "_DC_16",.-"_DC_16"

// DMA copy routine 17: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, unclipped.
// FUNCTION: 0x000f11e4

.section _TEXT,"ax",@progbits,unique,987620
.globl "_DC_17"
.type "_DC_17",@function
"_DC_17":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    {load} mov dl, ah
    mov word ptr [edi], dx
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xed  # jne 0xf11e4
    ret
.size "_DC_17",.-"_DC_17"

// DMA copy routine 18: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, right channel first, unclipped.
// FUNCTION: 0x000f11f8

.section _TEXT,"ax",@progbits,unique,987640
.globl "_DC_18"
.type "_DC_18",@function
"_DC_18":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    {load} mov al, dh
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xe2  # jne 0xf11f8
    ret
.size "_DC_18",.-"_DC_18"

// DMA copy routine 19: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, right channel first, unclipped.
// FUNCTION: 0x000f1217

.section _TEXT,"ax",@progbits,unique,987671
.globl "_DC_19"
.type "_DC_19",@function
"_DC_19":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    {load} mov al, dh
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xed  # jne 0xf1217
    ret
.size "_DC_19",.-"_DC_19"

// DMA copy routine 24: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, left and right to separate buffers (EDI
// and EBX), unclipped.
// FUNCTION: 0x000f122b

.section _TEXT,"ax",@progbits,unique,987691
.globl "_DC_24"
.type "_DC_24",@function
"_DC_24":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    mov byte ptr [edi], ah
    mov byte ptr [ebx], dh
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xe4  # jne 0xf122b
    ret
.size "_DC_24",.-"_DC_24"

// DMA copy routine 25: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, left and right to separate buffers (EDI
// and EBX), unclipped.
// FUNCTION: 0x000f1248

.section _TEXT,"ax",@progbits,unique,987720
.globl "_DC_25"
.type "_DC_25",@function
"_DC_25":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    mov byte ptr [edi], ah
    mov byte ptr [ebx], dh
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xef  # jne 0xf1248
    ret
.size "_DC_25",.-"_DC_25"

// DMA copy routine 26: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f125a

.section _TEXT,"ax",@progbits,unique,987738
.globl "_DC_26"
.type "_DC_26",@function
"_DC_26":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    mov byte ptr [edi], dh
    mov byte ptr [ebx], ah
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xe4  # jne 0xf125a
    ret
.size "_DC_26",.-"_DC_26"

// DMA copy routine 27: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f1277

.section _TEXT,"ax",@progbits,unique,987767
.globl "_DC_27"
.type "_DC_27",@function
"_DC_27":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    mov byte ptr [edi], dh
    mov byte ptr [ebx], ah
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xef  # jne 0xf1277
    ret
.size "_DC_27",.-"_DC_27"

// DMA copy routine 32: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit mono output at EDI, unclipped.
// FUNCTION: 0x000f1289

.section _TEXT,"ax",@progbits,unique,987785
.globl "_DC_32"
.type "_DC_32",@function
"_DC_32":
    mov eax, dword ptr [esi]
    add esi, 4
    xor eax, 0x8000
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xed  # jne 0xf1289
    ret
.size "_DC_32",.-"_DC_32"

// DMA copy routine 33: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit mono output at EDI, unclipped.
// FUNCTION: 0x000f129d

.section _TEXT,"ax",@progbits,unique,987805
.globl "_DC_33"
.type "_DC_33",@function
"_DC_33":
    mov eax, dword ptr [esi]
    add esi, 4
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xf2  # jne 0xf129d
    ret
.size "_DC_33",.-"_DC_33"

// DMA copy routine 36: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit mono output at EDI, high byte first, unclipped.
// FUNCTION: 0x000f12ac

.section _TEXT,"ax",@progbits,unique,987820
.globl "_DC_36"
.type "_DC_36",@function
"_DC_36":
    mov eax, dword ptr [esi]
    add esi, 4
    xor eax, 0x8000
    xchg ah, al
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xeb  # jne 0xf12ac
    ret
.size "_DC_36",.-"_DC_36"

// DMA copy routine 37: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit mono output at EDI, high byte first, unclipped.
// FUNCTION: 0x000f12c2

.section _TEXT,"ax",@progbits,unique,987842
.globl "_DC_37"
.type "_DC_37",@function
"_DC_37":
    mov eax, dword ptr [esi]
    add esi, 4
    xchg ah, al
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xf0  # jne 0xf12c2
    ret
.size "_DC_37",.-"_DC_37"

// DMA copy routine 48: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, unclipped.
// FUNCTION: 0x000f12d3

.section _TEXT,"ax",@progbits,unique,987859
.globl "_DC_48"
.type "_DC_48",@function
"_DC_48":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xe0  # jne 0xf12d3
    ret
.size "_DC_48",.-"_DC_48"

// DMA copy routine 49: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, unclipped.
// FUNCTION: 0x000f12f4

.section _TEXT,"ax",@progbits,unique,987892
.globl "_DC_49"
.type "_DC_49",@function
"_DC_49":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xeb  # jne 0xf12f4
    ret
.size "_DC_49",.-"_DC_49"

// DMA copy routine 50: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, right channel first, unclipped.
// FUNCTION: 0x000f130a

.section _TEXT,"ax",@progbits,unique,987914
.globl "_DC_50"
.type "_DC_50",@function
"_DC_50":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xe0  # jne 0xf130a
    ret
.size "_DC_50",.-"_DC_50"

// DMA copy routine 51: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, right channel first, unclipped.
// FUNCTION: 0x000f132b

.section _TEXT,"ax",@progbits,unique,987947
.globl "_DC_51"
.type "_DC_51",@function
"_DC_51":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xeb  # jne 0xf132b
    ret
.size "_DC_51",.-"_DC_51"

// DMA copy routine 52: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, high byte first, unclipped.
// FUNCTION: 0x000f1341

.section _TEXT,"ax",@progbits,unique,987969
.globl "_DC_52"
.type "_DC_52",@function
"_DC_52":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xdc  # jne 0xf1341
    ret
.size "_DC_52",.-"_DC_52"

// DMA copy routine 53: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, high byte first, unclipped.
// FUNCTION: 0x000f1366

.section _TEXT,"ax",@progbits,unique,988006
.globl "_DC_53"
.type "_DC_53",@function
"_DC_53":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xe7  # jne 0xf1366
    ret
.size "_DC_53",.-"_DC_53"

// DMA copy routine 54: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, right channel first, high byte first,
// unclipped.
// FUNCTION: 0x000f1380

.section _TEXT,"ax",@progbits,unique,988032
.globl "_DC_54"
.type "_DC_54",@function
"_DC_54":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xdc  # jne 0xf1380
    ret
.size "_DC_54",.-"_DC_54"

// DMA copy routine 55: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, right channel first, high byte first,
// unclipped.
// FUNCTION: 0x000f13a5

.section _TEXT,"ax",@progbits,unique,988069
.globl "_DC_55"
.type "_DC_55",@function
"_DC_55":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xe7  # jne 0xf13a5
    ret
.size "_DC_55",.-"_DC_55"

// DMA copy routine 56: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, left and right to separate buffers (EDI
// and EBX), unclipped.
// FUNCTION: 0x000f13bf

.section _TEXT,"ax",@progbits,unique,988095
.globl "_DC_56"
.type "_DC_56",@function
"_DC_56":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xde  # jne 0xf13bf
    ret
.size "_DC_56",.-"_DC_56"

// DMA copy routine 57: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, left and right to separate buffers (EDI
// and EBX), unclipped.
// FUNCTION: 0x000f13e2

.section _TEXT,"ax",@progbits,unique,988130
.globl "_DC_57"
.type "_DC_57",@function
"_DC_57":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xe9  # jne 0xf13e2
    ret
.size "_DC_57",.-"_DC_57"

// DMA copy routine 58: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f13fa

.section _TEXT,"ax",@progbits,unique,988154
.globl "_DC_58"
.type "_DC_58",@function
"_DC_58":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xde  # jne 0xf13fa
    ret
.size "_DC_58",.-"_DC_58"

// DMA copy routine 59: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f141d

.section _TEXT,"ax",@progbits,unique,988189
.globl "_DC_59"
.type "_DC_59",@function
"_DC_59":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xe9  # jne 0xf141d
    ret
.size "_DC_59",.-"_DC_59"

// DMA copy routine 60: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, high byte first, left and right to
// separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f1435

.section _TEXT,"ax",@progbits,unique,988213
.globl "_DC_60"
.type "_DC_60",@function
"_DC_60":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xda  # jne 0xf1435
    ret
.size "_DC_60",.-"_DC_60"

// DMA copy routine 61: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, high byte first, left and right to
// separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f145c

.section _TEXT,"ax",@progbits,unique,988252
.globl "_DC_61"
.type "_DC_61",@function
"_DC_61":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xe5  # jne 0xf145c
    ret
.size "_DC_61",.-"_DC_61"

// DMA copy routine 62: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit stereo output at EDI, right channel first, high byte first,
// left and right to separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f1478

.section _TEXT,"ax",@progbits,unique,988280
.globl "_DC_62"
.type "_DC_62",@function
"_DC_62":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xda  # jne 0xf1478
    ret
.size "_DC_62",.-"_DC_62"

// DMA copy routine 63: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit stereo output at EDI, right channel first, high byte first,
// left and right to separate buffers (EDI and EBX), unclipped.
// FUNCTION: 0x000f149f

.section _TEXT,"ax",@progbits,unique,988319
.globl "_DC_63"
.type "_DC_63",@function
"_DC_63":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xe5  # jne 0xf149f
    ret
.size "_DC_63",.-"_DC_63"

// DMA copy routine 64: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit mono output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f14bb

.section _TEXT,"ax",@progbits,unique,988347
.globl "_DC_64"
.type "_DC_64",@function
"_DC_64":
    mov eax, dword ptr [esi]
    add esi, 4
    cmp eax, 0x7fff
    jg .L000f14da
    cmp eax, 0xffff8000
    jl .L000f14e1
.L000f14ce:
    xor eax, 0x8000
    mov byte ptr [edi], ah
    inc edi
    dec ecx
    .byte 0x75; .byte 0xe2  # jne 0xf14bb
    ret
.L000f14da:
    mov eax, 0x7fff
    jmp .L000f14ce
.L000f14e1:
    mov eax, 0xffff8000
    jmp .L000f14ce
.size "_DC_64",.-"_DC_64"

// DMA copy routine 65: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit mono output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f14e8

.section _TEXT,"ax",@progbits,unique,988392
.globl "_DC_65"
.type "_DC_65",@function
"_DC_65":
    mov eax, dword ptr [esi]
    add esi, 4
    cmp eax, 0x7fff
    jg .L000f1502
    cmp eax, 0xffff8000
    jl .L000f1509
.L000f14fb:
    mov byte ptr [edi], ah
    inc edi
    dec ecx
    .byte 0x75; .byte 0xe7  # jne 0xf14e8
    ret
.L000f1502:
    mov eax, 0x7fff
    jmp .L000f14fb
.L000f1509:
    mov eax, 0xffff8000
    jmp .L000f14fb
.size "_DC_65",.-"_DC_65"

// DMA copy routine 80: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f1510

.section _TEXT,"ax",@progbits,unique,988432
.globl "_DC_80"
.type "_DC_80",@function
"_DC_80":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f154d
    cmp eax, 0xffff8000
    jl .L000f1554
.L000f1526:
    cmp edx, 0x7fff
    jg .L000f155b
    cmp edx, 0xffff8000
    jl .L000f1562
.L000f1536:
    xor eax, 0x8000
    xor edx, 0x8000
    {load} mov dl, ah
    mov word ptr [edi], dx
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xc4  # jne 0xf1510
    ret
.L000f154d:
    mov eax, 0x7fff
    jmp .L000f1526
.L000f1554:
    mov eax, 0xffff8000
    jmp .L000f1526
.L000f155b:
    mov edx, 0x7fff
    jmp .L000f1536
.L000f1562:
    mov edx, 0xffff8000
    jmp .L000f1536
.size "_DC_80",.-"_DC_80"

// DMA copy routine 81: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f1569

.section _TEXT,"ax",@progbits,unique,988521
.globl "_DC_81"
.type "_DC_81",@function
"_DC_81":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f159b
    cmp eax, 0xffff8000
    jl .L000f15a2
.L000f157f:
    cmp edx, 0x7fff
    jg .L000f15a9
    cmp edx, 0xffff8000
    jl .L000f15b0
.L000f158f:
    {load} mov dl, ah
    mov word ptr [edi], dx
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xcf  # jne 0xf1569
    ret
.L000f159b:
    mov eax, 0x7fff
    jmp .L000f157f
.L000f15a2:
    mov eax, 0xffff8000
    jmp .L000f157f
.L000f15a9:
    mov edx, 0x7fff
    jmp .L000f158f
.L000f15b0:
    mov edx, 0xffff8000
    jmp .L000f158f
.size "_DC_81",.-"_DC_81"

// DMA copy routine 82: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, right channel first, clipped to 16 bits.
// FUNCTION: 0x000f15b7

.section _TEXT,"ax",@progbits,unique,988599
.globl "_DC_82"
.type "_DC_82",@function
"_DC_82":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f15f4
    cmp eax, 0xffff8000
    jl .L000f15fb
.L000f15cd:
    cmp edx, 0x7fff
    jg .L000f1602
    cmp edx, 0xffff8000
    jl .L000f1609
.L000f15dd:
    xor eax, 0x8000
    xor edx, 0x8000
    {load} mov al, dh
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xc4  # jne 0xf15b7
    ret
.L000f15f4:
    mov eax, 0x7fff
    jmp .L000f15cd
.L000f15fb:
    mov eax, 0xffff8000
    jmp .L000f15cd
.L000f1602:
    mov edx, 0x7fff
    jmp .L000f15dd
.L000f1609:
    mov edx, 0xffff8000
    jmp .L000f15dd
.size "_DC_82",.-"_DC_82"

// DMA copy routine 83: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, right channel first, clipped to 16 bits.
// FUNCTION: 0x000f1610

.section _TEXT,"ax",@progbits,unique,988688
.globl "_DC_83"
.type "_DC_83",@function
"_DC_83":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1642
    cmp eax, 0xffff8000
    jl .L000f1649
.L000f1626:
    cmp edx, 0x7fff
    jg .L000f1650
    cmp edx, 0xffff8000
    jl .L000f1657
.L000f1636:
    {load} mov al, dh
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xcf  # jne 0xf1610
    ret
.L000f1642:
    mov eax, 0x7fff
    jmp .L000f1626
.L000f1649:
    mov eax, 0xffff8000
    jmp .L000f1626
.L000f1650:
    mov edx, 0x7fff
    jmp .L000f1636
.L000f1657:
    mov edx, 0xffff8000
    jmp .L000f1636
.size "_DC_83",.-"_DC_83"

// DMA copy routine 88: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, left and right to separate buffers (EDI
// and EBX), clipped to 16 bits.
// FUNCTION: 0x000f165e

.section _TEXT,"ax",@progbits,unique,988766
.globl "_DC_88"
.type "_DC_88",@function
"_DC_88":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1699
    cmp eax, 0xffff8000
    jl .L000f16a0
.L000f1674:
    cmp edx, 0x7fff
    jg .L000f16a7
    cmp edx, 0xffff8000
    jl .L000f16ae
.L000f1684:
    xor eax, 0x8000
    xor edx, 0x8000
    mov byte ptr [edi], ah
    mov byte ptr [ebx], dh
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xc6  # jne 0xf165e
    ret
.L000f1699:
    mov eax, 0x7fff
    jmp .L000f1674
.L000f16a0:
    mov eax, 0xffff8000
    jmp .L000f1674
.L000f16a7:
    mov edx, 0x7fff
    jmp .L000f1684
.L000f16ae:
    mov edx, 0xffff8000
    jmp .L000f1684
.size "_DC_88",.-"_DC_88"

// DMA copy routine 89: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, left and right to separate buffers (EDI
// and EBX), clipped to 16 bits.
// FUNCTION: 0x000f16b5

.section _TEXT,"ax",@progbits,unique,988853
.globl "_DC_89"
.type "_DC_89",@function
"_DC_89":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f16e5
    cmp eax, 0xffff8000
    jl .L000f16ec
.L000f16cb:
    cmp edx, 0x7fff
    jg .L000f16f3
    cmp edx, 0xffff8000
    jl .L000f16fa
.L000f16db:
    mov byte ptr [edi], ah
    mov byte ptr [ebx], dh
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xd1  # jne 0xf16b5
    ret
.L000f16e5:
    mov eax, 0x7fff
    jmp .L000f16cb
.L000f16ec:
    mov eax, 0xffff8000
    jmp .L000f16cb
.L000f16f3:
    mov edx, 0x7fff
    jmp .L000f16db
.L000f16fa:
    mov edx, 0xffff8000
    jmp .L000f16db
.size "_DC_89",.-"_DC_89"

// DMA copy routine 90: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 8-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1701

.section _TEXT,"ax",@progbits,unique,988929
.globl "_DC_90"
.type "_DC_90",@function
"_DC_90":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f173c
    cmp eax, 0xffff8000
    jl .L000f1743
.L000f1717:
    cmp edx, 0x7fff
    jg .L000f174a
    cmp edx, 0xffff8000
    jl .L000f1751
.L000f1727:
    xor eax, 0x8000
    xor edx, 0x8000
    mov byte ptr [edi], dh
    mov byte ptr [ebx], ah
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xc6  # jne 0xf1701
    ret
.L000f173c:
    mov eax, 0x7fff
    jmp .L000f1717
.L000f1743:
    mov eax, 0xffff8000
    jmp .L000f1717
.L000f174a:
    mov edx, 0x7fff
    jmp .L000f1727
.L000f1751:
    mov edx, 0xffff8000
    jmp .L000f1727
.size "_DC_90",.-"_DC_90"

// DMA copy routine 91: converts ECX frames of the 32-bit build buffer at ESI to
// signed 8-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1758

.section _TEXT,"ax",@progbits,unique,989016
.globl "_DC_91"
.type "_DC_91",@function
"_DC_91":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1788
    cmp eax, 0xffff8000
    jl .L000f178f
.L000f176e:
    cmp edx, 0x7fff
    jg .L000f1796
    cmp edx, 0xffff8000
    jl .L000f179d
.L000f177e:
    mov byte ptr [edi], dh
    mov byte ptr [ebx], ah
    inc edi
    inc ebx
    dec ecx
    .byte 0x75; .byte 0xd1  # jne 0xf1758
    ret
.L000f1788:
    mov eax, 0x7fff
    jmp .L000f176e
.L000f178f:
    mov eax, 0xffff8000
    jmp .L000f176e
.L000f1796:
    mov edx, 0x7fff
    jmp .L000f177e
.L000f179d:
    mov edx, 0xffff8000
    jmp .L000f177e
.size "_DC_91",.-"_DC_91"

// DMA copy routine 96: converts ECX frames of the 32-bit build buffer at ESI to
// unsigned 16-bit mono output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f17a4

.section _TEXT,"ax",@progbits,unique,989092
.globl "_DC_96"
.type "_DC_96",@function
"_DC_96":
    mov eax, dword ptr [esi]
    add esi, 4
    cmp eax, 0x7fff
    jg .L000f17c6
    cmp eax, 0xffff8000
    jl .L000f17cd
.L000f17b7:
    xor eax, 0x8000
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xdf  # jne 0xf17a4
    ret
.L000f17c6:
    mov eax, 0x7fff
    jmp .L000f17b7
.L000f17cd:
    mov eax, 0xffff8000
    jmp .L000f17b7
.size "_DC_96",.-"_DC_96"

// DMA copy routine 97: converts ECX frames of the 32-bit build buffer at ESI to
// signed 16-bit mono output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f17d4

.section _TEXT,"ax",@progbits,unique,989140
.globl "_DC_97"
.type "_DC_97",@function
"_DC_97":
    mov eax, dword ptr [esi]
    add esi, 4
    cmp eax, 0x7fff
    jg .L000f17f1
    cmp eax, 0xffff8000
    jl .L000f17f8
.L000f17e7:
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xe4  # jne 0xf17d4
    ret
.L000f17f1:
    mov eax, 0x7fff
    jmp .L000f17e7
.L000f17f8:
    mov eax, 0xffff8000
    jmp .L000f17e7
.size "_DC_97",.-"_DC_97"

// DMA copy routine 100: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit mono output at EDI, high byte first, clipped to 16 bits.
// FUNCTION: 0x000f17ff

.section _TEXT,"ax",@progbits,unique,989183
.globl "_DC_100"
.type "_DC_100",@function
"_DC_100":
    mov eax, dword ptr [esi]
    add esi, 4
    cmp eax, 0x7fff
    jg .L000f1823
    cmp eax, 0xffff8000
    jl .L000f182a
.L000f1812:
    xor eax, 0x8000
    xchg ah, al
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xdd  # jne 0xf17ff
    ret
.L000f1823:
    mov eax, 0x7fff
    jmp .L000f1812
.L000f182a:
    mov eax, 0xffff8000
    jmp .L000f1812
.size "_DC_100",.-"_DC_100"

// DMA copy routine 101: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit mono output at EDI, high byte first, clipped to 16 bits.
// FUNCTION: 0x000f1831

.section _TEXT,"ax",@progbits,unique,989233
.globl "_DC_101"
.type "_DC_101",@function
"_DC_101":
    mov eax, dword ptr [esi]
    add esi, 4
    cmp eax, 0x7fff
    jg .L000f1850
    cmp eax, 0xffff8000
    jl .L000f1857
.L000f1844:
    xchg ah, al
    mov word ptr [edi], ax
    add edi, 2
    dec ecx
    .byte 0x75; .byte 0xe2  # jne 0xf1831
    ret
.L000f1850:
    mov eax, 0x7fff
    jmp .L000f1844
.L000f1857:
    mov eax, 0xffff8000
    jmp .L000f1844
.size "_DC_101",.-"_DC_101"

// DMA copy routine 112: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f185e

.section _TEXT,"ax",@progbits,unique,989278
.globl "_DC_112"
.type "_DC_112",@function
"_DC_112":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f189d
    cmp eax, 0xffff8000
    jl .L000f18a4
.L000f1874:
    cmp edx, 0x7fff
    jg .L000f18ab
    cmp edx, 0xffff8000
    jl .L000f18b2
.L000f1884:
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xc2  # jne 0xf185e
    ret
.L000f189d:
    mov eax, 0x7fff
    jmp .L000f1874
.L000f18a4:
    mov eax, 0xffff8000
    jmp .L000f1874
.L000f18ab:
    mov edx, 0x7fff
    jmp .L000f1884
.L000f18b2:
    mov edx, 0xffff8000
    jmp .L000f1884
.size "_DC_112",.-"_DC_112"

// DMA copy routine 113: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, clipped to 16 bits.
// FUNCTION: 0x000f18b9

.section _TEXT,"ax",@progbits,unique,989369
.globl "_DC_113"
.type "_DC_113",@function
"_DC_113":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f18ed
    cmp eax, 0xffff8000
    jl .L000f18f4
.L000f18cf:
    cmp edx, 0x7fff
    jg .L000f18fb
    cmp edx, 0xffff8000
    jl .L000f1902
.L000f18df:
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xcd  # jne 0xf18b9
    ret
.L000f18ed:
    mov eax, 0x7fff
    jmp .L000f18cf
.L000f18f4:
    mov eax, 0xffff8000
    jmp .L000f18cf
.L000f18fb:
    mov edx, 0x7fff
    jmp .L000f18df
.L000f1902:
    mov edx, 0xffff8000
    jmp .L000f18df
.size "_DC_113",.-"_DC_113"

// DMA copy routine 114: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, right channel first, clipped to 16
// bits.
// FUNCTION: 0x000f1909

.section _TEXT,"ax",@progbits,unique,989449
.globl "_DC_114"
.type "_DC_114",@function
"_DC_114":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1948
    cmp eax, 0xffff8000
    jl .L000f194f
.L000f191f:
    cmp edx, 0x7fff
    jg .L000f1956
    cmp edx, 0xffff8000
    jl .L000f195d
.L000f192f:
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xc2  # jne 0xf1909
    ret
.L000f1948:
    mov eax, 0x7fff
    jmp .L000f191f
.L000f194f:
    mov eax, 0xffff8000
    jmp .L000f191f
.L000f1956:
    mov edx, 0x7fff
    jmp .L000f192f
.L000f195d:
    mov edx, 0xffff8000
    jmp .L000f192f
.size "_DC_114",.-"_DC_114"

// DMA copy routine 115: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, right channel first, clipped to 16
// bits.
// FUNCTION: 0x000f1964

.section _TEXT,"ax",@progbits,unique,989540
.globl "_DC_115"
.type "_DC_115",@function
"_DC_115":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1998
    cmp eax, 0xffff8000
    jl .L000f199f
.L000f197a:
    cmp edx, 0x7fff
    jg .L000f19a6
    cmp edx, 0xffff8000
    jl .L000f19ad
.L000f198a:
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xcd  # jne 0xf1964
    ret
.L000f1998:
    mov eax, 0x7fff
    jmp .L000f197a
.L000f199f:
    mov eax, 0xffff8000
    jmp .L000f197a
.L000f19a6:
    mov edx, 0x7fff
    jmp .L000f198a
.L000f19ad:
    mov edx, 0xffff8000
    jmp .L000f198a
.size "_DC_115",.-"_DC_115"

// DMA copy routine 116: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, high byte first, clipped to 16 bits.
// FUNCTION: 0x000f19b4

.section _TEXT,"ax",@progbits,unique,989620
.globl "_DC_116"
.type "_DC_116",@function
"_DC_116":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f19f7
    cmp eax, 0xffff8000
    jl .L000f19fe
.L000f19ca:
    cmp edx, 0x7fff
    jg .L000f1a05
    cmp edx, 0xffff8000
    jl .L000f1a0c
.L000f19da:
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xbe  # jne 0xf19b4
    ret
.L000f19f7:
    mov eax, 0x7fff
    jmp .L000f19ca
.L000f19fe:
    mov eax, 0xffff8000
    jmp .L000f19ca
.L000f1a05:
    mov edx, 0x7fff
    jmp .L000f19da
.L000f1a0c:
    mov edx, 0xffff8000
    jmp .L000f19da
.size "_DC_116",.-"_DC_116"

// DMA copy routine 117: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, high byte first, clipped to 16 bits.
// FUNCTION: 0x000f1a13

.section _TEXT,"ax",@progbits,unique,989715
.globl "_DC_117"
.type "_DC_117",@function
"_DC_117":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1a4b
    cmp eax, 0xffff8000
    jl .L000f1a52
.L000f1a29:
    cmp edx, 0x7fff
    jg .L000f1a59
    cmp edx, 0xffff8000
    jl .L000f1a60
.L000f1a39:
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [edi + 2], dx
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xc9  # jne 0xf1a13
    ret
.L000f1a4b:
    mov eax, 0x7fff
    jmp .L000f1a29
.L000f1a52:
    mov eax, 0xffff8000
    jmp .L000f1a29
.L000f1a59:
    mov edx, 0x7fff
    jmp .L000f1a39
.L000f1a60:
    mov edx, 0xffff8000
    jmp .L000f1a39
.size "_DC_117",.-"_DC_117"

// DMA copy routine 118: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, right channel first, high byte
// first, clipped to 16 bits.
// FUNCTION: 0x000f1a67

.section _TEXT,"ax",@progbits,unique,989799
.globl "_DC_118"
.type "_DC_118",@function
"_DC_118":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1aaa
    cmp eax, 0xffff8000
    jl .L000f1ab1
.L000f1a7d:
    cmp edx, 0x7fff
    jg .L000f1ab8
    cmp edx, 0xffff8000
    jl .L000f1abf
.L000f1a8d:
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xbe  # jne 0xf1a67
    ret
.L000f1aaa:
    mov eax, 0x7fff
    jmp .L000f1a7d
.L000f1ab1:
    mov eax, 0xffff8000
    jmp .L000f1a7d
.L000f1ab8:
    mov edx, 0x7fff
    jmp .L000f1a8d
.L000f1abf:
    mov edx, 0xffff8000
    jmp .L000f1a8d
.size "_DC_118",.-"_DC_118"

// DMA copy routine 119: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, right channel first, high byte first,
// clipped to 16 bits.
// FUNCTION: 0x000f1ac6

.section _TEXT,"ax",@progbits,unique,989894
.globl "_DC_119"
.type "_DC_119",@function
"_DC_119":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1afe
    cmp eax, 0xffff8000
    jl .L000f1b05
.L000f1adc:
    cmp edx, 0x7fff
    jg .L000f1b0c
    cmp edx, 0xffff8000
    jl .L000f1b13
.L000f1aec:
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [edi + 2], ax
    add edi, 4
    dec ecx
    .byte 0x75; .byte 0xc9  # jne 0xf1ac6
    ret
.L000f1afe:
    mov eax, 0x7fff
    jmp .L000f1adc
.L000f1b05:
    mov eax, 0xffff8000
    jmp .L000f1adc
.L000f1b0c:
    mov edx, 0x7fff
    jmp .L000f1aec
.L000f1b13:
    mov edx, 0xffff8000
    jmp .L000f1aec
.size "_DC_119",.-"_DC_119"

// DMA copy routine 120: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, left and right to separate buffers
// (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1b1a

.section _TEXT,"ax",@progbits,unique,989978
.globl "_DC_120"
.type "_DC_120",@function
"_DC_120":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1b5b
    cmp eax, 0xffff8000
    jl .L000f1b62
.L000f1b30:
    cmp edx, 0x7fff
    jg .L000f1b69
    cmp edx, 0xffff8000
    jl .L000f1b70
.L000f1b40:
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xc0  # jne 0xf1b1a
    ret
.L000f1b5b:
    mov eax, 0x7fff
    jmp .L000f1b30
.L000f1b62:
    mov eax, 0xffff8000
    jmp .L000f1b30
.L000f1b69:
    mov edx, 0x7fff
    jmp .L000f1b40
.L000f1b70:
    mov edx, 0xffff8000
    jmp .L000f1b40
.size "_DC_120",.-"_DC_120"

// DMA copy routine 121: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, left and right to separate buffers
// (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1b77

.section _TEXT,"ax",@progbits,unique,990071
.globl "_DC_121"
.type "_DC_121",@function
"_DC_121":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1bad
    cmp eax, 0xffff8000
    jl .L000f1bb4
.L000f1b8d:
    cmp edx, 0x7fff
    jg .L000f1bbb
    cmp edx, 0xffff8000
    jl .L000f1bc2
.L000f1b9d:
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xcb  # jne 0xf1b77
    ret
.L000f1bad:
    mov eax, 0x7fff
    jmp .L000f1b8d
.L000f1bb4:
    mov eax, 0xffff8000
    jmp .L000f1b8d
.L000f1bbb:
    mov edx, 0x7fff
    jmp .L000f1b9d
.L000f1bc2:
    mov edx, 0xffff8000
    jmp .L000f1b9d
.size "_DC_121",.-"_DC_121"

// DMA copy routine 122: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, right channel first, left and right
// to separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1bc9

.section _TEXT,"ax",@progbits,unique,990153
.globl "_DC_122"
.type "_DC_122",@function
"_DC_122":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1c0a
    cmp eax, 0xffff8000
    jl .L000f1c11
.L000f1bdf:
    cmp edx, 0x7fff
    jg .L000f1c18
    cmp edx, 0xffff8000
    jl .L000f1c1f
.L000f1bef:
    xor eax, 0x8000
    xor edx, 0x8000
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xc0  # jne 0xf1bc9
    ret
.L000f1c0a:
    mov eax, 0x7fff
    jmp .L000f1bdf
.L000f1c11:
    mov eax, 0xffff8000
    jmp .L000f1bdf
.L000f1c18:
    mov edx, 0x7fff
    jmp .L000f1bef
.L000f1c1f:
    mov edx, 0xffff8000
    jmp .L000f1bef
.size "_DC_122",.-"_DC_122"

// DMA copy routine 123: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, right channel first, left and right to
// separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1c26

.section _TEXT,"ax",@progbits,unique,990246
.globl "_DC_123"
.type "_DC_123",@function
"_DC_123":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1c5c
    cmp eax, 0xffff8000
    jl .L000f1c63
.L000f1c3c:
    cmp edx, 0x7fff
    jg .L000f1c6a
    cmp edx, 0xffff8000
    jl .L000f1c71
.L000f1c4c:
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xcb  # jne 0xf1c26
    ret
.L000f1c5c:
    mov eax, 0x7fff
    jmp .L000f1c3c
.L000f1c63:
    mov eax, 0xffff8000
    jmp .L000f1c3c
.L000f1c6a:
    mov edx, 0x7fff
    jmp .L000f1c4c
.L000f1c71:
    mov edx, 0xffff8000
    jmp .L000f1c4c
.size "_DC_123",.-"_DC_123"

// DMA copy routine 124: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, high byte first, left and right to
// separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1c78

.section _TEXT,"ax",@progbits,unique,990328
.globl "_DC_124"
.type "_DC_124",@function
"_DC_124":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1cbd
    cmp eax, 0xffff8000
    jl .L000f1cc4
.L000f1c8e:
    cmp edx, 0x7fff
    jg .L000f1ccb
    cmp edx, 0xffff8000
    jl .L000f1cd2
.L000f1c9e:
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xbc  # jne 0xf1c78
    ret
.L000f1cbd:
    mov eax, 0x7fff
    jmp .L000f1c8e
.L000f1cc4:
    mov eax, 0xffff8000
    jmp .L000f1c8e
.L000f1ccb:
    mov edx, 0x7fff
    jmp .L000f1c9e
.L000f1cd2:
    mov edx, 0xffff8000
    jmp .L000f1c9e
.size "_DC_124",.-"_DC_124"

// DMA copy routine 125: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, high byte first, left and right to
// separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1cd9

.section _TEXT,"ax",@progbits,unique,990425
.globl "_DC_125"
.type "_DC_125",@function
"_DC_125":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1d13
    cmp eax, 0xffff8000
    jl .L000f1d1a
.L000f1cef:
    cmp edx, 0x7fff
    jg .L000f1d21
    cmp edx, 0xffff8000
    jl .L000f1d28
.L000f1cff:
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], ax
    mov word ptr [ebx], dx
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xc7  # jne 0xf1cd9
    ret
.L000f1d13:
    mov eax, 0x7fff
    jmp .L000f1cef
.L000f1d1a:
    mov eax, 0xffff8000
    jmp .L000f1cef
.L000f1d21:
    mov edx, 0x7fff
    jmp .L000f1cff
.L000f1d28:
    mov edx, 0xffff8000
    jmp .L000f1cff
.size "_DC_125",.-"_DC_125"

// DMA copy routine 126: converts ECX frames of the 32-bit build buffer at ESI
// to unsigned 16-bit stereo output at EDI, right channel first, high byte
// first, left and right to separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1d2f

.section _TEXT,"ax",@progbits,unique,990511
.globl "_DC_126"
.type "_DC_126",@function
"_DC_126":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1d74
    cmp eax, 0xffff8000
    jl .L000f1d7b
.L000f1d45:
    cmp edx, 0x7fff
    jg .L000f1d82
    cmp edx, 0xffff8000
    jl .L000f1d89
.L000f1d55:
    xor eax, 0x8000
    xor edx, 0x8000
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xbc  # jne 0xf1d2f
    ret
.L000f1d74:
    mov eax, 0x7fff
    jmp .L000f1d45
.L000f1d7b:
    mov eax, 0xffff8000
    jmp .L000f1d45
.L000f1d82:
    mov edx, 0x7fff
    jmp .L000f1d55
.L000f1d89:
    mov edx, 0xffff8000
    jmp .L000f1d55
.size "_DC_126",.-"_DC_126"

// DMA copy routine 127: converts ECX frames of the 32-bit build buffer at ESI
// to signed 16-bit stereo output at EDI, right channel first, high byte first,
// left and right to separate buffers (EDI and EBX), clipped to 16 bits.
// FUNCTION: 0x000f1d90

.section _TEXT,"ax",@progbits,unique,990608
.globl "_DC_127"
.type "_DC_127",@function
"_DC_127":
    mov eax, dword ptr [esi]
    mov edx, dword ptr [esi + 4]
    add esi, 8
    cmp eax, 0x7fff
    jg .L000f1dca
    cmp eax, 0xffff8000
    jl .L000f1dd1
.L000f1da6:
    cmp edx, 0x7fff
    jg .L000f1dd8
    cmp edx, 0xffff8000
    jl .L000f1ddf
.L000f1db6:
    xchg ah, al
    xchg dh, dl
    mov word ptr [edi], dx
    mov word ptr [ebx], ax
    add edi, 2
    add ebx, 2
    dec ecx
    .byte 0x75; .byte 0xc7  # jne 0xf1d90
    ret
.L000f1dca:
    mov eax, 0x7fff
    jmp .L000f1da6
.L000f1dd1:
    mov eax, 0xffff8000
    jmp .L000f1da6
.L000f1dd8:
    mov edx, 0x7fff
    jmp .L000f1db6
.L000f1ddf:
    mov edx, 0xffff8000
    jmp .L000f1db6
.size "_DC_127",.-"_DC_127"

// Merge routine 0: adds unsigned 8-bit mono sample data from ESI, unscaled, to
// the mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1de6

.section _TEXT,"ax",@progbits,unique,990694
.globl "_M_0"
.type "_M_0",@function
"_M_0":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1e06
    mov ah, byte ptr [esi]
    xor eax, 0x8000
    inc esi
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xe0  # jb 0xf1de6
.L000f1e06:
    ret
.size "_M_0",.-"_M_0"

// Merge routine 1: adds unsigned 8-bit mono sample data from ESI, unscaled, to
// the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1e07

.section _TEXT,"ax",@progbits,unique,990727
.globl "_M_1"
.type "_M_1",@function
"_M_1":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1e2a
    mov ah, byte ptr [esi]
    xor eax, 0x8000
    inc esi
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xdd  # jb 0xf1e07
.L000f1e2a:
    ret
.size "_M_1",.-"_M_1"

// Merge routine 3: adds unsigned 8-bit stereo sample data from ESI, unscaled,
// to the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1e2b

.section _TEXT,"ax",@progbits,unique,990763
.globl "_M_3"
.type "_M_3",@function
"_M_3":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1e5c
    mov ah, byte ptr [esi]
    mov bh, byte ptr [esi + 1]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcf  # jb 0xf1e2b
.L000f1e5c:
    ret
.size "_M_3",.-"_M_3"

// Merge routine 7: adds unsigned 8-bit stereo (right channel first) sample data
// from ESI, unscaled, to the stereo build buffer at EDI, until the source or
// the build buffer ends.
// FUNCTION: 0x000f1e5d

.section _TEXT,"ax",@progbits,unique,990813
.globl "_M_7"
.type "_M_7",@function
"_M_7":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1e8e
    mov bh, byte ptr [esi]
    mov ah, byte ptr [esi + 1]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcf  # jb 0xf1e5d
.L000f1e8e:
    ret
.size "_M_7",.-"_M_7"

// Merge routine 8: adds unsigned 16-bit mono sample data from ESI, unscaled, to
// the mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1e8f

.section _TEXT,"ax",@progbits,unique,990863
.globl "_M_8"
.type "_M_8",@function
"_M_8":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1eb2
    mov ax, word ptr [esi]
    xor eax, 0x8000
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xdd  # jb 0xf1e8f
.L000f1eb2:
    ret
.size "_M_8",.-"_M_8"

// Merge routine 9: adds unsigned 16-bit mono sample data from ESI, unscaled, to
// the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1eb3

.section _TEXT,"ax",@progbits,unique,990899
.globl "_M_9"
.type "_M_9",@function
"_M_9":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1ed9
    mov ax, word ptr [esi]
    xor eax, 0x8000
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xda  # jb 0xf1eb3
.L000f1ed9:
    ret
.size "_M_9",.-"_M_9"

// Merge routine 11: adds unsigned 16-bit stereo sample data from ESI, unscaled,
// to the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1eda

.section _TEXT,"ax",@progbits,unique,990938
.globl "_M_11"
.type "_M_11",@function
"_M_11":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1f0d
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 4
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcd  # jb 0xf1eda
.L000f1f0d:
    ret
.size "_M_11",.-"_M_11"

// Merge routine 15: adds unsigned 16-bit stereo (right channel first) sample
// data from ESI, unscaled, to the stereo build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f1f0e

.section _TEXT,"ax",@progbits,unique,990990
.globl "_M_15"
.type "_M_15",@function
"_M_15":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1f41
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 4
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcd  # jb 0xf1f0e
.L000f1f41:
    ret
.size "_M_15",.-"_M_15"

// Merge routine 16: adds signed 8-bit mono sample data from ESI, unscaled, to
// the mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1f42

.section _TEXT,"ax",@progbits,unique,991042
.globl "_M_16"
.type "_M_16",@function
"_M_16":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1f5d
    mov ah, byte ptr [esi]
    inc esi
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xe5  # jb 0xf1f42
.L000f1f5d:
    ret
.size "_M_16",.-"_M_16"

// Merge routine 17: adds signed 8-bit mono sample data from ESI, unscaled, to
// the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1f5e

.section _TEXT,"ax",@progbits,unique,991070
.globl "_M_17"
.type "_M_17",@function
"_M_17":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1f7c
    mov ah, byte ptr [esi]
    inc esi
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xe2  # jb 0xf1f5e
.L000f1f7c:
    ret
.size "_M_17",.-"_M_17"

// Merge routine 19: adds signed 8-bit stereo sample data from ESI, unscaled, to
// the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1f7d

.section _TEXT,"ax",@progbits,unique,991101
.globl "_M_19"
.type "_M_19",@function
"_M_19":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1fa3
    mov ah, byte ptr [esi]
    mov bh, byte ptr [esi + 1]
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xda  # jb 0xf1f7d
.L000f1fa3:
    ret
.size "_M_19",.-"_M_19"

// Merge routine 23: adds signed 8-bit stereo (right channel first) sample data
// from ESI, unscaled, to the stereo build buffer at EDI, until the source or
// the build buffer ends.
// FUNCTION: 0x000f1fa4

.section _TEXT,"ax",@progbits,unique,991140
.globl "_M_23"
.type "_M_23",@function
"_M_23":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1fca
    mov bh, byte ptr [esi]
    mov ah, byte ptr [esi + 1]
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xda  # jb 0xf1fa4
.L000f1fca:
    ret
.size "_M_23",.-"_M_23"

// Merge routine 24: adds signed 16-bit mono sample data from ESI, unscaled, to
// the mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1fcb

.section _TEXT,"ax",@progbits,unique,991179
.globl "_M_24"
.type "_M_24",@function
"_M_24":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f1fe9
    mov ax, word ptr [esi]
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xe2  # jb 0xf1fcb
.L000f1fe9:
    ret
.size "_M_24",.-"_M_24"

// Merge routine 25: adds signed 16-bit mono sample data from ESI, unscaled, to
// the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f1fea

.section _TEXT,"ax",@progbits,unique,991210
.globl "_M_25"
.type "_M_25",@function
"_M_25":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f200b
    mov ax, word ptr [esi]
    add esi, 2
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xdf  # jb 0xf1fea
.L000f200b:
    ret
.size "_M_25",.-"_M_25"

// Merge routine 27: adds signed 16-bit stereo sample data from ESI, unscaled,
// to the stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f200c

.section _TEXT,"ax",@progbits,unique,991244
.globl "_M_27"
.type "_M_27",@function
"_M_27":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2034
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    add esi, 4
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd8  # jb 0xf200c
.L000f2034:
    ret
.size "_M_27",.-"_M_27"

// Merge routine 31: adds signed 16-bit stereo (right channel first) sample data
// from ESI, unscaled, to the stereo build buffer at EDI, until the source or
// the build buffer ends.
// FUNCTION: 0x000f2035

.section _TEXT,"ax",@progbits,unique,991285
.globl "_M_31"
.type "_M_31",@function
"_M_31":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f205d
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    add esi, 4
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd8  # jb 0xf2035
.L000f205d:
    ret
.size "_M_31",.-"_M_31"

// Merge routine 32: adds unsigned 8-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the mono build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f205e

.section _TEXT,"ax",@progbits,unique,991326
.globl "_M_32"
.type "_M_32",@function
"_M_32":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2089
    mov ah, byte ptr [esi]
    xor eax, 0x8000
    add ecx, dword ptr [.Ldata+0x14]
    adc esi, dword ptr [.Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd5  # jb 0xf205e
.L000f2089:
    ret
.size "_M_32",.-"_M_32"

// Merge routine 33: adds unsigned 8-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f208a

.section _TEXT,"ax",@progbits,unique,991370
.globl "_M_33"
.type "_M_33",@function
"_M_33":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f20b8
    mov ah, byte ptr [esi]
    xor eax, 0x8000
    add ecx, dword ptr [.Ldata+0x14]
    adc esi, dword ptr [.Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd2  # jb 0xf208a
.L000f20b8:
    ret
.size "_M_33",.-"_M_33"

// Merge routine 35: adds unsigned 8-bit stereo sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f20b9

.section _TEXT,"ax",@progbits,unique,991417
.globl "_M_35"
.type "_M_35",@function
"_M_35":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f20f8
    mov ah, byte ptr [esi]
    mov bh, byte ptr [esi + 1]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc1  # jb 0xf20b9
.L000f20f8:
    ret
.size "_M_35",.-"_M_35"

// Merge routine 39: adds unsigned 8-bit stereo (right channel first) sample
// data from ESI, stepping through it at the rate ratio, unscaled, to the stereo
// build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f20f9

.section _TEXT,"ax",@progbits,unique,991481
.globl "_M_39"
.type "_M_39",@function
"_M_39":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2138
    mov bh, byte ptr [esi]
    mov ah, byte ptr [esi + 1]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc1  # jb 0xf20f9
.L000f2138:
    ret
.size "_M_39",.-"_M_39"

// Merge routine 40: adds unsigned 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the mono build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f2139

.section _TEXT,"ax",@progbits,unique,991545
.globl "_M_40"
.type "_M_40",@function
"_M_40":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f216a
    mov ax, word ptr [esi]
    xor eax, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcf  # jb 0xf2139
.L000f216a:
    ret
.size "_M_40",.-"_M_40"

// Merge routine 41: adds unsigned 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f216b

.section _TEXT,"ax",@progbits,unique,991595
.globl "_M_41"
.type "_M_41",@function
"_M_41":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f219f
    mov ax, word ptr [esi]
    xor eax, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcc  # jb 0xf216b
.L000f219f:
    ret
.size "_M_41",.-"_M_41"

// Merge routine 43: adds unsigned 16-bit stereo sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f21a0

.section _TEXT,"ax",@progbits,unique,991648
.globl "_M_43"
.type "_M_43",@function
"_M_43":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f21e1
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbf  # jb 0xf21a0
.L000f21e1:
    ret
.size "_M_43",.-"_M_43"

// Merge routine 47: adds unsigned 16-bit stereo (right channel first) sample
// data from ESI, stepping through it at the rate ratio, unscaled, to the stereo
// build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f21e2

.section _TEXT,"ax",@progbits,unique,991714
.globl "_M_47"
.type "_M_47",@function
"_M_47":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2223
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbf  # jb 0xf21e2
.L000f2223:
    ret
.size "_M_47",.-"_M_47"

// Merge routine 48: adds signed 8-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the mono build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f2224

.section _TEXT,"ax",@progbits,unique,991780
.globl "_M_48"
.type "_M_48",@function
"_M_48":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f224a
    mov ah, byte ptr [esi]
    add ecx, dword ptr [.Ldata+0x14]
    adc esi, dword ptr [.Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xda  # jb 0xf2224
.L000f224a:
    ret
.size "_M_48",.-"_M_48"

// Merge routine 49: adds signed 8-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f224b

.section _TEXT,"ax",@progbits,unique,991819
.globl "_M_49"
.type "_M_49",@function
"_M_49":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2274
    mov ah, byte ptr [esi]
    add ecx, dword ptr [.Ldata+0x14]
    adc esi, dword ptr [.Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd7  # jb 0xf224b
.L000f2274:
    ret
.size "_M_49",.-"_M_49"

// Merge routine 51: adds signed 8-bit stereo sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f2275

.section _TEXT,"ax",@progbits,unique,991861
.globl "_M_51"
.type "_M_51",@function
"_M_51":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f22a9
    mov ah, byte ptr [esi]
    mov bh, byte ptr [esi + 1]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcc  # jb 0xf2275
.L000f22a9:
    ret
.size "_M_51",.-"_M_51"

// Merge routine 55: adds signed 8-bit stereo (right channel first) sample data
// from ESI, stepping through it at the rate ratio, unscaled, to the stereo
// build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f22aa

.section _TEXT,"ax",@progbits,unique,991914
.globl "_M_55"
.type "_M_55",@function
"_M_55":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f22de
    mov bh, byte ptr [esi]
    mov ah, byte ptr [esi + 1]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcc  # jb 0xf22aa
.L000f22de:
    ret
.size "_M_55",.-"_M_55"

// Merge routine 56: adds signed 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the mono build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f22df

.section _TEXT,"ax",@progbits,unique,991967
.globl "_M_56"
.type "_M_56",@function
"_M_56":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f230b
    mov ax, word ptr [esi]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd4  # jb 0xf22df
.L000f230b:
    ret
.size "_M_56",.-"_M_56"

// Merge routine 57: adds signed 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f230c

.section _TEXT,"ax",@progbits,unique,992012
.globl "_M_57"
.type "_M_57",@function
"_M_57":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f233b
    mov ax, word ptr [esi]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd1  # jb 0xf230c
.L000f233b:
    ret
.size "_M_57",.-"_M_57"

// Merge routine 59: adds signed 16-bit stereo sample data from ESI, stepping
// through it at the rate ratio, unscaled, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f233c

.section _TEXT,"ax",@progbits,unique,992060
.globl "_M_59"
.type "_M_59",@function
"_M_59":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2372
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xca  # jb 0xf233c
.L000f2372:
    ret
.size "_M_59",.-"_M_59"

// Merge routine 63: adds signed 16-bit stereo (right channel first) sample data
// from ESI, stepping through it at the rate ratio, unscaled, to the stereo
// build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2373

.section _TEXT,"ax",@progbits,unique,992115
.globl "_M_63"
.type "_M_63",@function
"_M_63":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f23a9
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    add dword ptr [edi], ebp
    movsx ebp, bx
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xca  # jb 0xf2373
.L000f23a9:
    ret
.size "_M_63",.-"_M_63"

// Merge routine 64: adds 8-bit mono sample data from ESI, through the
// volume/pan tables at EDX, to the mono build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f23aa

.section _TEXT,"ax",@progbits,unique,992170
.globl "_M_64"
.type "_M_64",@function
"_M_64":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f23c5
    mov al, byte ptr [esi]
    inc esi
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xe5  # jb 0xf23aa
.L000f23c5:
    ret
.size "_M_64",.-"_M_64"

// Merge routine 65: adds 8-bit mono sample data from ESI, through the
// volume/pan tables at EDX, to the stereo build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f23c6

.section _TEXT,"ax",@progbits,unique,992198
.globl "_M_65"
.type "_M_65",@function
"_M_65":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f23eb
    mov al, byte ptr [esi]
    inc esi
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + eax*4 + 0x400]
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xdb  # jb 0xf23c6
.L000f23eb:
    ret
.size "_M_65",.-"_M_65"

// Merge routine 66: adds 8-bit stereo sample data from ESI, through the
// volume/pan tables at EDX, to the mono build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f23ec

.section _TEXT,"ax",@progbits,unique,992236
.globl "_M_66"
.type "_M_66",@function
"_M_66":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2415
    mov al, byte ptr [esi]
    mov bl, byte ptr [esi + 1]
    add esi, 2
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd7  # jb 0xf23ec
.L000f2415:
    ret
.size "_M_66",.-"_M_66"

// Merge routine 67: adds 8-bit stereo sample data from ESI, through the
// volume/pan tables at EDX, to the stereo build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f2416

.section _TEXT,"ax",@progbits,unique,992278
.globl "_M_67"
.type "_M_67",@function
"_M_67":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2440
    mov al, byte ptr [esi]
    mov bl, byte ptr [esi + 1]
    add esi, 2
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd6  # jb 0xf2416
.L000f2440:
    ret
.size "_M_67",.-"_M_67"

// Merge routine 70: adds 8-bit stereo (right channel first) sample data from
// ESI, through the volume/pan tables at EDX, to the mono build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f2441

.section _TEXT,"ax",@progbits,unique,992321
.globl "_M_70"
.type "_M_70",@function
"_M_70":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f246a
    mov bl, byte ptr [esi]
    mov al, byte ptr [esi + 1]
    add esi, 2
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd7  # jb 0xf2441
.L000f246a:
    ret
.size "_M_70",.-"_M_70"

// Merge routine 71: adds 8-bit stereo (right channel first) sample data from
// ESI, through the volume/pan tables at EDX, to the stereo build buffer at EDI,
// until the source or the build buffer ends.
// FUNCTION: 0x000f246b

.section _TEXT,"ax",@progbits,unique,992363
.globl "_M_71"
.type "_M_71",@function
"_M_71":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2495
    mov bl, byte ptr [esi]
    mov al, byte ptr [esi + 1]
    add esi, 2
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd6  # jb 0xf246b
.L000f2495:
    ret
.size "_M_71",.-"_M_71"

// Merge routine 72: adds unsigned 16-bit mono sample data from ESI, scaled by
// the volume/pan factors at EDX, to the mono build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f2496

.section _TEXT,"ax",@progbits,unique,992406
.globl "_M_72"
.type "_M_72",@function
"_M_72":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f24bf
    mov ax, word ptr [esi]
    xor eax, 0x8000
    add esi, 2
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd7  # jb 0xf2496
.L000f24bf:
    ret
.size "_M_72",.-"_M_72"

// Merge routine 73: adds unsigned 16-bit mono sample data from ESI, scaled by
// the volume/pan factors at EDX, to the stereo build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f24c0

.section _TEXT,"ax",@progbits,unique,992448
.globl "_M_73"
.type "_M_73",@function
"_M_73":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f24f9
    mov ax, word ptr [esi]
    xor eax, 0x8000
    add esi, 2
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, ax
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc7  # jb 0xf24c0
.L000f24f9:
    ret
.size "_M_73",.-"_M_73"

// Merge routine 74: adds unsigned 16-bit stereo sample data from ESI, scaled by
// the volume/pan factors at EDX, to the mono build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f24fa

.section _TEXT,"ax",@progbits,unique,992506
.globl "_M_74"
.type "_M_74",@function
"_M_74":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f253c
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbe  # jb 0xf24fa
.L000f253c:
    ret
.size "_M_74",.-"_M_74"

// Merge routine 75: adds unsigned 16-bit stereo sample data from ESI, scaled by
// the volume/pan factors at EDX, to the stereo build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f253d

.section _TEXT,"ax",@progbits,unique,992573
.globl "_M_75"
.type "_M_75",@function
"_M_75":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2580
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbd  # jb 0xf253d
.L000f2580:
    ret
.size "_M_75",.-"_M_75"

// Merge routine 78: adds unsigned 16-bit stereo (right channel first) sample
// data from ESI, scaled by the volume/pan factors at EDX, to the mono build
// buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2581

.section _TEXT,"ax",@progbits,unique,992641
.globl "_M_78"
.type "_M_78",@function
"_M_78":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f25c3
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbe  # jb 0xf2581
.L000f25c3:
    ret
.size "_M_78",.-"_M_78"

// Merge routine 79: adds unsigned 16-bit stereo (right channel first) sample
// data from ESI, scaled by the volume/pan factors at EDX, to the stereo build
// buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f25c4

.section _TEXT,"ax",@progbits,unique,992708
.globl "_M_79"
.type "_M_79",@function
"_M_79":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2607
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbd  # jb 0xf25c4
.L000f2607:
    ret
.size "_M_79",.-"_M_79"

// Merge routine 88: adds signed 16-bit mono sample data from ESI, scaled by the
// volume/pan factors at EDX, to the mono build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f2608

.section _TEXT,"ax",@progbits,unique,992776
.globl "_M_88"
.type "_M_88",@function
"_M_88":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f262c
    mov ax, word ptr [esi]
    add esi, 2
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xdc  # jb 0xf2608
.L000f262c:
    ret
.size "_M_88",.-"_M_88"

// Merge routine 89: adds signed 16-bit mono sample data from ESI, scaled by the
// volume/pan factors at EDX, to the stereo build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f262d

.section _TEXT,"ax",@progbits,unique,992813
.globl "_M_89"
.type "_M_89",@function
"_M_89":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2661
    mov ax, word ptr [esi]
    add esi, 2
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, ax
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xcc  # jb 0xf262d
.L000f2661:
    ret
.size "_M_89",.-"_M_89"

// Merge routine 90: adds signed 16-bit stereo sample data from ESI, scaled by
// the volume/pan factors at EDX, to the mono build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f2662

.section _TEXT,"ax",@progbits,unique,992866
.globl "_M_90"
.type "_M_90",@function
"_M_90":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2699
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc9  # jb 0xf2662
.L000f2699:
    ret
.size "_M_90",.-"_M_90"

// Merge routine 91: adds signed 16-bit stereo sample data from ESI, scaled by
// the volume/pan factors at EDX, to the stereo build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f269a

.section _TEXT,"ax",@progbits,unique,992922
.globl "_M_91"
.type "_M_91",@function
"_M_91":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f26d2
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc8  # jb 0xf269a
.L000f26d2:
    ret
.size "_M_91",.-"_M_91"

// Merge routine 94: adds signed 16-bit stereo (right channel first) sample data
// from ESI, scaled by the volume/pan factors at EDX, to the mono build buffer
// at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f26d3

.section _TEXT,"ax",@progbits,unique,992979
.globl "_M_94"
.type "_M_94",@function
"_M_94":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f270a
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc9  # jb 0xf26d3
.L000f270a:
    ret
.size "_M_94",.-"_M_94"

// Merge routine 95: adds signed 16-bit stereo (right channel first) sample data
// from ESI, scaled by the volume/pan factors at EDX, to the stereo build buffer
// at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f270b

.section _TEXT,"ax",@progbits,unique,993035
.globl "_M_95"
.type "_M_95",@function
"_M_95":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2743
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    add esi, 4
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc8  # jb 0xf270b
.L000f2743:
    ret
.size "_M_95",.-"_M_95"

// Merge routine 96: adds 8-bit mono sample data from ESI, stepping through it
// at the rate ratio, through the volume/pan tables at EDX, to the mono build
// buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2744

.section _TEXT,"ax",@progbits,unique,993092
.globl "_M_96"
.type "_M_96",@function
"_M_96":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f276a
    mov al, byte ptr [esi]
    add ecx, dword ptr [.Ldata+0x14]
    adc esi, dword ptr [.Ldata+0x18]
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xda  # jb 0xf2744
.L000f276a:
    ret
.size "_M_96",.-"_M_96"

// Merge routine 97: adds 8-bit mono sample data from ESI, stepping through it
// at the rate ratio, through the volume/pan tables at EDX, to the stereo build
// buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f276b

.section _TEXT,"ax",@progbits,unique,993131
.globl "_M_97"
.type "_M_97",@function
"_M_97":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f279b
    mov al, byte ptr [esi]
    add ecx, dword ptr [.Ldata+0x14]
    adc esi, dword ptr [.Ldata+0x18]
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + eax*4 + 0x400]
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xd0  # jb 0xf276b
.L000f279b:
    ret
.size "_M_97",.-"_M_97"

// Merge routine 98: adds 8-bit stereo sample data from ESI, stepping through it
// at the rate ratio, through the volume/pan tables at EDX, to the mono build
// buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f279c

.section _TEXT,"ax",@progbits,unique,993180
.globl "_M_98"
.type "_M_98",@function
"_M_98":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f27d3
    mov al, byte ptr [esi]
    mov bl, byte ptr [esi + 1]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc9  # jb 0xf279c
.L000f27d3:
    ret
.size "_M_98",.-"_M_98"

// Merge routine 99: adds 8-bit stereo sample data from ESI, stepping through it
// at the rate ratio, through the volume/pan tables at EDX, to the stereo build
// buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f27d4

.section _TEXT,"ax",@progbits,unique,993236
.globl "_M_99"
.type "_M_99",@function
"_M_99":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f280c
    mov al, byte ptr [esi]
    mov bl, byte ptr [esi + 1]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc8  # jb 0xf27d4
.L000f280c:
    ret
.size "_M_99",.-"_M_99"

// Merge routine 102: adds 8-bit stereo (right channel first) sample data from
// ESI, stepping through it at the rate ratio, through the volume/pan tables at
// EDX, to the mono build buffer at EDI, until the source or the build buffer
// ends.
// FUNCTION: 0x000f280d

.section _TEXT,"ax",@progbits,unique,993293
.globl "_M_102"
.type "_M_102",@function
"_M_102":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2844
    mov bl, byte ptr [esi]
    mov al, byte ptr [esi + 1]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc9  # jb 0xf280d
.L000f2844:
    ret
.size "_M_102",.-"_M_102"

// Merge routine 103: adds 8-bit stereo (right channel first) sample data from
// ESI, stepping through it at the rate ratio, through the volume/pan tables at
// EDX, to the stereo build buffer at EDI, until the source or the build buffer
// ends.
// FUNCTION: 0x000f2845

.section _TEXT,"ax",@progbits,unique,993349
.globl "_M_103"
.type "_M_103",@function
"_M_103":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f287d
    mov bl, byte ptr [esi]
    mov al, byte ptr [esi + 1]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    mov ebp, dword ptr [edx + eax*4]
    add dword ptr [edi], ebp
    mov ebp, dword ptr [edx + ebx*4 + 0x400]
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc8  # jb 0xf2845
.L000f287d:
    ret
.size "_M_103",.-"_M_103"

// Merge routine 104: adds unsigned 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f287e

.section _TEXT,"ax",@progbits,unique,993406
.globl "_M_104"
.type "_M_104",@function
"_M_104":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f28b5
    mov ax, word ptr [esi]
    xor eax, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xc9  # jb 0xf287e
.L000f28b5:
    ret
.size "_M_104",.-"_M_104"

// Merge routine 105: adds unsigned 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f28b6

.section _TEXT,"ax",@progbits,unique,993462
.globl "_M_105"
.type "_M_105",@function
"_M_105":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f28fd
    mov ax, word ptr [esi]
    xor eax, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, ax
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xb9  # jb 0xf28b6
.L000f28fd:
    ret
.size "_M_105",.-"_M_105"

// Merge routine 106: adds unsigned 16-bit stereo sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f28fe

.section _TEXT,"ax",@progbits,unique,993534
.globl "_M_106"
.type "_M_106",@function
"_M_106":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f294e
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xb0  # jb 0xf28fe
.L000f294e:
    ret
.size "_M_106",.-"_M_106"

// Merge routine 107: adds unsigned 16-bit stereo sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f294f

.section _TEXT,"ax",@progbits,unique,993615
.globl "_M_107"
.type "_M_107",@function
"_M_107":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f29a0
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xaf  # jb 0xf294f
.L000f29a0:
    ret
.size "_M_107",.-"_M_107"

// Merge routine 110: adds unsigned 16-bit stereo (right channel first) sample
// data from ESI, stepping through it at the rate ratio, scaled by the
// volume/pan factors at EDX, to the mono build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f29a1

.section _TEXT,"ax",@progbits,unique,993697
.globl "_M_110"
.type "_M_110",@function
"_M_110":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f29f1
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xb0  # jb 0xf29a1
.L000f29f1:
    ret
.size "_M_110",.-"_M_110"

// Merge routine 111: adds unsigned 16-bit stereo (right channel first) sample
// data from ESI, stepping through it at the rate ratio, scaled by the
// volume/pan factors at EDX, to the stereo build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f29f2

.section _TEXT,"ax",@progbits,unique,993778
.globl "_M_111"
.type "_M_111",@function
"_M_111":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2a43
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    xor eax, 0x8000
    xor ebx, 0x8000
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xaf  # jb 0xf29f2
.L000f2a43:
    ret
.size "_M_111",.-"_M_111"

// Merge routine 120: adds signed 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2a44

.section _TEXT,"ax",@progbits,unique,993860
.globl "_M_120"
.type "_M_120",@function
"_M_120":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2a76
    mov ax, word ptr [esi]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xce  # jb 0xf2a44
.L000f2a76:
    ret
.size "_M_120",.-"_M_120"

// Merge routine 121: adds signed 16-bit mono sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2a77

.section _TEXT,"ax",@progbits,unique,993911
.globl "_M_121"
.type "_M_121",@function
"_M_121":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2ab9
    mov ax, word ptr [esi]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, ax
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbe  # jb 0xf2a77
.L000f2ab9:
    ret
.size "_M_121",.-"_M_121"

// Merge routine 122: adds signed 16-bit stereo sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// mono build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2aba

.section _TEXT,"ax",@progbits,unique,993978
.globl "_M_122"
.type "_M_122",@function
"_M_122":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2aff
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbb  # jb 0xf2aba
.L000f2aff:
    ret
.size "_M_122",.-"_M_122"

// Merge routine 123: adds signed 16-bit stereo sample data from ESI, stepping
// through it at the rate ratio, scaled by the volume/pan factors at EDX, to the
// stereo build buffer at EDI, until the source or the build buffer ends.
// FUNCTION: 0x000f2b00

.section _TEXT,"ax",@progbits,unique,994048
.globl "_M_123"
.type "_M_123",@function
"_M_123":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2b46
    mov ax, word ptr [esi]
    mov bx, word ptr [esi + 2]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xba  # jb 0xf2b00
.L000f2b46:
    ret
.size "_M_123",.-"_M_123"

// Merge routine 126: adds signed 16-bit stereo (right channel first) sample
// data from ESI, stepping through it at the rate ratio, scaled by the
// volume/pan factors at EDX, to the mono build buffer at EDI, until the source
// or the build buffer ends.
// FUNCTION: 0x000f2b47

.section _TEXT,"ax",@progbits,unique,994119
.globl "_M_126"
.type "_M_126",@function
"_M_126":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2b8c
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi], ebp
    add edi, 4
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xbb  # jb 0xf2b47
.L000f2b8c:
    ret
.size "_M_126",.-"_M_126"

// Merge routine 127: adds signed 16-bit stereo (right channel first) sample
// data from ESI, stepping through it at the rate ratio, scaled by the
// volume/pan factors at EDX, to the stereo build buffer at EDI, until the
// source or the build buffer ends.
// FUNCTION: 0x000f2b8d

.section _TEXT,"ax",@progbits,unique,994189
.globl "_M_127"
.type "_M_127",@function
"_M_127":
    cmp esi, dword ptr [.Ldata+0x8]
    jae .L000f2bd3
    mov bx, word ptr [esi]
    mov ax, word ptr [esi + 2]
    {load} xor ebp, ebp
    add ecx, dword ptr [.Ldata+0x14]
    {load} adc ebp, ebp
    add esi, dword ptr [ebp*4 + .Ldata+0x18]
    movsx ebp, ax
    imul ebp, dword ptr [edx]
    sar ebp, 7
    add dword ptr [edi], ebp
    movsx ebp, bx
    imul ebp, dword ptr [edx + 0x400]
    sar ebp, 7
    add dword ptr [edi + 4], ebp
    add edi, 8
    cmp edi, dword ptr [.Ldata+0x10]
    .byte 0x72; .byte 0xba  # jb 0xf2b8d
.L000f2bd3:
    ret
.size "_M_127",.-"_M_127"

// Mixes `amount` bytes of sample `S`'s current buffer, from its position, into
// its driver's build buffer at byte offset `dest_offset`, through the merge
// routine M_n for n = output stereo (1) | sample stereo (2) | sample channels
// swapped (4) | 16-bit sample (8) | signed sample (16) | resampling (32, when
// the rate differs from the DMA rate by more than DIG_RESAMPLING_TOLERANCE) |
// volume/pan scaling (64, skipped at full volume and centre pan unless a stereo
// sample goes to mono output). Advances the sample's position past what it
// used. Returns the new build buffer offset.
// FUNCTION: 0x000f2bd4

.section _TEXT,"ax",@progbits,unique,994260
.globl "_AILSSA_merge"
.type "_AILSSA_merge",@function
"_AILSSA_merge":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push ds
    push es
    mov ecx, dword ptr [ebp + 8]
    mov edi, dword ptr [ecx + 4]
    mov eax, 0
    cmp dword ptr [edi + 0x1c], 2
    je .L000f2bf3
    cmp dword ptr [edi + 0x1c], 3
    jne .L000f2bf6
.L000f2bf3:
    or eax, 1
.L000f2bf6:
    cmp dword ptr [ecx + 0x44], 2
    je .L000f2c02
    cmp dword ptr [ecx + 0x44], 3
    jne .L000f2c11
.L000f2c02:
    or eax, 2
    test dword ptr [ecx + 0x48], 2
    je .L000f2c11
    or eax, 4
.L000f2c11:
    cmp dword ptr [ecx + 0x44], 1
    je .L000f2c1d
    cmp dword ptr [ecx + 0x44], 3
    jne .L000f2c20
.L000f2c1d:
    or eax, 8
.L000f2c20:
    test eax, 2
    je .L000f2c2e
    test eax, 1
    je .L000f2c40
.L000f2c2e:
    cmp dword ptr [ecx + 0x50], 0x7f
    jne .L000f2c40
    cmp dword ptr [edi + 0x6c], 0x7f
    jne .L000f2c40
    cmp dword ptr [ecx + 0x54], 0x40
    je .L000f2c4a
.L000f2c40:
    or eax, 0x40
    test eax, 8
    je .L000f2c56
.L000f2c4a:
    test dword ptr [ecx + 0x48], 1
    je .L000f2c56
    or eax, 0x10
.L000f2c56:
    mov dword ptr [.Ldata], eax
    mov eax, 0
    mov edx, dword ptr [ecx + 0x4c]
    cmp edx, 0
    jg .L000f2c6d
    mov edx, 1
.L000f2c6d:
    mov ebx, dword ptr [edi + 0x18]
    shl ebx, 0x10
    div ebx
    {load} mov edx, eax
    sub edx, 0x10000
    jg .L000f2c81
    neg edx
.L000f2c81:
    cmp edx, dword ptr ["_AIL_preference"+0x18]
    jbe .L000f2ccb
    {load} mov ebx, eax
    shr ebx, 0x10
    shl eax, 0x10
    mov dword ptr [.Ldata+0x14], eax
    {load} mov eax, ebx
    inc ebx
    test dword ptr [.Ldata], 2
    je .L000f2ca9
    shl eax, 1
    shl ebx, 1
.L000f2ca9:
    test dword ptr [.Ldata], 8
    je .L000f2cb9
    shl eax, 1
    shl ebx, 1
.L000f2cb9:
    mov dword ptr [.Ldata+0x18], eax
    mov dword ptr [.Ldata+0x1c], ebx
    or dword ptr [.Ldata], 0x20
.L000f2ccb:
    mov ebx, dword ptr [edi + 0x50]
    mov eax, dword ptr [edi + 0x54]
    mov dword ptr [.Ldata+0xc], eax
    {load} add ebx, eax
    mov dword ptr [.Ldata+0x10], ebx
    {load} mov edi, eax
    add edi, dword ptr [ebp + 0x10]
    mov ebx, dword ptr [ecx + 0x2c]
    mov esi, dword ptr [ecx + ebx*4 + 0xc]
    add esi, dword ptr [ecx + ebx*4 + 0x1c]
    {load} mov eax, esi
    add eax, dword ptr [ebp + 0xc]
    mov dword ptr [.Ldata+0x8], eax
    lea edx, [ecx + 0x58]
    mov eax, 0
    mov ebx, 0
    mov ecx, 0x80000000
    push ebp
    mov ebp, dword ptr [.Ldata]
    call dword ptr cs:[ebp*4 + "ssa_dispatch_table"+0x200]
    pop ebp
    mov ecx, dword ptr [ebp + 8]
    mov ebx, dword ptr [ecx + 0x2c]
    sub esi, dword ptr [ecx + ebx*4 + 0xc]
    mov dword ptr [ecx + ebx*4 + 0x1c], esi
    {load} mov eax, edi
    sub eax, dword ptr [.Ldata+0xc]
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AILSSA_merge",.-"_AILSSA_merge"

// Converts `dig`'s build buffer into DMA half-buffer `buffer` (0 or 1) in the
// hardware's format, through the copy routine DC_n for n = the hardware mode
// flags' low 4 bits | stereo (16) | 16-bit (32) | clipping (64, when more than
// one sample is playing). Hardware with mode flag 0x20 gets left and right in
// the two halves of the DMA buffer instead.
// FUNCTION: 0x000f2d37

.section _TEXT,"ax",@progbits,unique,994615
.globl "_AILSSA_DMA_copy"
.type "_AILSSA_DMA_copy",@function
"_AILSSA_DMA_copy":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push ds
    push es
    mov ebx, dword ptr [ebp + 8]
    mov eax, dword ptr [ebx + 0x20]
    and eax, 0xf
    cmp dword ptr [ebx + 0x1c], 2
    je .L000f2d54
    cmp dword ptr [ebx + 0x1c], 3
    jne .L000f2d57
.L000f2d54:
    or eax, 0x10
.L000f2d57:
    cmp dword ptr [ebx + 0x1c], 1
    je .L000f2d63
    cmp dword ptr [ebx + 0x1c], 3
    jne .L000f2d66
.L000f2d63:
    or eax, 0x20
.L000f2d66:
    cmp dword ptr [ebx + 0x68], 1
    jbe .L000f2d6f
    or eax, 0x40
.L000f2d6f:
    mov dword ptr [.Ldata], eax
    mov esi, dword ptr [ebx + 0x54]
    mov ecx, dword ptr [ebx + 0x4c]
    test dword ptr [ebx + 0x20], 0x20
    jne .L000f2d9d
    mov eax, dword ptr [ebp + 0xc]
    mov edi, dword ptr [ebx + eax*4 + 0x30]
    test dword ptr [ebx + 0x20], 8
    je .L000f2db6
    mov eax, dword ptr [ebx + 0x14]
    shr eax, 1
    lea ebx, [eax + edi]
    jmp .L000f2db6
.L000f2d9d:
    or dword ptr [.Ldata], 8
    mov eax, dword ptr [ebx + 0x14]
    mov edi, dword ptr [ebx + 0x30]
    lea ebx, [eax + edi]
    shr eax, 1
    mul dword ptr [ebp + 0xc]
    {load} add edi, eax
    {load} add ebx, eax
.L000f2db6:
    mov eax, dword ptr [.Ldata]
    call dword ptr cs:[eax*4 + "ssa_dispatch_table"]
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AILSSA_DMA_copy",.-"_AILSSA_DMA_copy"

// Clears `dig`'s build buffer. The Miles copyright string follows its code.
// FUNCTION: 0x000f2dca

.section _TEXT,"ax",@progbits,unique,994762
.globl "_AILSSA_flush_buffer"
.type "_AILSSA_flush_buffer",@function
"_AILSSA_flush_buffer":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push ds
    push es
    cld
    push ds
    pop es
    mov ebx, dword ptr [ebp + 8]
    mov edi, dword ptr [ebx + 0x54]
    mov ecx, dword ptr [ebx + 0x50]
    mov eax, 0
    push ecx
    and ecx, 3
    rep stosb byte ptr es:[edi], al
    pop ecx
    shr ecx, 2
    rep stosd dword ptr es:[edi], eax
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
    .byte 0x0d; .byte 0x0a; .byte 0x0d; .byte 0x0a; .byte 0x4d
    .byte 0x69; .byte 0x6c; .byte 0x65; .byte 0x73; .byte 0x20; .byte 0x53; .byte 0x6f; .byte 0x75
    .byte 0x6e
    .byte 0x64; .byte 0x20; .byte 0x53; .byte 0x79
    .byte 0x73; .byte 0x74
    .byte 0x65; .byte 0x6d
    .byte 0x0d; .byte 0x0a; .byte 0x43; .byte 0x6f; .byte 0x70
    .byte 0x79; .byte 0x72
    .byte 0x69; .byte 0x67; .byte 0x68; .byte 0x74; .byte 0x20; .byte 0x28; .byte 0x43
    .byte 0x29; .byte 0x20
    .byte 0x31; .byte 0x39
    .byte 0x39; .byte 0x31
    .byte 0x2d; .byte 0x39; .byte 0x36; .byte 0x20; .byte 0x52
    .byte 0x41
    .byte 0x44
    .byte 0x20; .byte 0x47; .byte 0x61
    .byte 0x6d
    .byte 0x65; .byte 0x20; .byte 0x54; .byte 0x6f; .byte 0x6f
    .byte 0x6c
    .byte 0x73; .byte 0x2c
    .byte 0x20; .byte 0x49; .byte 0x6e
    .byte 0x63; .byte 0x2e
    .byte 0x0d; .byte 0x0a; .byte 0x0d; .byte 0x0a
.size "_AILSSA_flush_buffer",.-"_AILSSA_flush_buffer"

// Locks the mixer's data and code (the dispatch table up to here) against
// paging.
// FUNCTION: 0x000f2e3c

.section _TEXT,"ax",@progbits,unique,994876
.globl "_AILSSA_VMM_lock"
.type "_AILSSA_VMM_lock",@function
"_AILSSA_VMM_lock":
    push ebx
    push esi
    push edi
    push ds
    push es
    .byte 0x68; .long "___WD_Present"  # push 0x31b88
    .byte 0x68; .long .Ldata  # push 0x31b68
    call "_AIL_vmm_lock_range"
    add esp, 8
    .byte 0x68; .long "_AILSSA_VMM_lock"  # push 0xe2e3c
    .byte 0x68; .long "ssa_dispatch_table"  # push 0xe0da8
    call "_AIL_vmm_lock_range"
    add esp, 8
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    ret
.size "_AILSSA_VMM_lock",.-"_AILSSA_VMM_lock"

.section _DATA,"aw",@progbits
.Ldata:
    .fill 8, 4, 0
