#include "net.h"

extern "C" {
unsigned inp(unsigned port);
unsigned outp(unsigned port, unsigned value);
#pragma intrinsic(inp, outp)
void _disable(void);
#pragma aux _disable = "cli";
void _enable(void);
#pragma aux _enable = "sti" "cld";
char *strcat(char *dst, const char *src);
#pragma intrinsic(strcat)
int stricmp(const char *a, const char *b);
int strnicmp(const char *a, const char *b, unsigned n);
void delay(unsigned ms);
unsigned short FP_SEG(const volatile void __far *p);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];
#define FP_OFF(p) ((unsigned)(p))
extern u8 D_00110068[];
extern u8 D_001100fe[];
extern char D_00128690[];
extern char D_001286c2[];
extern char D_001286db[];
extern char D_001286f5[];
extern char D_001286f9[];
extern char D_001286fe[];
extern char D_00128702[];
extern char D_00128709[];
extern char D_0012870b[];
extern char D_0012870f[];
extern char D_00128712[];
extern char D_0012872a[];
extern char D_00128745[];
extern char D_0012874c[];
extern char D_001287e2[];
extern char D_001287ea[];
extern char D_00128767[];
extern char D_00128783[];
extern char D_00128786[];
typedef struct __iobuf FILE;
FILE *fopen(const char *name, const char *mode);
int fgetc(FILE *f);
int fclose(FILE *f);
}

struct TbSerialDev {
    u8 unknown0[0x896];
    u8 rx_buf[0x800];
    u16 base;
    u16 vector;
    u16 irq;
    u16 rx_head;
    u16 rx_tail;
    u16 head;
    u16 tail;
    u8 unknown10a4[4];
    u8 port;
};

struct ComHandlerInfo {
    TbSerialDev *dev;
    u16 base;
    u16 vector;
    u16 irq;
    u16 flags;
    void (__interrupt __far *old_handler)(void);
    u32 old_pm_off;
    u16 old_pm_sel;
    u16 old_rm_seg;
    u16 old_rm_off;
    u8 tx_buf[0x1000];
    u32 tx_head;
    u32 tx_tail;
};

struct ModemResponse {
    char text[0x28];
    long code;
};

struct SerialHandler {
    u8 unknown0[0x4334];
    u16 dos_sel[4];

    SerialHandler();
    ~SerialHandler();
    long Init(long port);
    long DeInit(long port);
    long TerminateInputIO(long port);
    void *AllocDosMem(u32 size, u16 *sel);
    void FreeDosMem(u16 sel);
    long ReadByte(long port);
    void WriteChar(TbSerialDev *dev, u8 c);
    void WriteCharNoWait(TbSerialDev *dev, u8 c);
    void EmptyOutputBuffer(long port);
    long WriteBytes(long port, u8 *buf, long len);
    long WriteBytesNoWait(long port, u8 *buf, long len);
    long SetBaud(long port, long baud);
    void SetDtr(long port);
    void SetRts(long port);
    void ClearDtr(long port);
    void ClearRts(long port);
    void backup_serial_int_vectors(TbSerialDev *dev);
    void setup_serial_int_vectors(TbSerialDev *dev);
    void init_com_port(TbSerialDev *dev);
};


/* The static initialiser for this object (.fn_init) is emitted ahead of SerialHandler's constructor. */
// FUNCTION: 0xf51f0
SerialHandler SerialCommHandler;
extern ComHandlerInfo com_dev[];
extern ModemResponse modem_response[];

void handle_com(TbSerialDev *dev);

extern "C" {
}

void __interrupt __loadds com_handler0(void);
void __interrupt __loadds com_handler1(void);
void __interrupt __loadds com_handler2(void);
void __interrupt __loadds com_handler3(void);

// FUNCTION: 0xf5254
SerialHandler::SerialHandler()
{
    long i;

    for (i = 0; i < 4; i++) {
        com_dev[i].dev = 0;
        switch (i) {
        case 0:
            com_dev[i].base = 0x3f8;
            com_dev[i].vector = 0xc;
            com_dev[i].irq = 4;
            break;
        case 1:
            com_dev[i].base = 0x2f8;
            com_dev[i].vector = 0xb;
            com_dev[i].irq = 3;
            break;
        case 2:
            com_dev[i].base = 0x3e8;
            com_dev[i].vector = 0xc;
            com_dev[i].irq = 4;
            break;
        case 3:
            com_dev[i].base = 0x2e8;
            com_dev[i].vector = 0xb;
            com_dev[i].irq = 3;
            break;
        }
        com_dev[i].flags = 0;
        com_dev[i].old_handler = 0;
        com_dev[i].old_pm_off = 0;
        com_dev[i].old_pm_sel = 0;
        com_dev[i].old_rm_seg = 0;
        com_dev[i].old_rm_off = 0;
        com_dev[i].tx_tail = 0;
        com_dev[i].tx_head = 0;
    }
}

// FUNCTION: 0xf534c
SerialHandler::~SerialHandler()
{
}

// FUNCTION: 0xf6348
long SerialHandler::Init(long port)
{
    TbSerialDev *dev;
    u32 size;

    if (port >= 4)
        return 0;
    if (com_dev[port].flags & 1)
        dev = com_dev[port].dev;
    else
        dev = (TbSerialDev *)AllocDosMem(0x10c9, &dos_sel[port]);
    if (!dev)
        return 0;
    size = D_001100fe - D_00110068;
    memcpy(dev, D_00110068, size);
    *(u16 *)&dev->unknown0[2] = com_dev[port].base;
    fn_000c20c0(D_00128690, size, *(u16 *)&dev->unknown0[2], com_dev[port].irq, com_dev[port].vector);
    dev->port = port;
    dev->base = com_dev[dev->port].base;
    dev->vector = com_dev[dev->port].vector;
    dev->irq = com_dev[dev->port].irq;
    dev->head = 0;
    dev->tail = 0;
    dev->rx_head = 0;
    dev->rx_tail = 0;
    com_dev[dev->port].dev = dev;
    com_dev[dev->port].flags = 1;
    backup_serial_int_vectors(dev);
    setup_serial_int_vectors(dev);
    init_com_port(dev);
    *(u32 *)dev->unknown10a4 = 9600;
    return (long)dev;
}

// FUNCTION: 0xf5544
long SerialHandler::DeInit(long port)
{
    REGS r;
    REGS r2;
    TbSerialDev *dev;

    if (port >= 4)
        return -1;
    dev = com_dev[port].dev;
    outp(dev->base + 1, 0);
    outp(dev->base + 4, 0);
    outp(0x21, inp(0x21) | (1 << dev->irq));
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x201;
    r.x.ebx = dev->vector;
    r.x.ecx = com_dev[dev->port].old_rm_seg;
    r.x.edx = com_dev[dev->port].old_rm_off;
    int386(0x31, &r, &r);
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x205;
    r.x.ebx = dev->vector;
    r.x.ecx = com_dev[dev->port].old_pm_sel;
    r.x.edx = com_dev[dev->port].old_pm_off;
    int386(0x31, &r, &r);
    memset(&r, 0, sizeof(r));
    r.x.eax = 0xf3;
    r.x.edx = dev->port;
    int386(0x14, &r, &r);
    r2.x.eax = 0x101;
    r2.x.edx = dos_sel[port];
    int386(0x31, &r2, &r2);
    com_dev[port].dev = 0;
    com_dev[port].flags = 0;
    return 0;
}

// FUNCTION: 0xf5714
long SerialHandler::ReadByte(long port)
{
    TbSerialDev *dev;
    long c;

    if (port < 4) {
        dev = com_dev[port].dev;
        if (dev->rx_head != dev->rx_tail) {
            c = dev->rx_buf[dev->rx_head];
            dev->rx_head++;
            if (dev->rx_head > 0x7ff)
                dev->rx_head = 0;
            return c;
        }
    }
    return -1;
}

// FUNCTION: 0xf5780
void SerialHandler::WriteChar(TbSerialDev *dev, u8 c)
{
    u32 busy = 1;
    u32 lsr;

    _disable();
    do {
        lsr = inp(dev->base + 5);
        while (lsr & 1) {
            dev->rx_buf[dev->rx_tail] = inp(dev->base);
            dev->rx_tail++;
            if (dev->rx_tail > 0x7ff)
                dev->rx_tail = 0;
            lsr = inp(dev->base + 5);
        }
        if (lsr & 0x20)
            busy = 0;
    } while (busy);
    outp(dev->base, c);
    _enable();
}

// FUNCTION: 0xf5818
void SerialHandler::WriteCharNoWait(TbSerialDev *dev, u8 c)
{
    ComHandlerInfo *info = &com_dev[dev->port];

    info->tx_buf[info->tx_tail++] = c;
    if (info->tx_tail >= 0x1000)
        info->tx_tail = 0;
}

// FUNCTION: 0xf5864
// NONMATCHING: ours copies the line status to ecx and keeps the port in esi; target tests al directly
void SerialHandler::EmptyOutputBuffer(long port)
{
    TbSerialDev *dev;
    ComHandlerInfo *info;
    u32 lsr;
    u32 i;

    if (port >= 4)
        return;
    dev = com_dev[port].dev;
    _disable();
    lsr = inp(dev->base + 5);
    while (lsr & 1) {
        dev->rx_buf[dev->rx_tail] = inp(dev->base);
        dev->rx_tail++;
        if (dev->rx_tail > 0x7ff)
            dev->rx_tail = 0;
        lsr = inp(dev->base + 5);
    }
    if (lsr & 0x20) {
        info = &com_dev[port];
        if (info->tx_head != info->tx_tail) {
            i = info->tx_head++;
            outp(dev->base, info->tx_buf[i]);
            if (info->tx_head >= 0x1000)
                info->tx_head = 0;
        }
    }
    _enable();
}

// FUNCTION: 0xf5948
long SerialHandler::WriteBytes(long port, u8 *buf, long len)
{
    TbSerialDev *dev;
    u8 *p;
    long n;
    long i;

    if (port >= 4)
        return 0;
    dev = com_dev[port].dev;
    p = buf;
    for (i = 0, n = 0; i < len; i++, p++, n++)
        WriteChar(dev, *p);
    return n;
}

// FUNCTION: 0xf59a8
long SerialHandler::WriteBytesNoWait(long port, u8 *buf, long len)
{
    TbSerialDev *dev;
    u8 *p;
    long n;
    long i;

    if (port >= 4)
        return 0;
    dev = com_dev[port].dev;
    p = buf;
    for (i = 0, n = 0; i < len; i++, p++, n++)
        WriteCharNoWait(dev, *p);
    return n;
}

// FUNCTION: 0xf5a3c
long SerialHandler::SetBaud(long port, long baud)
{
    TbSerialDev *dev;
    long divisor;

    if (port >= 4)
        return -1;
    dev = com_dev[port].dev;
    if (baud < 300)
        baud = 300;
    else if (baud > 115200)
        baud = 115200;
    divisor = 115200 / baud;
    _disable();
    outp(dev->base + 3, 0x83);
    outp(dev->base + 1, divisor >> 8);
    outp(dev->base, divisor & 0xff);
    outp(dev->base + 3, 3);
    _enable();
    return 0;
}

// FUNCTION: 0xf5ad0
void SerialHandler::SetDtr(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) | 1);
    }
}

// FUNCTION: 0xf5b0c
void SerialHandler::SetRts(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) | 2);
    }
}

// FUNCTION: 0xf5b48
void SerialHandler::ClearDtr(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~1);
    }
}

// FUNCTION: 0xf5b84
void SerialHandler::ClearRts(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~2);
    }
}

// FUNCTION: 0xf5bc0
long SerialHandler::TerminateInputIO(long port)
{
    return 0;
}

// FUNCTION: 0xf5bc4
void *SerialHandler::AllocDosMem(u32 size, u16 *sel)
{
    REGS r;

    r.x.eax = 0x100;
    r.x.ebx = (size + 15) >> 4;
    int386(0x31, &r, &r);
    if (r.x.cflag)
        return 0;
    if (sel)
        *sel = r.x.edx;
    return (void *)((r.x.eax & 0xffff) << 4);
}

// FUNCTION: 0xf5c14
void SerialHandler::FreeDosMem(u16 sel)
{
    REGS r;

    r.x.eax = 0x101;
    r.x.edx = sel;
    int386(0x31, &r, &r);
}

// FUNCTION: 0xf5c3c
void SerialHandler::backup_serial_int_vectors(TbSerialDev *dev)
{
    REGS r;

    memset(&r, 0, sizeof(r));
    r.x.eax = 0x200;
    r.h.bl = dev->vector;
    int386(0x31, &r, &r);
    com_dev[dev->port].old_rm_seg = r.x.ecx;
    com_dev[dev->port].old_rm_off = r.x.edx;
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x204;
    r.h.bl = dev->vector;
    int386(0x31, &r, &r);
    com_dev[dev->port].old_pm_sel = r.x.ecx;
    com_dev[dev->port].old_pm_off = r.x.edx;
}

void lock_code_start(void);
void lock_code_end(void);

static inline void lock_region(REGS *r, u32 addr, u32 len)
{
    memset(r, 0, sizeof(*r));
    r->x.eax = 0x600;
    r->x.ebx = addr >> 16;
    r->x.ecx = addr & 0xffff;
    r->x.esi = 0;
    r->x.edi = len;
    int386(0x31, r, r);
}

// FUNCTION: 0xf5d40
void SerialHandler::setup_serial_int_vectors(TbSerialDev *dev)
{
    REGS r;
    void (__interrupt __far *handler)(void);

    switch (dev->port) {
    case 0:
        handler = com_handler0;
        break;
    case 1:
        handler = com_handler1;
        break;
    case 2:
        handler = com_handler2;
        break;
    case 3:
        handler = com_handler3;
        break;
    }
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x205;
    r.x.ebx = dev->vector;
    r.x.ecx = FP_SEG(handler);
    r.x.edx = FP_OFF(handler);
    int386(0x31, &r, &r);
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x201;
    r.x.ebx = dev->vector;
    r.x.ecx = ((u32)dev >> 4) & 0xffff;
    r.x.edx = (u32)dev & 0xf;
    r.x.edx = (u32)dev - (r.x.ecx << 4);
    int386(0x31, &r, &r);
    lock_region(&r, (u32)lock_code_start, (u32)lock_code_end - (u32)lock_code_start);
}

// FUNCTION: 0xf5e74
void SerialHandler::init_com_port(TbSerialDev *dev)
{
    TbSerialDev *d;

    outp(0x21, inp(0x21) | (1 << dev->irq));
    outp(dev->base + 4, 0);
    outp(dev->base + 1, 0);
    outp(dev->base + 2, 0);
    outp(dev->base + 2, 0xc1);
    if ((inp(dev->base + 2) & 0xf8) == 0xc0) {
        fn_000c20c0(D_001286c2);
        outp(dev->base + 2, 0x41);
    } else {
        fn_000c20c0(D_001286db);
        outp(dev->base + 2, 0);
    }
    outp(dev->base + 1, 1);
    outp(dev->base + 4, 0xb);
    outp(0x21, inp(0x21) & ~(1 << dev->irq));
    inp(dev->base + 2);
    inp(dev->base);
    inp(dev->base + 5);
    inp(dev->base + 6);
    _enable();
    if (dev->port < 4) {
        d = com_dev[dev->port].dev;
        outp(d->base + 4, inp(d->base + 4) | 2);
    }
    if (dev->port < 4) {
        d = com_dev[dev->port].dev;
        outp(d->base + 4, inp(d->base + 4) | 1);
    }
    SetBaud(dev->port, 9600);
}

// FUNCTION: 0xf6024
void lock_code_start(void)
{
}

// FUNCTION: 0xf6028
void __interrupt __loadds com_handler0(void)
{
    handle_com(com_dev[0].dev);
    outp(0x20, 0x20);
}

// FUNCTION: 0xf6054
void __interrupt __loadds com_handler1(void)
{
    handle_com(com_dev[1].dev);
    outp(0x20, 0x20);
}

// FUNCTION: 0xf6080
void __interrupt __loadds com_handler2(void)
{
    handle_com(com_dev[2].dev);
    outp(0x20, 0x20);
}

// FUNCTION: 0xf60ac
void __interrupt __loadds com_handler3(void)
{
    handle_com(com_dev[3].dev);
    outp(0x20, 0x20);
}

// FUNCTION: 0xf60d8
void handle_com(TbSerialDev *dev)
{
    u32 iir;

    for (;;) {
        iir = inp(dev->base + 2) & 7;
        if (iir & 4) {
            u32 lsr = inp(dev->base + 5);
            while (lsr & 1) {
                u32 c = inp(dev->base);
                dev->rx_buf[dev->rx_tail] = c;
                dev->rx_tail++;
                if (dev->rx_tail > 0x7ff)
                    dev->rx_tail = 0;
                lsr = inp(dev->base + 5);
            }
        }
        if (!(iir & 6))
            break;
        inp(dev->base + 5);
    }
    outp(0x20, 0x20);
}

// FUNCTION: 0xf6184
void inc_head(TbSerialDev *dev)
{
    if (++dev->head > 0x7ff)
        dev->head = 0;
}

// FUNCTION: 0xf61a8
void inc_tail(TbSerialDev *dev)
{
    if (++dev->tail > 0x7ff)
        dev->tail = 0;
}

// FUNCTION: 0xf61cc
void inc_rec_head(TbSerialDev *dev)
{
    if (++dev->rx_head > 0x7ff)
        dev->rx_head = 0;
}

// FUNCTION: 0xf61f0
void inc_rec_tail(TbSerialDev *dev)
{
    if (++dev->rx_tail > 0x7ff)
        dev->rx_tail = 0;
}

// FUNCTION: 0xf6214
void lock_code_end(void)
{
}

// FUNCTION: 0xf6218
CommHandler::CommHandler()
{
    memset(this, 0, 0x50);
    CH(this)->online = 0;
    CH(this)->init_func = 0;
    CH(this)->dial_func = 0;
    CH(this)->other_func = 0;
    CH(this)->hangup_func = 0;
    CH(this)->unknown19c = 6000;
    CH(this)->timeout = 1000;
    CH(this)->dial_timeout = 4000;
    strcpy(CH(this)->init_str, D_001286f5);
    strcpy(CH(this)->dial_str, D_001286f9);
    strcpy(CH(this)->hangup_str, D_001286fe);
    strcpy(CH(this)->reset_str, D_00128702);
}

// FUNCTION: 0xf630c
CommHandler::~CommHandler()
{
    CH(this)->init_func = 0;
    CH(this)->dial_func = 0;
    CH(this)->other_func = 0;
    CH(this)->hangup_func = 0;
    CH(this)->online = 0;
}

// FUNCTION: 0xf633c
long CommHandler::Init(long port)
{
    CH(this)->port = port;
    return SerialCommHandler.Init(port);
}

// FUNCTION: 0xf636c
long CommHandler::DeInit(void)
{
    return SerialCommHandler.DeInit(CH(this)->port);
}

// FUNCTION: 0xf6380
long CommHandler::ReadByte(void)
{
    return SerialCommHandler.ReadByte(CH(this)->port);
}

// FUNCTION: 0xf63f4
u32 CommHandler::ReadBytes(void *buf, u32 len)
{
    u8 *p = (u8 *)buf;
    u32 n = 0;
    u32 more = 1;
    long c;

    do {
        if (n < len) {
            c = ReadByte();
            if (c == -1) {
                more = 0;
            } else {
                *p++ = c;
                n++;
            }
        } else {
            more = 0;
        }
    } while (more);
    return n;
}

// FUNCTION: 0xf64bc
long CommHandler::WriteBytes(u8 *buf, long len)
{
    return SerialCommHandler.WriteBytes(CH(this)->port, buf, len);
}

// FUNCTION: 0xf651c
long CommHandler::WriteBytesNoWait(u8 *buf, long len)
{
    return SerialCommHandler.WriteBytesNoWait(CH(this)->port, buf, len);
}

// FUNCTION: 0xf65bc
void CommHandler::EmptyOutputBuffer(void)
{
    SerialCommHandler.EmptyOutputBuffer(CH(this)->port);
}

// FUNCTION: 0xf65d0
long CommHandler::SetBaud(long baud)
{
    return SerialCommHandler.SetBaud(CH(this)->port, baud);
}

// FUNCTION: 0xf65e8
void CommHandler::SendModemString(char *str)
{
    char buf[0x50];

    strcpy(buf, str);
    strcat(buf, D_00128709);
    SerialCommHandler.WriteBytes(CH(this)->port, (u8 *)buf, strlen(buf));
    strcpy(CH(this)->last_cmd, buf);
}

// FUNCTION: 0xf66bc
long CommHandler::InitModem(void)
{
    long ret;

    CH(this)->cur_func = CH(this)->init_func;
    SendModemString(CH(this)->init_str);
    ret = GetModemResponse(CH(this)->timeout);
    CH(this)->cur_func = 0;
    return ret;
}

// FUNCTION: 0xf67c0
long CommHandler::DialModem(char *number)
{
    char buf[0x50];
    long ret;

    CH(this)->cur_func = CH(this)->dial_func;
    strcpy(buf, CH(this)->dial_str);
    strcat(buf, number);
    SendModemString(buf);
    ret = GetModemResponse(CH(this)->dial_timeout);
    if (ret == 2)
        CH(this)->online = 1;
    CH(this)->cur_func = 0;
    return ret;
}

// FUNCTION: 0xf6940
// NONMATCHING: register allocation differs around the inlined ClearDtr/WriteBytes (port, loop counter)
long CommHandler::HangUpModem(void)
{
    TbSerialDev *dev;
    long ret = -1;
    u32 t;

    CH(this)->cur_func = CH(this)->hangup_func;
    t = clock();
    while (clock() - t < 100)
        ;
    if (CH(this)->port < 4) {
        dev = com_dev[CH(this)->port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~1);
    }
    delay(50);
    SetDtr();
    SerialCommHandler.WriteBytes(CH(this)->port, (u8 *)D_0012870b, 3);
    t = clock();
    while (clock() - t < 100)
        ;
    strcpy(CH(this)->response, D_0012870f);
    GetModemResponse(CH(this)->timeout);
    fn_000c20c0(D_00128712, CH(this)->response);
    SendModemString(CH(this)->hangup_str);
    GetModemResponse(CH(this)->timeout);
    fn_000c20c0(D_0012872a, CH(this)->response);
    SendModemString(D_00128745);
    GetModemResponse(CH(this)->timeout);
    fn_000c20c0(D_0012874c, CH(this)->response);
    CH(this)->online = 0;
    if (CH(this)->port < 4) {
        dev = com_dev[CH(this)->port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~1);
    }
    if (CH(this)->port < 4) {
        dev = com_dev[CH(this)->port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~2);
    }
    CH(this)->cur_func = 0;
    return ret;
}

// FUNCTION: 0xf6d00
long CommHandler::AnswerModem(void)
{
    long ret;

    CH(this)->cur_func = CH(this)->other_func;
    SendModemString(CH(this)->reset_str);
    ret = 3;
    while (ret == 3 || ret == 1)
        ret = GetModemResponse(CH(this)->dial_timeout);
    if (ret == 2)
        CH(this)->online = 1;
    CH(this)->cur_func = 0;
    return ret;
}

inline void CommHandler::read_a_line(FILE *f, char *p)
{
    int c;

    while ((c = fgetc(f)) != '\r' && c != '\n')
        *p++ = c;
    *p = 0;
}

// FUNCTION: 0xf6e1c
long CommHandler::ReadConfigFile(char *filename)
{
    char buf[0x50];
    FILE *f;
    s16 i;

    fn_000c20c0(D_00128767, filename);
    if (filename) {
        f = fopen(filename, D_00128783);
        if (f) {
            fn_000c20c0(D_00128786, filename);
            for (i = 0; i < 4; i++) {
                read_a_line(f, buf);
                read_a_line(f, buf);
                read_a_line(f, buf);
            }
            read_a_line(f, buf);
            read_a_line(f, CH(this)->init_str);
            read_a_line(f, CH(this)->dial_str);
            read_a_line(f, CH(this)->hangup_str);
            read_a_line(f, CH(this)->reset_str);
            fclose(f);
            return 0;
        }
    }
    return -1;
}

// FUNCTION: 0xf6fb0
void CommHandler::ReadSerialDev(__4dn7ojSerialInitData *data)
{
    char **strs;

    if (!data)
        return;
    if (data->port < 4 && *(s32 *)&data->unknown8[0] != -1) {
        com_dev[data->port].irq = *(u16 *)&data->unknown8[0];
        com_dev[data->port].vector = *(u16 *)&data->unknown8[0] + 8;
    }
    if (*(u32 *)&data->unknown8[4] != 2)
        return;
    strs = (char **)&data->unknown8[8];
    if (strs[0])
        strcpy(CH(this)->init_str, strs[0]);
    else
        strs[0] = CH(this)->init_str;
    if (strs[1])
        strcpy(CH(this)->dial_str, strs[1]);
    else
        strs[1] = CH(this)->dial_str;
    if (strs[2])
        strcpy(CH(this)->hangup_str, strs[2]);
    else
        strs[2] = CH(this)->hangup_str;
    if (strs[3])
        strcpy(CH(this)->reset_str, strs[3]);
    else
        strs[3] = CH(this)->reset_str;
}

// FUNCTION: 0xf70f8
long CommHandler::GetModemResponse(long timeout)
{
    char buf[0xa0];
    u32 start;
    long ret = -1;
    long c;
    u16 len = 0;
    u8 done = 0;
    ModemResponse *m;
    TbSerialDev *dev;

    memset(buf, 0, sizeof(buf));
    start = clock();
    do {
        if (CH(this)->cur_func && CH(this)->cur_func() == -7)
            return -7;
        if (clock() - start > timeout)
            break;
        c = -1;
        if (CH(this)->port < 4) {
            dev = com_dev[CH(this)->port].dev;
            if (dev->rx_head != dev->rx_tail) {
                c = dev->rx_buf[dev->rx_head];
                dev->rx_head++;
                if (dev->rx_head > 0x7ff)
                    dev->rx_head = 0;
            }
        }
        if (c == -1)
            continue;
        if (c == '\r' || len == 0x50) {
            for (m = modem_response; m->code; m++) {
                if (!stricmp(buf, m->text)) {
                    done = 1;
                    ret = m->code;
                    strcpy(CH(this)->response, buf);
                    break;
                }
            }
            if (!done && !strnicmp(buf, D_001287ea, strlen(D_001287e2))) {
                strcpy(CH(this)->response, buf);
                ret = 2;
                break;
            }
            memset(buf, 0, sizeof(buf));
            len = 0;
        } else if (c != '\n') {
            buf[len++] = c;
        }
    } while (!done);
    return ret;
}

// FUNCTION: 0xf72e8
void CommHandler::SetModemResponseTimeOut(long seconds)
{
    CH(this)->unknown19c = seconds * 100;
}

// FUNCTION: 0xf7304
void CommHandler::SetInitModemFunc(long (*fn)(void))
{
    CH(this)->init_func = fn;
}

// FUNCTION: 0xf730c
void CommHandler::SetDialModemFunc(long (*fn)(void))
{
    CH(this)->dial_func = fn;
}

// FUNCTION: 0xf7314
void CommHandler::SetHangUpModemFunc(long (*fn)(void))
{
    CH(this)->hangup_func = fn;
}

// FUNCTION: 0xf731c
void CommHandler::SetAnswerModemFunc(long (*fn)(void))
{
    CH(this)->other_func = fn;
}

// FUNCTION: 0xf7324
void CommHandler::SetDtr(void)
{
    TbSerialDev *dev;

    if (CH(this)->port < 4) {
        dev = com_dev[CH(this)->port].dev;
        outp(dev->base + 4, inp(dev->base + 4) | 1);
    }
}

// FUNCTION: 0xf7368
void CommHandler::SetRts(void)
{
    SerialCommHandler.SetRts(CH(this)->port);
}

// FUNCTION: 0xf73ac
void CommHandler::ClearDtr(void)
{
    SerialCommHandler.ClearDtr(CH(this)->port);
}

// FUNCTION: 0xf73f0
void CommHandler::ClearRts(void)
{
    SerialCommHandler.ClearRts(CH(this)->port);
}

// FUNCTION: 0xf7434
long CommHandler::RunModemFunc(long (*fn)(void))
{
    if (fn)
        return fn();
    return 0;
}
