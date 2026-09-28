.intel_syntax noprefix

// FUNCTION: 0x000fe040

.section _TEXT,"ax",@progbits,unique,1040448
.globl "reqspr_draw_"
.type "reqspr_draw_",@function
"reqspr_draw_":
    .byte 0x60  # pushal
    add esp, -0xc
    mov cl, byte ptr [ebx + 4]
    {load} or cl, cl
    je .L000fe0d1
    {load} xor ecx, ecx
    mov cl, byte ptr [ebx + 5]
    {load} mov ebp, ecx
    {load} or cl, cl
    je .L000fe0d1
    mov esi, dword ptr [ebx]
    add eax, dword ptr [_D_00188c3c]
    add edx, dword ptr [_D_00188c40]
    mov ecx, dword ptr ["W?research_names$npnpna"]
    mov dword ptr [esp + 8], ecx
    imul edx, ecx
    {load} add edx, eax
    add edx, dword ptr ["_lbDisplay"+0x4]
    {load} mov edi, edx
    {load} xor eax, eax
    {load} mov ebx, ebp
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L000fe094
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L000fe08c:
    and ecx, 0x3f
    {load} add edx, ecx
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L000fe094:
    {load} mov eax, edx
    mov cl, byte ptr [esi]
    and eax, 3
    inc esi
    lea eax, [eax + ecx*4]
    jmp dword ptr [eax*4 + .L000fe0d8]
    .byte 0x8b; .byte 0xc0
.L000fe0a8:
    {load} mov eax, edx
    mov cl, byte ptr [esi]
    and eax, 3
    inc esi
    lea eax, [eax + ecx*4]
    jmp dword ptr [eax*4 + .L000ff0d8]
    .byte 0x8b; .byte 0xc0
.L000fe0bc:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L000fe094
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
    .byte 0x83; .byte 0xc4; .byte 0x10
    .byte 0x61
    .byte 0xc3
.L000fe0d1:
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L000fe0d8:
    .long .L000fe0bc
    .long .L000fe0bc
    .long .L000fe0bc
    .long .L000fe0bc
    .long .L001000e1
    .long .L001000e1
    .long .L001000e1
    .long .L001000e1
    .long .L001000ee
    .long .L001000ee
    .long .L001000ee
    .long .L001000ee
    .long .L00100101
    .long .L00100101
    .long .L00100101
    .long .L00100101
    .long .L0010026a
    .long .L00100153
    .long .L0010019e
    .long .L001001fb
    .long .L001002c2
    .long .L001001bd
    .long .L0010021a
    .long .L00100277
    .long .L00100332
    .long .L0010023f
    .long .L0010028a
    .long .L001002d5
    .long .L001003ba
    .long .L001002a3
    .long .L001002ee
    .long .L0010034b
    .long .L0010045a
    .long .L0010030d
    .long .L0010036a
    .long .L001003d9
    .long .L001004ca
    .long .L0010038f
    .long .L001003fe
    .long .L0010046d
    .long .L00100552
    .long .L00100429
    .long .L00100486
    .long .L001004e3
    .long .L001005f2
    .long .L001004a5
    .long .L00100502
    .long .L00100571
    .long .L001006aa
    .long .L00100527
    .long .L00100596
    .long .L00100617
    .long .L00100732
    .long .L001005c1
    .long .L00100642
    .long .L001006c3
    .long .L001007d2
    .long .L00100673
    .long .L001006e2
    .long .L00100751
    .long .L0010088a
    .long .L00100707
    .long .L00100776
    .long .L001007f7
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L0010095a
    .long .L00100c81
    .long .L00100df2
    .long .L00100f75
    .long .L00100b22
    .long .L00100e77
    .long .L00100ffa
    .long .L001009b7
    .long .L00100d06
    .long .L00101085
    .long .L00100a2a
    .long .L00100b89
    .long .L00100f02
    .long .L00100aa3
    .long .L00100c02
    .long .L00100d73
    .long .L00101247
    .long .L00101181
    .long .L001011ba
    .long .L001011fc
    .long .L001012b1
    .long .L001011d0
    .long .L00101212
    .long .L0010125d
    .long .L00101327
    .long .L0010122b
    .long .L00101276
    .long .L001012ca
    .long .L001013a9
    .long .L00101292
    .long .L001012e6
    .long .L00101343
    .long .L00101437
    .long .L00101305
    .long .L00101362
    .long .L001013c8
    .long .L001014ad
    .long .L00101384
    .long .L001013ea
    .long .L00101450
    .long .L0010152f
    .long .L0010140f
    .long .L0010146c
    .long .L001014c9
    .long .L001015bd
    .long .L0010148b
    .long .L001014e8
    .long .L0010154e
    .long .L00101657
    .long .L0010150a
    .long .L00101570
    .long .L001015df
    .long .L001016d9
    .long .L00101595
    .long .L00101604
    .long .L00101673
    .long .L00101767
    .long .L0010162c
    .long .L00101692
    .long .L001016f8
    .long .L00101801
    .long .L001016b4
    .long .L0010171a
    .long .L00101789
    .long .L001018a7
    .long .L0010173f
    .long .L001017ae
    .long .L00101826
    .long .L00101935
    .long .L001017d6
    .long .L0010184e
    .long .L001018c6
    .long .L001019cf
    .long .L00101879
    .long .L001018e8
    .long .L00101957
    .long .L00101a75
    .long .L0010190d
    .long .L0010197c
    .long .L001019f4
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L00101b27
    .long .L00102262
    .long .L0010259e
    .long .L001028f5
    .long .L00101f41
    .long .L001026c1
    .long .L00102a18
    .long .L00101c0e
    .long .L00102385
    .long .L00102b44
    .long .L00101d16
    .long .L00102037
    .long .L001027ed
    .long .L00101e27
    .long .L00102148
    .long .L00102484
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L000fe08c
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L00102846
    .long .L00101e86
    .long .L001021a7
    .long .L001024e3
    .long .L00101b75
    .long .L001022c4
    .long .L00102600
    .long .L00102957
    .long .L00101f94
    .long .L00102726
    .long .L00102a7d
    .long .L00101c67
    .long .L001023db
    .long .L00102bac
    .long .L00101d72
    .long .L00102093
    .long .L000fe0a8
    .long .L000fe0a8
    .long .L000fe0a8
    .long .L000fe0a8
.L000ff0d8:
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .long .L00101bc3
    .long .L00102326
    .long .L00102662
    .long .L001029b9
    .long .L00101fe7
    .long .L0010278b
    .long .L00102ae2
    .long .L00101cc0
    .long .L00102431
    .long .L00102c14
    .long .L00101dce
    .long .L001020ef
    .long .L0010289f
    .long .L00101ee5
    .long .L00102206
    .long .L00102542
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L001000e1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001000ee:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100101:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L00100153:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L0010019e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001001bd:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L001001fb:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010021a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010023f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010026a:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100277:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010028a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001002a3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001002c2:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001002d5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001002ee:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010030d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100332:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010034b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010036a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010038f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001003ba:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001003d9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001003fe:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100429:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010045a:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L0010046d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100486:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001004a5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001004ca:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001004e3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100502:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100527:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100552:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100571:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100596:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001005c1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001005f2:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100617:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100642:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100673:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001006aa:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001006c3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001006e2:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100707:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100732:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100751:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L00100776:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x8a; .byte 0x46; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x0f
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L001007d2:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
.L001007f7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8b; .byte 0x46; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x06
    .byte 0x89; .byte 0x42; .byte 0x06
    .byte 0x8b; .byte 0x46; .byte 0x0a
    .byte 0x89; .byte 0x42; .byte 0x0a
    .byte 0x8a; .byte 0x46; .byte 0x0e
    .byte 0x88; .byte 0x42; .byte 0x0e
    .byte 0x8a; .byte 0x46; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x0f
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x8a; .byte 0x46; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x0f
    .byte 0x8a; .byte 0x46; .byte 0x10
    .byte 0x88; .byte 0x42; .byte 0x10
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L0010088a:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x01
    .byte 0x89; .byte 0x42; .byte 0x01
    .byte 0x8b; .byte 0x46; .byte 0x05
    .byte 0x89; .byte 0x42; .byte 0x05
    .byte 0x8b; .byte 0x46; .byte 0x09
    .byte 0x89; .byte 0x42; .byte 0x09
    .byte 0x8a; .byte 0x46; .byte 0x0d
    .byte 0x88; .byte 0x42; .byte 0x0d
    .byte 0x8a; .byte 0x46; .byte 0x0e
    .byte 0x88; .byte 0x42; .byte 0x0e
    .byte 0x8a; .byte 0x46; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x0f
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8b; .byte 0x46; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x06
    .byte 0x89; .byte 0x42; .byte 0x06
    .byte 0x8b; .byte 0x46; .byte 0x0a
    .byte 0x89; .byte 0x42; .byte 0x0a
    .byte 0x8a; .byte 0x46; .byte 0x0e
    .byte 0x88; .byte 0x42; .byte 0x0e
    .byte 0x8a; .byte 0x46; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x0f
    .byte 0x8a; .byte 0x46; .byte 0x10
    .byte 0x88; .byte 0x42; .byte 0x10
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x8a; .byte 0x46; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x0f
    .byte 0x8a; .byte 0x46; .byte 0x10
    .byte 0x88; .byte 0x42; .byte 0x10
    .byte 0x8a; .byte 0x46; .byte 0x11
    .byte 0x88; .byte 0x42; .byte 0x11
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L000fe094-.-4
.L0010095a:
    sub ecx, 0
    shr ecx, 2
    je .L001009b2
.L00100962:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001009b2
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001009b2
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001009b2
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001009b2
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001009b2
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001009b2
    jmp .L00100962
.L001009b2:
    jmp .L000fe094
.L001009b7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00100a1f
.L001009c3:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100a1f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100a1f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100a1f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100a1f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100a1f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100a1f
    jmp .L001009c3
.L00100a1f:
    add esi, 1
    add edx, 1
    jmp .L000fe094
.L00100a2a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00100a98
.L00100a3c:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100a98
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100a98
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100a98
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100a98
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100a98
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100a98
    jmp .L00100a3c
.L00100a98:
    add esi, 2
    add edx, 2
    jmp .L000fe094
.L00100aa3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00100b17
.L00100abb:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100b17
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100b17
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100b17
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100b17
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100b17
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100b17
    jmp .L00100abb
.L00100b17:
    add esi, 3
    add edx, 3
    jmp .L000fe094
.L00100b22:
    sub ecx, 0
    shr ecx, 2
    je .L00100b7a
.L00100b2a:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100b7a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100b7a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100b7a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100b7a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100b7a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100b7a
    jmp .L00100b2a
.L00100b7a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    add esi, 1
    add edx, 1
    jmp .L000fe094
.L00100b89:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00100bf1
.L00100b95:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100bf1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100bf1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100bf1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100bf1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100bf1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100bf1
    jmp .L00100b95
.L00100bf1:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    add esi, 2
    add edx, 2
    jmp .L000fe094
.L00100c02:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00100c70
.L00100c14:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100c70
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100c70
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100c70
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100c70
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100c70
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100c70
    jmp .L00100c14
.L00100c70:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    add esi, 3
    add edx, 3
    jmp .L000fe094
.L00100c81:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00100cf5
.L00100c99:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100cf5
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100cf5
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100cf5
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100cf5
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100cf5
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100cf5
    jmp .L00100c99
.L00100cf5:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    add esi, 4
    add edx, 4
    jmp .L000fe094
.L00100d06:
    sub ecx, 0
    shr ecx, 2
    je .L00100d5e
.L00100d0e:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100d5e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100d5e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100d5e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100d5e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100d5e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100d5e
    jmp .L00100d0e
.L00100d5e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    add esi, 2
    add edx, 2
    jmp .L000fe094
.L00100d73:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00100ddb
.L00100d7f:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100ddb
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100ddb
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100ddb
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100ddb
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100ddb
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100ddb
    jmp .L00100d7f
.L00100ddb:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    add esi, 3
    add edx, 3
    jmp .L000fe094
.L00100df2:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00100e60
.L00100e04:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100e60
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100e60
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100e60
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100e60
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100e60
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00100e60
    jmp .L00100e04
.L00100e60:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    add esi, 4
    add edx, 4
    jmp .L000fe094
.L00100e77:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00100eeb
.L00100e8f:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100eeb
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100eeb
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100eeb
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100eeb
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100eeb
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00100eeb
    jmp .L00100e8f
.L00100eeb:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    add esi, 5
    add edx, 5
    jmp .L000fe094
.L00100f02:
    sub ecx, 0
    shr ecx, 2
    je .L00100f5a
.L00100f0a:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100f5a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100f5a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100f5a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100f5a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100f5a
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00100f5a
    jmp .L00100f0a
.L00100f5a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    add esi, 3
    add edx, 3
    jmp .L000fe094
.L00100f75:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00100fdd
.L00100f81:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100fdd
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100fdd
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100fdd
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100fdd
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100fdd
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00100fdd
    jmp .L00100f81
.L00100fdd:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    add esi, 4
    add edx, 4
    jmp .L000fe094
.L00100ffa:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00101068
.L0010100c:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101068
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101068
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101068
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101068
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101068
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101068
    jmp .L0010100c
.L00101068:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    add esi, 5
    add edx, 5
    jmp .L000fe094
.L00101085:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L001010f9
.L0010109d:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001010f9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001010f9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001010f9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001010f9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001010f9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001010f9
    jmp .L0010109d
.L001010f9:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    add esi, 6
    add edx, 6
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x83; .byte 0xc2; .byte 0x00
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x83; .byte 0xc2; .byte 0x01
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x83; .byte 0xc2; .byte 0x02
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x83; .byte 0xc2; .byte 0x03
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x83; .byte 0xc2; .byte 0x01
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x83; .byte 0xc2; .byte 0x02
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x83; .byte 0xc2; .byte 0x03
    .byte 0xe9; .long .L000fe094-.-4
.L00101181:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x83; .byte 0xc2; .byte 0x02
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x83; .byte 0xc2; .byte 0x03
    .byte 0xe9; .long .L000fe094-.-4
.L001011ba:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L001011d0:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x83; .byte 0xc2; .byte 0x03
    .byte 0xe9; .long .L000fe094-.-4
.L001011fc:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00101212:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L0010122b:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    mov byte ptr [edx + 5], al
    add edx, 6
    jmp .L000fe094
.L00101247:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    add edx, 4
    jmp .L000fe094
.L0010125d:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    add edx, 5
    jmp .L000fe094
.L00101276:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    add edx, 6
    jmp .L000fe094
.L00101292:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    add edx, 7
    jmp .L000fe094
.L001012b1:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L001012ca:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov byte ptr [edx + 5], al
    add edx, 6
    jmp .L000fe094
.L001012e6:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov byte ptr [edx + 6], al
    add edx, 7
    jmp .L000fe094
.L00101305:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov byte ptr [edx + 7], al
    add edx, 8
    jmp .L000fe094
.L00101327:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov byte ptr [edx + 4], al
    mov byte ptr [edx + 5], al
    add edx, 6
    jmp .L000fe094
.L00101343:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov byte ptr [edx + 5], al
    mov byte ptr [edx + 6], al
    add edx, 7
    jmp .L000fe094
.L00101362:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov byte ptr [edx + 6], al
    mov byte ptr [edx + 7], al
    add edx, 8
    jmp .L000fe094
.L00101384:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov byte ptr [edx + 7], al
    mov byte ptr [edx + 8], al
    add edx, 9
    jmp .L000fe094
.L001013a9:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov byte ptr [edx + 4], al
    mov byte ptr [edx + 5], al
    mov byte ptr [edx + 6], al
    add edx, 7
    jmp .L000fe094
.L001013c8:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov byte ptr [edx + 5], al
    mov byte ptr [edx + 6], al
    mov byte ptr [edx + 7], al
    add edx, 8
    jmp .L000fe094
.L001013ea:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov byte ptr [edx + 6], al
    mov byte ptr [edx + 7], al
    mov byte ptr [edx + 8], al
    add edx, 9
    jmp .L000fe094
.L0010140f:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov byte ptr [edx + 7], al
    mov byte ptr [edx + 8], al
    mov byte ptr [edx + 9], al
    add edx, 0xa
    jmp .L000fe094
.L00101437:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    add edx, 8
    jmp .L000fe094
.L00101450:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    add edx, 9
    jmp .L000fe094
.L0010146c:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    add edx, 0xa
    jmp .L000fe094
.L0010148b:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    add edx, 0xb
    jmp .L000fe094
.L001014ad:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov byte ptr [edx + 8], al
    add edx, 9
    jmp .L000fe094
.L001014c9:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov byte ptr [edx + 9], al
    add edx, 0xa
    jmp .L000fe094
.L001014e8:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov byte ptr [edx + 0xa], al
    add edx, 0xb
    jmp .L000fe094
.L0010150a:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov byte ptr [edx + 0xb], al
    add edx, 0xc
    jmp .L000fe094
.L0010152f:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov byte ptr [edx + 8], al
    mov byte ptr [edx + 9], al
    add edx, 0xa
    jmp .L000fe094
.L0010154e:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov byte ptr [edx + 9], al
    mov byte ptr [edx + 0xa], al
    add edx, 0xb
    jmp .L000fe094
.L00101570:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov byte ptr [edx + 0xa], al
    mov byte ptr [edx + 0xb], al
    add edx, 0xc
    jmp .L000fe094
.L00101595:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov byte ptr [edx + 0xb], al
    mov byte ptr [edx + 0xc], al
    add edx, 0xd
    jmp .L000fe094
.L001015bd:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov byte ptr [edx + 8], al
    mov byte ptr [edx + 9], al
    mov byte ptr [edx + 0xa], al
    add edx, 0xb
    jmp .L000fe094
.L001015df:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov byte ptr [edx + 9], al
    mov byte ptr [edx + 0xa], al
    mov byte ptr [edx + 0xb], al
    add edx, 0xc
    jmp .L000fe094
.L00101604:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov byte ptr [edx + 0xa], al
    mov byte ptr [edx + 0xb], al
    mov byte ptr [edx + 0xc], al
    add edx, 0xd
    jmp .L000fe094
.L0010162c:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov byte ptr [edx + 0xb], al
    mov byte ptr [edx + 0xc], al
    mov byte ptr [edx + 0xd], al
    add edx, 0xe
    jmp .L000fe094
.L00101657:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    add edx, 0xc
    jmp .L000fe094
.L00101673:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    add edx, 0xd
    jmp .L000fe094
.L00101692:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov dword ptr [edx + 0xa], eax
    add edx, 0xe
    jmp .L000fe094
.L001016b4:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov dword ptr [edx + 0xb], eax
    add edx, 0xf
    jmp .L000fe094
.L001016d9:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov byte ptr [edx + 0xc], al
    add edx, 0xd
    jmp .L000fe094
.L001016f8:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    mov byte ptr [edx + 0xd], al
    add edx, 0xe
    jmp .L000fe094
.L0010171a:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov dword ptr [edx + 0xa], eax
    mov byte ptr [edx + 0xe], al
    add edx, 0xf
    jmp .L000fe094
.L0010173f:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov dword ptr [edx + 0xb], eax
    mov byte ptr [edx + 0xf], al
    add edx, 0x10
    jmp .L000fe094
.L00101767:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov byte ptr [edx + 0xc], al
    mov byte ptr [edx + 0xd], al
    add edx, 0xe
    jmp .L000fe094
.L00101789:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    mov byte ptr [edx + 0xd], al
    mov byte ptr [edx + 0xe], al
    add edx, 0xf
    jmp .L000fe094
.L001017ae:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov dword ptr [edx + 0xa], eax
    mov byte ptr [edx + 0xe], al
    mov byte ptr [edx + 0xf], al
    add edx, 0x10
    jmp .L000fe094
.L001017d6:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov dword ptr [edx + 0xb], eax
    mov byte ptr [edx + 0xf], al
    mov byte ptr [edx + 0x10], al
    add edx, 0x11
    jmp .L000fe094
.L00101801:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov byte ptr [edx + 0xc], al
    mov byte ptr [edx + 0xd], al
    mov byte ptr [edx + 0xe], al
    add edx, 0xf
    jmp .L000fe094
.L00101826:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    mov byte ptr [edx + 0xd], al
    mov byte ptr [edx + 0xe], al
    mov byte ptr [edx + 0xf], al
    add edx, 0x10
    jmp .L000fe094
.L0010184e:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov dword ptr [edx + 0xa], eax
    mov byte ptr [edx + 0xe], al
    mov byte ptr [edx + 0xf], al
    mov byte ptr [edx + 0x10], al
    add edx, 0x11
    jmp .L000fe094
.L00101879:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov dword ptr [edx + 0xb], eax
    mov byte ptr [edx + 0xf], al
    mov byte ptr [edx + 0x10], al
    mov byte ptr [edx + 0x11], al
    add edx, 0x12
    jmp .L000fe094
.L001018a7:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov dword ptr [edx + 0xc], eax
    add edx, 0x10
    jmp .L000fe094
.L001018c6:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    mov dword ptr [edx + 0xd], eax
    add edx, 0x11
    jmp .L000fe094
.L001018e8:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov dword ptr [edx + 0xa], eax
    mov dword ptr [edx + 0xe], eax
    add edx, 0x12
    jmp .L000fe094
.L0010190d:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov dword ptr [edx + 3], eax
    mov dword ptr [edx + 7], eax
    mov dword ptr [edx + 0xb], eax
    mov dword ptr [edx + 0xf], eax
    add edx, 0x13
    jmp .L000fe094
.L00101935:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov dword ptr [edx + 0xc], eax
    mov byte ptr [edx + 0x10], al
    add edx, 0x11
    jmp .L000fe094
.L00101957:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    mov dword ptr [edx + 0xd], eax
    mov byte ptr [edx + 0x11], al
    add edx, 0x12
    jmp .L000fe094
.L0010197c:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov dword ptr [edx + 2], eax
    mov dword ptr [edx + 6], eax
    mov dword ptr [edx + 0xa], eax
    mov dword ptr [edx + 0xe], eax
    mov byte ptr [edx + 0x12], al
    add edx, 0x13
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x13
    .byte 0x83; .byte 0xc2; .byte 0x14
    .byte 0xe9; .long .L000fe094-.-4
.L001019cf:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov dword ptr [edx + 0xc], eax
    mov byte ptr [edx + 0x10], al
    mov byte ptr [edx + 0x11], al
    add edx, 0x12
    jmp .L000fe094
.L001019f4:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    mov dword ptr [edx + 1], eax
    mov dword ptr [edx + 5], eax
    mov dword ptr [edx + 9], eax
    mov dword ptr [edx + 0xd], eax
    mov byte ptr [edx + 0x11], al
    mov byte ptr [edx + 0x12], al
    add edx, 0x13
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x89; .byte 0x42; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x06
    .byte 0x89; .byte 0x42; .byte 0x0a
    .byte 0x89; .byte 0x42; .byte 0x0e
    .byte 0x88; .byte 0x42; .byte 0x12
    .byte 0x88; .byte 0x42; .byte 0x13
    .byte 0x83; .byte 0xc2; .byte 0x14
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x13
    .byte 0x88; .byte 0x42; .byte 0x14
    .byte 0x83; .byte 0xc2; .byte 0x15
    .byte 0xe9; .long .L000fe094-.-4
.L00101a75:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    mov dword ptr [edx], eax
    mov dword ptr [edx + 4], eax
    mov dword ptr [edx + 8], eax
    mov dword ptr [edx + 0xc], eax
    mov byte ptr [edx + 0x10], al
    mov byte ptr [edx + 0x11], al
    mov byte ptr [edx + 0x12], al
    add edx, 0x13
    jmp .L000fe094
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x01
    .byte 0x89; .byte 0x42; .byte 0x05
    .byte 0x89; .byte 0x42; .byte 0x09
    .byte 0x89; .byte 0x42; .byte 0x0d
    .byte 0x88; .byte 0x42; .byte 0x11
    .byte 0x88; .byte 0x42; .byte 0x12
    .byte 0x88; .byte 0x42; .byte 0x13
    .byte 0x83; .byte 0xc2; .byte 0x14
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x89; .byte 0x42; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x06
    .byte 0x89; .byte 0x42; .byte 0x0a
    .byte 0x89; .byte 0x42; .byte 0x0e
    .byte 0x88; .byte 0x42; .byte 0x12
    .byte 0x88; .byte 0x42; .byte 0x13
    .byte 0x88; .byte 0x42; .byte 0x14
    .byte 0x83; .byte 0xc2; .byte 0x15
    .byte 0xe9; .long .L000fe094-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x8a; .byte 0x06
    .byte 0x8a; .byte 0x26
    .byte 0x46
    .byte 0x88; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0f
    .byte 0x88; .byte 0x42; .byte 0x13
    .byte 0x88; .byte 0x42; .byte 0x14
    .byte 0x88; .byte 0x42; .byte 0x15
    .byte 0x83; .byte 0xc2; .byte 0x16
    .byte 0xe9; .long .L000fe094-.-4
.L00101b27:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 4
    shr ecx, 2
    je .L00101b70
.L00101b3e:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101b70
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101b70
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101b70
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101b70
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101b70
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101b70
    jmp .L00101b3e
.L00101b70:
    jmp .L000fe094
.L00101b75:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x44
    shr ecx, 2
    je .L00101bbe
.L00101b8c:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101bbe
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101bbe
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101bbe
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101bbe
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101bbe
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101bbe
    jmp .L00101b8c
.L00101bbe:
    jmp .L000fe094
.L00101bc3:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0
    shr ecx, 2
    je .L00101c09
.L00101bd7:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101c09
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101c09
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101c09
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101c09
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101c09
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101c09
    jmp .L00101bd7
.L00101c09:
    jmp .L000fe094
.L00101c0e:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 3
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00101c5f
.L00101c27:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101c5f
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101c5f
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101c5f
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101c5f
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101c5f
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101c5f
    jmp .L00101c27
.L00101c5f:
    add edx, 1
    jmp .L000fe094
.L00101c67:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x43
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00101cb8
.L00101c80:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101cb8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101cb8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101cb8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101cb8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101cb8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101cb8
    jmp .L00101c80
.L00101cb8:
    add edx, 1
    jmp .L000fe094
.L00101cc0:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -1
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00101d0e
.L00101cd6:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101d0e
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101d0e
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101d0e
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101d0e
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101d0e
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00101d0e
    jmp .L00101cd6
.L00101d0e:
    add edx, 1
    jmp .L000fe094
.L00101d16:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00101d6a
.L00101d32:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101d6a
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101d6a
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101d6a
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101d6a
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101d6a
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101d6a
    jmp .L00101d32
.L00101d6a:
    add edx, 2
    jmp .L000fe094
.L00101d72:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x42
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00101dc6
.L00101d8e:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101dc6
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101dc6
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101dc6
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101dc6
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101dc6
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101dc6
    jmp .L00101d8e
.L00101dc6:
    add edx, 2
    jmp .L000fe094
.L00101dce:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00101e1f
.L00101de7:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101e1f
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101e1f
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101e1f
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101e1f
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101e1f
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00101e1f
    jmp .L00101de7
.L00101e1f:
    add edx, 2
    jmp .L000fe094
.L00101e27:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 1
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00101e7e
.L00101e46:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101e7e
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101e7e
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101e7e
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101e7e
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101e7e
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101e7e
    jmp .L00101e46
.L00101e7e:
    add edx, 3
    jmp .L000fe094
.L00101e86:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x41
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00101edd
.L00101ea5:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101edd
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101edd
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101edd
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101edd
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101edd
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101edd
    jmp .L00101ea5
.L00101edd:
    add edx, 3
    jmp .L000fe094
.L00101ee5:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -3
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00101f39
.L00101f01:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101f39
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101f39
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101f39
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101f39
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101f39
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00101f39
    jmp .L00101f01
.L00101f39:
    add edx, 3
    jmp .L000fe094
.L00101f41:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 4
    shr ecx, 2
    je .L00101f8a
.L00101f58:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101f8a
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101f8a
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101f8a
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101f8a
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101f8a
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101f8a
    jmp .L00101f58
.L00101f8a:
    mov byte ptr [edx], al
    add edx, 1
    jmp .L000fe094
.L00101f94:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x44
    shr ecx, 2
    je .L00101fdd
.L00101fab:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101fdd
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101fdd
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101fdd
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101fdd
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101fdd
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00101fdd
    jmp .L00101fab
.L00101fdd:
    mov byte ptr [edx], al
    add edx, 1
    jmp .L000fe094
.L00101fe7:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0
    shr ecx, 2
    je .L0010202d
.L00101ffb:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010202d
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010202d
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010202d
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010202d
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010202d
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010202d
    jmp .L00101ffb
.L0010202d:
    mov byte ptr [edx], al
    add edx, 1
    jmp .L000fe094
.L00102037:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 3
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00102088
.L00102050:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102088
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102088
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102088
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102088
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102088
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102088
    jmp .L00102050
.L00102088:
    mov byte ptr [edx + 1], al
    add edx, 2
    jmp .L000fe094
.L00102093:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x43
    mov byte ptr [edx], al
    shr ecx, 2
    je .L001020e4
.L001020ac:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001020e4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001020e4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001020e4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001020e4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001020e4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001020e4
    jmp .L001020ac
.L001020e4:
    mov byte ptr [edx + 1], al
    add edx, 2
    jmp .L000fe094
.L001020ef:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -1
    mov byte ptr [edx], al
    shr ecx, 2
    je .L0010213d
.L00102105:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010213d
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010213d
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010213d
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010213d
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010213d
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010213d
    jmp .L00102105
.L0010213d:
    mov byte ptr [edx + 1], al
    add edx, 2
    jmp .L000fe094
.L00102148:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L0010219c
.L00102164:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010219c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010219c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010219c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010219c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010219c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010219c
    jmp .L00102164
.L0010219c:
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L001021a7:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x42
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L001021fb
.L001021c3:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001021fb
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001021fb
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001021fb
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001021fb
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001021fb
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001021fb
    jmp .L001021c3
.L001021fb:
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L00102206:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00102257
.L0010221f:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102257
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102257
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102257
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102257
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102257
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102257
    jmp .L0010221f
.L00102257:
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L00102262:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 1
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L001022b9
.L00102281:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001022b9
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001022b9
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001022b9
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001022b9
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001022b9
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001022b9
    jmp .L00102281
.L001022b9:
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L001022c4:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x41
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L0010231b
.L001022e3:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010231b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010231b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010231b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010231b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010231b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010231b
    jmp .L001022e3
.L0010231b:
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00102326:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -3
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L0010237a
.L00102342:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010237a
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010237a
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010237a
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010237a
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010237a
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010237a
    jmp .L00102342
.L0010237a:
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00102385:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 4
    shr ecx, 2
    je .L001023ce
.L0010239c:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001023ce
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001023ce
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001023ce
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001023ce
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001023ce
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001023ce
    jmp .L0010239c
.L001023ce:
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    add edx, 2
    jmp .L000fe094
.L001023db:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x44
    shr ecx, 2
    je .L00102424
.L001023f2:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102424
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102424
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102424
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102424
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102424
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102424
    jmp .L001023f2
.L00102424:
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    add edx, 2
    jmp .L000fe094
.L00102431:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0
    shr ecx, 2
    je .L00102477
.L00102445:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102477
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102477
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102477
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102477
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102477
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102477
    jmp .L00102445
.L00102477:
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    add edx, 2
    jmp .L000fe094
.L00102484:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 3
    mov byte ptr [edx], al
    shr ecx, 2
    je .L001024d5
.L0010249d:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001024d5
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001024d5
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001024d5
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001024d5
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001024d5
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001024d5
    jmp .L0010249d
.L001024d5:
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L001024e3:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x43
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00102534
.L001024fc:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102534
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102534
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102534
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102534
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102534
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102534
    jmp .L001024fc
.L00102534:
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L00102542:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -1
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00102590
.L00102558:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102590
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102590
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102590
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102590
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102590
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102590
    jmp .L00102558
.L00102590:
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L0010259e:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L001025f2
.L001025ba:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001025f2
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001025f2
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001025f2
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001025f2
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001025f2
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001025f2
    jmp .L001025ba
.L001025f2:
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00102600:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x42
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00102654
.L0010261c:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102654
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102654
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102654
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102654
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102654
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102654
    jmp .L0010261c
.L00102654:
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00102662:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L001026b3
.L0010267b:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001026b3
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001026b3
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001026b3
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001026b3
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001026b3
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L001026b3
    jmp .L0010267b
.L001026b3:
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L001026c1:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 1
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00102718
.L001026e0:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102718
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102718
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102718
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102718
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102718
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102718
    jmp .L001026e0
.L00102718:
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L00102726:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x41
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L0010277d
.L00102745:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010277d
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010277d
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010277d
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010277d
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010277d
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L0010277d
    jmp .L00102745
.L0010277d:
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L0010278b:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -3
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L001027df
.L001027a7:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001027df
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001027df
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001027df
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001027df
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001027df
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001027df
    jmp .L001027a7
.L001027df:
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L001027ed:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 4
    shr ecx, 2
    je .L00102836
.L00102804:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102836
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102836
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102836
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102836
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102836
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00102836
    jmp .L00102804
.L00102836:
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L00102846:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x44
    shr ecx, 2
    je .L0010288f
.L0010285d:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010288f
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010288f
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010288f
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010288f
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010288f
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010288f
    jmp .L0010285d
.L0010288f:
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L0010289f:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0
    shr ecx, 2
    je .L001028e5
.L001028b3:
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001028e5
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001028e5
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001028e5
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001028e5
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001028e5
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L001028e5
    jmp .L001028b3
.L001028e5:
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    add edx, 3
    jmp .L000fe094
.L001028f5:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 3
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00102946
.L0010290e:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102946
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102946
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102946
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102946
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102946
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102946
    jmp .L0010290e
.L00102946:
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00102957:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x43
    mov byte ptr [edx], al
    shr ecx, 2
    je .L001029a8
.L00102970:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001029a8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001029a8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001029a8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001029a8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001029a8
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001029a8
    jmp .L00102970
.L001029a8:
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L001029b9:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -1
    mov byte ptr [edx], al
    shr ecx, 2
    je .L00102a07
.L001029cf:
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102a07
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102a07
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102a07
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102a07
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102a07
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00102a07
    jmp .L001029cf
.L00102a07:
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    add edx, 4
    jmp .L000fe094
.L00102a18:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00102a6c
.L00102a34:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102a6c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102a6c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102a6c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102a6c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102a6c
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102a6c
    jmp .L00102a34
.L00102a6c:
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L00102a7d:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x42
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00102ad1
.L00102a99:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102ad1
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102ad1
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102ad1
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102ad1
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102ad1
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102ad1
    jmp .L00102a99
.L00102ad1:
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L00102ae2:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -2
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    shr ecx, 2
    je .L00102b33
.L00102afb:
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102b33
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102b33
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102b33
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102b33
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102b33
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00102b33
    jmp .L00102afb
.L00102b33:
    mov byte ptr [edx + 2], al
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    add edx, 5
    jmp .L000fe094
.L00102b44:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 1
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00102b9b
.L00102b63:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102b9b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102b9b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102b9b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102b9b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102b9b
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102b9b
    jmp .L00102b63
.L00102b9b:
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    mov byte ptr [edx + 5], al
    add edx, 6
    jmp .L000fe094
.L00102bac:
    and ecx, 0x3f
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, 0x41
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00102c03
.L00102bcb:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c03
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c03
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c03
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c03
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c03
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c03
    jmp .L00102bcb
.L00102c03:
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    mov byte ptr [edx + 5], al
    add edx, 6
    jmp .L000fe094
.L00102c14:
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    shl eax, 0x10
    mov al, byte ptr [esi]
    mov ah, byte ptr [esi]
    inc esi
    add ecx, -3
    mov byte ptr [edx], al
    mov byte ptr [edx + 1], al
    mov byte ptr [edx + 2], al
    shr ecx, 2
    je .L00102c68
.L00102c30:
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c68
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c68
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c68
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c68
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c68
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00102c68
    jmp .L00102c30
.L00102c68:
    mov byte ptr [edx + 3], al
    mov byte ptr [edx + 4], al
    mov byte ptr [edx + 5], al
    add edx, 6
    jmp .L000fe094
.size "reqspr_draw_",.-"reqspr_draw_"
