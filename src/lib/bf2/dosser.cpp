#include "net.h"

extern "C" {
unsigned inp(unsigned port);
unsigned outp(unsigned port, unsigned value);
#pragma intrinsic(inp, outp)
void _disable(void);
void _enable(void);
#pragma intrinsic(_disable, _enable)
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
/* Modem result strings, verbose and numeric, and what GetModemResponse returns for each. */
ModemResponse modem_response[] = {
    { "OK", 1 },
    { "CONNECT 300", 2 },
    { "CONNECT 600", 2 },
    { "CONNECT 0600", 2 },
    { "CONNECT 2400", 2 },
    { "CONNECT 4800", 2 },
    { "CONNECT 7200", 2 },
    { "CONNECT 9600", 2 },
    { "CONNECT 12000", 2 },
    { "CONNECT 14400", 2 },
    { "CONNECT 16800", 2 },
    { "CONNECT 19200", 2 },
    { "CONNECT 21600", 2 },
    { "CONNECT 24000", 2 },
    { "CONNECT 26400", 2 },
    { "CONNECT 28800", 2 },
    { "CONNECT 38400", 2 },
    { "CONNECT 57600", 2 },
    { "CONNECT 115200", 2 },
    { "CONNECT 75TX/1200RX", 2 },
    { "CONNECT 1200TX/75RX", 2 },
    { "CONNECT", 2 },
    { "RING", 3 },
    { "BUSY", 4 },
    { "ERROR", 5 },
    { "NO CARRIER", 6 },
    { "NO DIALTONE", 7 },
    { "NO DIAL TONE", 7 },
    { "0", 1 },
    { "1", 2 },
    { "2", 3 },
    { "3", 6 },
    { "4", 5 },
    { "5", 2 },
    { "6", 7 },
    { "7", 4 },
    { "10", 2 },
    { "11", 2 },
    { "12", 2 },
    { "", 0 },
};

void handle_com(TbSerialDev *dev);

extern "C" {
}

void __interrupt __loadds com_handler0(void);
void __interrupt __loadds com_handler1(void);
void __interrupt __loadds com_handler2(void);
void __interrupt __loadds com_handler3(void);

// Sets up the four COM ports' default addresses and IRQs (COM1/3 0x3f8/
// 0x3e8 on IRQ 4, COM2/4 0x2f8/0x2e8 on IRQ 3), all closed.
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

// Destroys the serial handler (nothing to do).
// NONMATCHING: the entry also covers SerialHandler::Init's body, where the size
// _D_001100fe - _D_00110068 is computed in other registers
// FUNCTION: 0xf534c
SerialHandler::~SerialHandler()
{
}

// Opens COM port `port` (0-3): copies the interrupt code and buffers into
// DOS memory, hooks its IRQ and sets the UART to 9600 baud, 8N1. Returns
// the port's device block, or 0.
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
    fn_000c20c0("SERIALHANDLER size %d Port %x IRQ %x INT VECT %x\n", size, *(u16 *)&dev->unknown0[2], com_dev[port].irq, com_dev[port].vector);
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

// Closes COM port `port`: disables its interrupts, restores the IRQ
// vectors and frees its DOS memory. Returns 0, or -1 for a bad port.
// FUNCTION: 0xf5544
long SerialHandler::DeInit(long port)
{
    TbSerialDev *dev;
    REGS r;

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
    FreeDosMem(dos_sel[port]);
    com_dev[port].dev = 0;
    com_dev[port].flags = 0;
    return 0;
}

// Returns the next received byte of COM port `port`, or -1 if none.
// FUNCTION: 0xf5714
long SerialHandler::ReadByte(long port)
{
    TbSerialDev *dev;
    long byte;

    if (port < 4) {
        dev = com_dev[port].dev;
        if (dev->rx_head != dev->rx_tail) {
            byte = dev->rx_buf[dev->rx_head];
            dev->rx_head++;
            if (dev->rx_head > 0x7ff)
                dev->rx_head = 0;
            return byte;
        }
    }
    return -1;
}

// Sends byte c on dev's port, waiting for the transmitter and keeping any
// bytes received meanwhile.
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

// Queues byte c in dev's port's transmit buffer (sent by
// EmptyOutputBuffer).
// FUNCTION: 0xf5818
void SerialHandler::WriteCharNoWait(TbSerialDev *dev, u8 c)
{
    ComHandlerInfo *info = &com_dev[dev->port];

    info->tx_buf[info->tx_tail++] = c;
    if (info->tx_tail >= 0x1000)
        info->tx_tail = 0;
}

// Keeps the bytes received on COM port `port` and, if the transmitter is
// free, sends one queued byte.
// FUNCTION: 0xf5864
void SerialHandler::EmptyOutputBuffer(long port)
{
    TbSerialDev *dev;
    ComHandlerInfo *info;
    u8 lsr;
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

// Sends len bytes from buf on COM port `port`, waiting for each. Returns
// the count sent (0 for a bad port).
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

// Queues len bytes from buf for COM port `port`. Returns the count queued.
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

// Sets COM port `port`'s speed to baud (300-115200), 8N1. Returns 0, or -1
// for a bad port.
// FUNCTION: 0xf5a3c
long SerialHandler::SetBaud(long port, long baud)
{
    TbSerialDev *dev;
    long divisor;

    if (port < 4) {
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
    return -1;
}

// Raises DTR on COM port `port`.
// FUNCTION: 0xf5ad0
void SerialHandler::SetDtr(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) | 1);
    }
}

// Raises RTS on COM port `port`.
// FUNCTION: 0xf5b0c
void SerialHandler::SetRts(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) | 2);
    }
}

// Drops DTR on COM port `port`.
// FUNCTION: 0xf5b48
void SerialHandler::ClearDtr(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~1);
    }
}

// Drops RTS on COM port `port`.
// FUNCTION: 0xf5b84
void SerialHandler::ClearRts(long port)
{
    TbSerialDev *dev;

    if (port < 4) {
        dev = com_dev[port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & ~2);
    }
}

// Does nothing. Returns 0.
// FUNCTION: 0xf5bc0
long SerialHandler::TerminateInputIO(long port)
{
    return 0;
}

// Allocates size bytes of DOS memory (DPMI), storing its selector in *sel.
// Returns its linear address, or 0.
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

// Frees the DOS memory with selector sel.
// FUNCTION: 0xf5c14
void SerialHandler::FreeDosMem(u16 sel)
{
    REGS r;

    r.x.eax = 0x101;
    r.x.edx = sel;
    int386(0x31, &r, &r);
}

// Saves dev's IRQ vector (real and protected mode) for restoring.
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

// Points dev's IRQ at the port's handler (protected mode) and at the copy
// in DOS memory (real mode), and locks the interrupt code.
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

// Programs dev's UART (FIFO if present, receive interrupts, DTR and RTS
// on) and unmasks its IRQ, at 9600 baud.
// FUNCTION: 0xf5e74
void SerialHandler::init_com_port(TbSerialDev *dev)
{
    TbSerialDev *port_dev;

    outp(0x21, inp(0x21) | (1 << dev->irq));
    outp(dev->base + 4, 0);
    outp(dev->base + 1, 0);
    outp(dev->base + 2, 0);
    outp(dev->base + 2, 0xc1);
    if ((inp(dev->base + 2) & 0xf8) == 0xc0) {
        fn_000c20c0("COM PORT HAS 16550 FIFO\n");
        outp(dev->base + 2, 0x41);
    } else {
        fn_000c20c0("COM PORT HAS A 8250 PORT\n");
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
        port_dev = com_dev[dev->port].dev;
        outp(port_dev->base + 4, inp(port_dev->base + 4) | 2);
    }
    if (dev->port < 4) {
        port_dev = com_dev[dev->port].dev;
        outp(port_dev->base + 4, inp(port_dev->base + 4) | 1);
    }
    SetBaud(dev->port, 9600);
}

// Marks the start of the code locked for the serial interrupts.
// FUNCTION: 0xf6024
void lock_code_start(void)
{
}

// COM1 interrupt handler.
// FUNCTION: 0xf6028
void __interrupt __loadds com_handler0(void)
{
    handle_com(com_dev[0].dev);
    outp(0x20, 0x20);
}

// COM2 interrupt handler.
// FUNCTION: 0xf6054
void __interrupt __loadds com_handler1(void)
{
    handle_com(com_dev[1].dev);
    outp(0x20, 0x20);
}

// COM3 interrupt handler.
// FUNCTION: 0xf6080
void __interrupt __loadds com_handler2(void)
{
    handle_com(com_dev[2].dev);
    outp(0x20, 0x20);
}

// COM4 interrupt handler.
// FUNCTION: 0xf60ac
void __interrupt __loadds com_handler3(void)
{
    handle_com(com_dev[3].dev);
    outp(0x20, 0x20);
}

// Moves the bytes received on dev's port into its 2K receive buffer.
// FUNCTION: 0xf60d8
void handle_com(TbSerialDev *dev)
{
    u32 iir;

    for (;;) {
        iir = inp(dev->base + 2) & 7;
        if (iir & 4) {
            u32 lsr = inp(dev->base + 5);
            while (lsr & 1) {
                u32 byte = inp(dev->base);
                dev->rx_buf[dev->rx_tail] = byte;
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

// Advances dev's transmit head (wrapping at 2K).
// FUNCTION: 0xf6184
void inc_head(TbSerialDev *dev)
{
    if (++dev->head > 0x7ff)
        dev->head = 0;
}

// Advances dev's transmit tail (wrapping at 2K).
// FUNCTION: 0xf61a8
void inc_tail(TbSerialDev *dev)
{
    if (++dev->tail > 0x7ff)
        dev->tail = 0;
}

// Advances dev's receive head (wrapping at 2K).
// FUNCTION: 0xf61cc
void inc_rec_head(TbSerialDev *dev)
{
    if (++dev->rx_head > 0x7ff)
        dev->rx_head = 0;
}

// Advances dev's receive tail (wrapping at 2K).
// FUNCTION: 0xf61f0
void inc_rec_tail(TbSerialDev *dev)
{
    if (++dev->rx_tail > 0x7ff)
        dev->rx_tail = 0;
}

// Marks the end of the code locked for the serial interrupts.
// FUNCTION: 0xf6214
void lock_code_end(void)
{
}

// Sets up a modem/serial connection with the default modem strings (ATZ,
// ATDT, ATH, ATS0=1) and timeouts, offline.
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
    strcpy(CH(this)->init_str, "ATZ");
    strcpy(CH(this)->dial_str, "ATDT");
    strcpy(CH(this)->hangup_str, "ATH");
    strcpy(CH(this)->reset_str, "ATS0=1");
}

// Clears the connection's callbacks and online flag.
// FUNCTION: 0xf630c
CommHandler::~CommHandler()
{
    CH(this)->init_func = 0;
    CH(this)->dial_func = 0;
    CH(this)->other_func = 0;
    CH(this)->hangup_func = 0;
    CH(this)->online = 0;
}

// Opens COM port `port` for this connection. Returns the port's device
// block, or 0.
// FUNCTION: 0xf633c
long CommHandler::Init(long port)
{
    CH(this)->port = port;
    return SerialCommHandler.Init(port);
}

// Closes this connection's COM port. Returns 0, or -1.
// FUNCTION: 0xf636c
long CommHandler::DeInit(void)
{
    return SerialCommHandler.DeInit(CH(this)->port);
}

// Returns the next received byte, or -1 if none.
// FUNCTION: 0xf6380
long CommHandler::ReadByte(void)
{
    return SerialCommHandler.ReadByte(CH(this)->port);
}

// Reads up to len received bytes into buf. Returns the count read.
// FUNCTION: 0xf63f4
u32 CommHandler::ReadBytes(void *buf, u32 len)
{
    u32 n = 0;
    u32 more = 1;
    u8 *p = (u8 *)buf;
    long byte;

    do {
        if (n >= len) {
            more = 0;
        } else {
            byte = ReadByte();
            if (byte == -1) {
                more = 0;
            } else {
                *p++ = byte;
                n++;
            }
        }
    } while (more);
    return n;
}

// Sends len bytes from buf, waiting for each. Returns the count sent.
// FUNCTION: 0xf64bc
long CommHandler::WriteBytes(u8 *buf, long len)
{
    return SerialCommHandler.WriteBytes(CH(this)->port, buf, len);
}

// Queues len bytes from buf for sending. Returns the count queued.
// FUNCTION: 0xf651c
long CommHandler::WriteBytesNoWait(u8 *buf, long len)
{
    return SerialCommHandler.WriteBytesNoWait(CH(this)->port, buf, len);
}

// Sends one queued byte if the port is free (see
// SerialHandler::EmptyOutputBuffer).
// FUNCTION: 0xf65bc
void CommHandler::EmptyOutputBuffer(void)
{
    SerialCommHandler.EmptyOutputBuffer(CH(this)->port);
}

// Sets the port's speed to baud. Returns 0, or -1.
// FUNCTION: 0xf65d0
long CommHandler::SetBaud(long baud)
{
    return SerialCommHandler.SetBaud(CH(this)->port, baud);
}

// Sends modem command str followed by a carriage return.
// FUNCTION: 0xf65e8
void CommHandler::SendModemString(char *str)
{
    char buf[0x50];

    strcpy(buf, str);
    strcat(buf, "\r");
    SerialCommHandler.WriteBytes(CH(this)->port, (u8 *)buf, strlen(buf));
    strcpy(CH(this)->last_cmd, buf);
}

// Sends the init string and returns the modem's response (see
// GetModemResponse).
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

// Dials number and returns the modem's response; 2 (CONNECT) puts the
// connection online.
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

// Hangs up: drops DTR, sends +++, the hang-up string and ATS0=0, then
// drops DTR and RTS. Returns -1.
// FUNCTION: 0xf6940
long CommHandler::HangUpModem(void)
{
    u32 start;
    long ret = -1;

    CH(this)->cur_func = CH(this)->hangup_func;
    start = clock();
    while (clock() - start < 100)
        ;
    SerialCommHandler.ClearDtr(CH(this)->port);
    delay(50);
    SerialCommHandler.SetDtr(CH(this)->port);
    SerialCommHandler.WriteBytes(CH(this)->port, (u8 *)"+++", 3);
    start = clock();
    while (clock() - start < 100)
        ;
    strcpy(CH(this)->response, "OK");
    GetModemResponse(CH(this)->timeout);
    fn_000c20c0("+++ RESPONSE CODE = %s\n", CH(this)->response);
    SendModemString(CH(this)->hangup_str);
    GetModemResponse(CH(this)->timeout);
    fn_000c20c0("HangUP RESPONSE CODE = %s\n", CH(this)->response);
    SendModemString("ATS0=0");
    GetModemResponse(CH(this)->timeout);
    fn_000c20c0("ATS0=0 RESPONSE CODE = %s\n", CH(this)->response);
    CH(this)->online = 0;
    ClearDtr();
    ClearRts();
    CH(this)->cur_func = 0;
    return ret;
}

// Sets the modem to answer (the auto-answer string) and waits through RING
// and OK for the call. Returns the modem's response; 2 (CONNECT) puts the
// connection online.
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

// Reads the modem strings from config file filename (lines 14-17). Returns
// 0, or -1 if it cannot be opened.
// FUNCTION: 0xf6e1c
long CommHandler::ReadConfigFile(char *filename)
{
    char buf[0x50];
    FILE *f;
    s16 i;

    fn_000c20c0("ATTEMPTING TO OPEN FILE %s\n", filename);
    if (filename) {
        f = fopen(filename, "rt");
        if (f) {
            fn_000c20c0("CommHandler::ReadConfigFile with file %s\n", filename);
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

// Applies data's IRQ to its port and, for a modem (mode 2), its modem
// strings (or fills in the defaults where it has none).
// FUNCTION: 0xf6fb0
void CommHandler::ReadSerialDev(__4dn7ojSerialInitData *data)
{
    if (data) {
        if (data->port < 4 && data->irq != -1) {
            com_dev[data->port].irq = data->irq;
            com_dev[data->port].vector = data->irq + 8;
        }
        if (data->mode == 2) {
            if (data->init_str)
                strcpy(CH(this)->init_str, data->init_str);
            else
                data->init_str = CH(this)->init_str;
            if (data->dial_str)
                strcpy(CH(this)->dial_str, data->dial_str);
            else
                data->dial_str = CH(this)->dial_str;
            if (data->hangup_str)
                strcpy(CH(this)->hangup_str, data->hangup_str);
            else
                data->hangup_str = CH(this)->hangup_str;
            if (data->reset_str)
                strcpy(CH(this)->reset_str, data->reset_str);
            else
                data->reset_str = CH(this)->reset_str;
        }
    } else {
        fn_000c20c0("CommHandler::ReadSerialDev NULL parameter passed\n");
    }
}

// Reads a line (up to a carriage return or line feed) from f into p.
// FUNCTION: 0xf70d8
void CommHandler::read_a_line(FILE *f, char *p)
{
    int c;

    while ((c = fgetc(f)) != '\r' && c != '\n')
        *p++ = c;
    *p = 0;
}

// Waits up to timeout clock ticks for a modem response line. Returns 1 OK,
// 2 CONNECT, 3 RING, 4 BUSY, 5 ERROR, 6 NO CARRIER, 7 NO DIALTONE, -1 on
// timeout, or -7 if the current callback asks to abort.
// FUNCTION: 0xf70f8
long CommHandler::GetModemResponse(long timeout)
{
    long byte;
    u8 done = 0;
    u32 start;
    long ret = -1;
    u16 len = 0;
    char buf[0xa0];
    ModemResponse *resp;

    memset(buf, 0, sizeof(buf));
    start = clock();
    do {
        if (RunModemFunc(CH(this)->cur_func) == -7)
            return -7;
        if (clock() - start > timeout)
            break;
        byte = ReadByte();
        if (byte == -1)
            continue;
        if (byte == '\r' || len == 0x50) {
            for (resp = modem_response;; resp++) {
                if (resp->code == 0)
                    break;
                if (!stricmp(buf, resp->text)) {
                    done = 1;
                    ret = resp->code;
                    strcpy(CH(this)->response, buf);
                    break;
                }
            }
            if (!done && !strnicmp(buf, "CONNECT", strlen("CONNECT"))) {
                strcpy(CH(this)->response, buf);
                ret = 2;
                break;
            }
            memset(buf, 0, sizeof(buf));
            len = 0;
        } else if (byte != '\n') {
            buf[len++] = byte;
        }
    } while (!done);
    return ret;
}

// Sets the modem response time-out in seconds.
// FUNCTION: 0xf72e8
void CommHandler::SetModemResponseTimeOut(long seconds)
{
    CH(this)->unknown19c = seconds * 100;
}

// Sets the callback run while waiting during InitModem.
// FUNCTION: 0xf7304
void CommHandler::SetInitModemFunc(long (*fn)(void))
{
    CH(this)->init_func = fn;
}

// Sets the callback run while waiting during DialModem.
// FUNCTION: 0xf730c
void CommHandler::SetDialModemFunc(long (*fn)(void))
{
    CH(this)->dial_func = fn;
}

// Sets the callback run while waiting during HangUpModem.
// FUNCTION: 0xf7314
void CommHandler::SetHangUpModemFunc(long (*fn)(void))
{
    CH(this)->hangup_func = fn;
}

// Sets the callback run while waiting during AnswerModem.
// FUNCTION: 0xf731c
void CommHandler::SetAnswerModemFunc(long (*fn)(void))
{
    CH(this)->other_func = fn;
}

// Raises DTR.
// FUNCTION: 0xf7324
void CommHandler::SetDtr(void)
{
    TbSerialDev *dev;

    if (CH(this)->port < 4) {
        dev = com_dev[CH(this)->port].dev;
        outp(dev->base + 4, inp(dev->base + 4) | 1);
    }
}

// Raises RTS.
// FUNCTION: 0xf7368
void CommHandler::SetRts(void)
{
    SerialCommHandler.SetRts(CH(this)->port);
}

// Drops DTR.
// FUNCTION: 0xf73ac
void CommHandler::ClearDtr(void)
{
    SerialCommHandler.ClearDtr(CH(this)->port);
}

// Drops RTS.
// FUNCTION: 0xf73f0
void CommHandler::ClearRts(void)
{
    SerialCommHandler.ClearRts(CH(this)->port);
}

// Runs callback fn, if any. Returns its result, or 0.
// FUNCTION: 0xf7434
long CommHandler::RunModemFunc(long (*fn)(void))
{
    if (fn)
        return fn();
    return 0;
}
