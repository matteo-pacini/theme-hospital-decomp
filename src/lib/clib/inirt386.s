.intel_syntax noprefix

// __InitRtns: run each startup (XI) routine with priority <= al, lowest priority first, marking each done.
// FUNCTION: 0x000d05b0

.section _TEXT,"ax",@progbits,unique,853424
.globl "__InitRtns"
.type "__InitRtns",@function
"__InitRtns":
    push esi
    push edi
    push ebx
    push edx
    push es
    mov edx, eax
.L000d05b7:
    .byte 0x3e; .byte 0xbe; .long "W?.mod_init$npn()v"  # mov esi, 0x33d64
    .byte 0x3e; .byte 0xbf; .long "___wcpp_2_data_module_dtor_ref_"  # mov edi, 0x33dc4
    mov ebx, edi
    mov al, dl
.L000d05c7:
    cmp esi, edi
    jae .L000d05df
    cmp byte ptr [esi], 2
    je .L000d05da
    cmp byte ptr [esi + 1], al
    ja .L000d05da
    mov ebx, esi
    mov al, byte ptr [esi + 1]
.L000d05da:
    add esi, 6
    jmp .L000d05c7
.L000d05df:
    cmp ebx, edi
    je .L000d05f5
    mov eax, dword ptr [ebx + 2]
    or eax, eax
    je .L000d05f0
    push ds
    pop es
    push edx
    call eax
    pop edx
.L000d05f0:
    mov byte ptr [ebx], 2
    jmp .L000d05b7
.L000d05f5:
    pop es
    pop edx
    pop ebx
    pop edi
    pop esi
    ret
.size "__InitRtns",.-"__InitRtns"

// __FiniRtns: run each exit (YI) routine with priority >= al, highest first, calling only those <= dl; each is marked done.
// FUNCTION: 0x000d05fb

.section _TEXT,"ax",@progbits,unique,853499
.globl "__FiniRtns"
.type "__FiniRtns",@function
"__FiniRtns":
    push esi
    push edi
    push ebx
    push es
    mov dh, dl
    mov dl, al
.L000d0603:
    .byte 0x3e; .byte 0xbe; .long "___wcpp_2_data_module_dtor_ref_"  # mov esi, 0x33dc4
    .byte 0x3e; .byte 0xbf; .long "__End_YI"  # mov edi, 0x33dd6
    mov ebx, edi
    mov al, dl
.L000d0613:
    cmp esi, edi
    jae .L000d062b
    cmp byte ptr [esi], 2
    je .L000d0626
    cmp al, byte ptr [esi + 1]
    ja .L000d0626
    mov ebx, esi
    mov al, byte ptr [esi + 1]
.L000d0626:
    add esi, 6
    jmp .L000d0613
.L000d062b:
    cmp ebx, edi
    je .L000d0645
    cmp al, dh
    ja .L000d0640
    mov eax, dword ptr [ebx + 2]
    or eax, eax
    je .L000d0640
    push ds
    pop es
    push edx
    call eax
    pop edx
.L000d0640:
    mov byte ptr [ebx], 2
    jmp .L000d0603
.L000d0645:
    pop es
    pop ebx
    pop edi
    pop esi
    ret
.globl "__null_FPE_rtn"
"__null_FPE_rtn":
    .byte 0xc3
.L000d064b:
    .byte 0x53
    .byte 0x51
    .byte 0x52
    .byte 0x56
    .byte 0x57
    .byte 0x55
    .byte 0x83; .byte 0xec; .byte 0x0c
    .byte 0x89; .byte 0xe3
    .byte 0xa1; .long "__LpCmdLine"
    .byte 0x31; .byte 0xd2
    .byte 0xe8; .long "_SplitParms_"-.-4
    .byte 0x8b; .byte 0x0c; .byte 0x24
    .byte 0x40
    .byte 0x8b; .byte 0x15; .long "__LpCmdLine"
    .byte 0x89; .byte 0x44; .byte 0x24; .byte 0x08
    .byte 0x29; .byte 0xd1
    .byte 0xc1; .byte 0xe0; .byte 0x02
    .byte 0x41
    .byte 0x89; .byte 0x44; .byte 0x24; .byte 0x04
    .byte 0x83; .byte 0xc0; .byte 0x04
    .byte 0x01; .byte 0xc8
    .byte 0xe8; .long "_nmalloc_"-.-4
    .byte 0x89; .byte 0xc5
    .byte 0x85; .byte 0xc0
    .byte 0x74; .byte .L000d06e5-.-1
    .byte 0x89; .byte 0xe3
    .byte 0x8b; .byte 0x35; .long "__LpCmdLine"
    .byte 0x01; .byte 0xc8
    .byte 0x89; .byte 0xef
    .byte 0xa3; .long "__argv"
    .byte 0x06
    .byte 0x8c; .byte 0xd8
    .byte 0x8e; .byte 0xc0
    .byte 0x57
    .byte 0x89; .byte 0xc8
    .byte 0xc1; .byte 0xe9; .byte 0x02
    .byte 0xf2; .byte 0xa5
    .byte 0x8a; .byte 0xc8
    .byte 0x80; .byte 0xe1; .byte 0x03
    .byte 0xf2; .byte 0xa4
    .byte 0x5f
    .byte 0x07
    .byte 0x8b; .byte 0x0d; .long "__argv"
    .byte 0xa1; .long "__LpPgmName"
    .byte 0x8d; .byte 0x51; .byte 0x04
    .byte 0x89; .byte 0x01
    .byte 0x89; .byte 0xe8
    .byte 0x8b; .byte 0x74; .byte 0x24; .byte 0x04
    .byte 0xe8; .long "_SplitParms_"-.-4
    .byte 0x8b; .byte 0x0d; .long "__argv"
    .byte 0x8b; .byte 0x44; .byte 0x24; .byte 0x08
    .byte 0x01; .byte 0xf1
    .byte 0xa3; .long "__argc"
    .byte 0xc7; .byte 0x01; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0xeb; .byte .L000d06ef-.-1
.L000d06e5:
    .byte 0xa3; .long "__argv"
    .byte 0xa3; .long "__argc"
.L000d06ef:
    .byte 0x8b; .byte 0x44; .byte 0x24; .byte 0x08
    .byte 0xa3; .long "____Argc"
    .byte 0xa1; .long "__argv"
    .byte 0xa3; .long "____Argv"
    .byte 0x83; .byte 0xc4; .byte 0x0c
    .byte 0x5d
    .byte 0x5f
    .byte 0x5e
    .byte 0x5a
    .byte 0x59
    .byte 0x5b
    .byte 0xc3
.size "__FiniRtns",.-"__FiniRtns"

// argcv386's XI entry: __Init_Argv (0xd064b) at priority 0x20.
.section XI,"aw",@progbits
    .byte 0x00, 0x20
    .long .L000d064b
