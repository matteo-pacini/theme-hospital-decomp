#include "bflib.h"

struct TbDPMIRegs {
    u32 EDI;
    u32 ESI;
    u32 EBP;
    u32 reserved;
    u32 EBX;
    u32 EDX;
    u32 ECX;
    u32 EAX;
    u16 flags;
    u16 ES;
    u16 DS;
    u16 FS;
    u16 GS;
    u16 IP;
    u16 CS;
    u16 SP;
    u16 SS;
};

extern "C" {
extern u16 lbVesaBytesPerLine;
extern u16 lbVesaHRes;
extern u16 lbVesaVRes;
extern u8 *lbVesaData;
extern s16 lbVesaPage;
extern s16 lbVesaGran;

int strncmp(const char *a, const char *b, u32 n);

void vesa_set_window(u32 window, u32 position, u32 zero);
#pragma aux vesa_set_window = 0xb8 0x05 0x4f 0x00 0x00 0x66 0x55 0xcd 0x10 0x66 0x5d \
    parm [ebx] [edx] [ecx] modify [eax];

// FUNCTION: 0x000ce630
int LbVesaGetGran(s16 mode)
{
    TbDPMIRegs rmi;
    REGS in;
    REGS out;
    SREGS sregs;

    memset(&rmi, 0, sizeof(rmi));
    rmi.ES = (u32)lbVesaData >> 4;
    rmi.DS = (u32)lbVesaData >> 4;
    rmi.EDI = 0;
    rmi.EAX = 0x4f01;
    rmi.ECX = mode;
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    memset(&sregs, 0, sizeof(sregs));
    segread(&sregs);
    in.x.eax = 0x300;
    in.x.ebx = 0x10;
    in.x.edi = (u32)&rmi;
    int386x(0x31, &in, &out, &sregs);
    lbVesaGran = (s16)((lbVesaData[5] << 8) + lbVesaData[4]);
    lbVesaBytesPerLine = (s16)((lbVesaData[0x11] << 8) + lbVesaData[0x10]);
    lbVesaHRes = (s16)((lbVesaData[0x13] << 8) + lbVesaData[0x12]);
    lbVesaVRes = (s16)((lbVesaData[0x15] << 8) + lbVesaData[0x14]);
    return 1;
}

// FUNCTION: 0x000ce738
int LbVesaSetMode(s16 mode)
{
    REGS in;
    REGS out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.h.ah = 0x4f;
    in.h.al = 2;
    in.w.bx = mode;
    int386(0x10, &in, &out);
    LbVesaGetGran(mode);
    return 1;
}

// FUNCTION: 0x000ce794
int LbVesaSetPage(s16 page)
{
    if (lbVesaPage != page) {
        vesa_set_window(0, (page << 6) / lbVesaGran, 0);
        vesa_set_window(1, (page << 6) / lbVesaGran, 0);
        lbVesaPage = page;
    }
    return 1;
}

// FUNCTION: 0x000ce814
int LbVesaGetInfo(void)
{
    TbDPMIRegs rmi;
    REGS in;
    REGS out;
    SREGS sregs;

    memset(&rmi, 0, sizeof(rmi));
    rmi.ES = (u32)lbVesaData >> 4;
    rmi.DS = (u32)lbVesaData >> 4;
    rmi.EDI = 0;
    rmi.EAX = 0x4f00;
    rmi.EBX = 0x101;
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    memset(&sregs, 0, sizeof(sregs));
    segread(&sregs);
    in.x.eax = 0x300;
    in.x.ebx = 0x10;
    in.x.edi = (u32)&rmi;
    int386x(0x31, &in, &out, &sregs);
    if (strncmp((char *)lbVesaData, "VESA", 4) == 0)
        return 1;
    else
        return -1;
}

// FUNCTION: 0x000ce8d8
int LbVesaIsModeAvailable(s16 mode)
{
    u16 *p;
    u32 addr;

    if (LbVesaGetInfo() == 1) {
        p = (u16 *)(lbVesaData + 0xe);
        addr = *p;
        p++;
        addr += *p << 4;
        for (p = (u16 *)addr; *p != 0xffff; p++) {
            if (*p == (u16)mode)
                return 1;
        }
    }
    return 0;
}

}
