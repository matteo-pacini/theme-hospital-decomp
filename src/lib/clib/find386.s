.intel_syntax noprefix

// FUNCTION: 0x000d1228

.section _TEXT,"ax",@progbits,unique,856616
.globl "_dos_findfirst_"
.type "_dos_findfirst_",@function
"_dos_findfirst_":
    push ecx
    push eax
    push edx
    mov edx, ebx
    mov ah, 0x1a
    int 0x21
    pop ecx
    pop edx
    mov ah, 0x4e
    int 0x21
    call "__doserror_"
    mov edx, ebx
    call "_dos_findclose_"
    pop ecx
    ret
.size "_dos_findfirst_",.-"_dos_findfirst_"

// FUNCTION: 0x000d1245

.section _TEXT,"ax",@progbits,unique,856645
.globl "_dos_findnext_"
.type "_dos_findnext_",@function
"_dos_findnext_":
    push edx
    mov edx, eax
    mov ah, 0x1a
    int 0x21
    call "fn_000d1287_"
    mov ah, 0x4f
    int 0x21
    call "__doserror_"
    call "_dos_findclose_"
    pop edx
    ret
    .byte 0x31; .byte 0xc0
    .byte 0xc3
.size "_dos_findnext_",.-"_dos_findnext_"

// FUNCTION: 0x000d1264

.section _TEXT,"ax",@progbits,unique,856676
.globl "_dos_findclose_"
.type "_dos_findclose_",@function
"_dos_findclose_":
    cmp byte ptr ["__Extender"], 9
    jne .L000d1286
    push eax
    push ebx
    push ecx
    push esi
    push edi
    mov ah, 0x2f
    int 0x21
    mov esi, ebx
    mov edi, edx
    mov ecx, 0x2b
    rep movsb byte ptr es:[edi], byte ptr [esi]
    pop edi
    pop esi
    pop ecx
    pop ebx
    pop eax
.L000d1286:
    ret
.size "_dos_findclose_",.-"_dos_findclose_"

// FUNCTION: 0x000d1287

.section _TEXT,"ax",@progbits,unique,856711
.globl "fn_000d1287_"
.type "fn_000d1287_",@function
"fn_000d1287_":
    cmp byte ptr ["__Extender"], 9
    jne .L000d12a9
    push eax
    push ebx
    push ecx
    push esi
    push edi
    mov ah, 0x2f
    int 0x21
    mov edi, ebx
    mov esi, edx
    mov ecx, 0x2b
    rep movsb byte ptr es:[edi], byte ptr [esi]
    pop edi
    pop esi
    pop ecx
    pop ebx
    pop eax
.L000d12a9:
    ret
.size "fn_000d1287_",.-"fn_000d1287_"
