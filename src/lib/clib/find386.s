.intel_syntax noprefix

// _dos_findfirst: find the first file matching path and attributes attr, results in *buf (DOS 4Eh with buf as the DTA); returns 0 or the DOS error.
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
    call "fn_000d1264_"
    pop ecx
    ret
.size "_dos_findfirst_",.-"_dos_findfirst_"

// _dos_findnext: find the next file of the search in *buf (DOS 4Fh); returns 0 or the DOS error.
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
    call "fn_000d1264_"
    pop edx
    ret
.size "_dos_findnext_",.-"_dos_findnext_"

// _dos_findclose: end a search; nothing to release, returns 0.
// FUNCTION: 0x000d1261

.section _TEXT,"ax",@progbits,unique,856673
.globl "_dos_findclose_"
.type "_dos_findclose_",@function
"_dos_findclose_":
    xor eax, eax
    ret
.size "_dos_findclose_",.-"_dos_findclose_"

// fn_000d1264: under CodeBuilder (_Extender 9), copy the 43-byte DTA into the caller's find buffer edx.
// FUNCTION: 0x000d1264

.section _TEXT,"ax",@progbits,unique,856676
.globl "fn_000d1264_"
.type "fn_000d1264_",@function
"fn_000d1264_":
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
.size "fn_000d1264_",.-"fn_000d1264_"

// fn_000d1287: under CodeBuilder (_Extender 9), copy the caller's find buffer edx into the DTA.
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
