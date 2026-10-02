struct DWORDREGS {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int cflag;
};

union REGS {
    struct DWORDREGS x;
};

int _dosretax(int ax, int carry);

int DoDosCall(union REGS *in_regs, union REGS *out_regs);
#pragma aux DoDosCall = \
    "push ebp" \
    "push edx" \
    "mov eax,[edi]" \
    "mov ebx,4[edi]" \
    "mov ecx,8[edi]" \
    "mov edx,0ch[edi]" \
    "mov esi,10h[edi]" \
    "mov edi,14h[edi]" \
    "clc" \
    "int 21h" \
    "mov ebp,edi" \
    "pop edi" \
    "mov [edi],eax" \
    "mov 4[edi],ebx" \
    "mov 8[edi],ecx" \
    "mov 0ch[edi],edx" \
    "mov 10h[edi],esi" \
    "mov 14h[edi],ebp" \
    "sbb eax,eax" \
    "pop ebp" \
    parm [edi] [edx] \
    value [eax] \
    modify [ebx ecx edx esi edi];

// intdos: issue INT 21h with in_regs, results in out_regs (cflag set on carry, errno on error); returns eax.
// FUNCTION: 0xf4880
int intdos(union REGS *in_regs, union REGS *out_regs)
{
    int status;

    status = DoDosCall(in_regs, out_regs);
    out_regs->x.cflag = status;
    _dosretax(out_regs->x.eax, status);
    return out_regs->x.eax;
}
