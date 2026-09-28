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
}

struct TbSerialDev {
    u8 unknown0[0x896];
    u8 rx_buf[0x800];
    u16 base;
    u16 vector;
    u16 irq;
    u16 rx_head;
    u16 rx_tail;
    u8 unknown10a0[8];
    u8 port;
};

struct ComHandlerInfo {
    TbSerialDev *dev;
    u16 base;
    u16 vector;
    u16 irq;
    u16 flags;
    u8 unknownc[6];
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

    ~SerialHandler();
    long Init(long port);
    long DeInit(long port);
    void WriteChar(TbSerialDev *dev, u8 c);
    void EmptyOutputBuffer(long port);
    long SetBaud(long port, long baud);
    void backup_serial_int_vectors(TbSerialDev *dev);
    void setup_serial_int_vectors(TbSerialDev *dev);
    void init_com_port(TbSerialDev *dev);
    long Write(long port, u8 *buf, long len);
};

struct CommHandlerData {
    char response[0x50];
    long timeout;
    long dial_timeout;
    char init_str[0x50];
    char dial_str[0x50];
    char hangup_str[0x50];
    char reset_str[0x50];
    long port;
    long unknown19c;
    char last_cmd[0x50];
    u8 online;
    long (*init_func)(void);
    long (*dial_func)(void);
    long (*hangup_func)(void);
    long (*other_func)(void);
    long (*cur_func)(void);
};

#define CH(p) ((CommHandlerData *)(p))

extern SerialHandler SerialCommHandler;
extern ComHandlerInfo com_dev[];
extern ModemResponse modem_response[];

extern "C" {
void fn_000f5254(SerialHandler *self);
#pragma aux fn_000f5254 "W?$dt:SerialHandler$n()_"
}

// FUNCTION: 0xf5254
// NONMATCHING: the range is the com_dev table initialisation (SerialHandler constructor) followed by
// the tail of SerialHandler::Init
void fn_000f5254(SerialHandler *self)
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
        com_dev[i].old_pm_off = 0;
        com_dev[i].old_pm_sel = 0;
        com_dev[i].old_rm_seg = 0;
        com_dev[i].old_rm_off = 0;
        com_dev[i].tx_head = 0;
        com_dev[i].tx_tail = 0;
    }
}

// FUNCTION: 0xf5544
// NONMATCHING: range also holds the following function; register allocation differs
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

// FUNCTION: 0xf5780
// NONMATCHING: range also holds the following tx-buffer function
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

// FUNCTION: 0xf5864
// NONMATCHING: range also holds two following functions
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

// FUNCTION: 0xf5a3c
// NONMATCHING: range also holds four following functions
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

extern "C" {
extern u8 D_000f6028[];
extern u8 D_000f6054[];
extern u8 D_000f6080[];
extern u8 D_000f60ac[];
extern u8 D_000f6024[];
extern u8 D_000f6214[];
}

// FUNCTION: 0xf5d40
// NONMATCHING: handler addresses are far code labels the target names by containing function; no cs selector load
void SerialHandler::setup_serial_int_vectors(TbSerialDev *dev)
{
    REGS r;
    u8 *handler;

    switch (dev->port) {
    case 0:
        handler = D_000f6028;
        break;
    case 1:
        handler = D_000f6054;
        break;
    case 2:
        handler = D_000f6080;
        break;
    case 3:
        handler = D_000f60ac;
        break;
    }
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x205;
    r.x.ebx = dev->vector;
    r.x.ecx = 0;
    r.x.edx = (u32)handler;
    int386(0x31, &r, &r);
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x201;
    r.x.ebx = dev->vector;
    r.x.ecx = ((u32)dev >> 4) & 0xffff;
    r.x.edx = (u32)dev - ((((u32)dev >> 4) & 0xffff) << 4);
    int386(0x31, &r, &r);
    memset(&r, 0, sizeof(r));
    r.x.eax = 0x600;
    r.x.ebx = (u32)D_000f6024 >> 16;
    r.x.ecx = (u32)D_000f6024 & 0xffff;
    r.x.esi = 0;
    r.x.edi = D_000f6214 - D_000f6024;
    int386(0x31, &r, &r);
}

// FUNCTION: 0xf5e74
// NONMATCHING: range also holds the serial interrupt handlers; register allocation differs
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

long SerialHandler::Write(long port, u8 *buf, long len)
{
    TbSerialDev *dev;
    long i;
    long n = 0;

    if (port >= 4)
        return len ^ len;
    dev = com_dev[port].dev;
    for (i = 0; i < len; i++, buf++) {
        WriteChar(dev, *buf);
        n++;
    }
    return n;
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
// NONMATCHING: target falls through into SerialHandler::Init placed after it; ours emits the tail jump
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
// NONMATCHING: range also holds the following function
long CommHandler::ReadByte(void)
{
    TbSerialDev *dev;
    long c;

    if (CH(this)->port >= 4)
        return -1;
    dev = com_dev[CH(this)->port].dev;
    if (dev->rx_head == dev->rx_tail)
        return -1;
    c = dev->rx_buf[dev->rx_head];
    dev->rx_head++;
    if (dev->rx_head > 0x7ff)
        dev->rx_head = 0;
    return c;
}

// FUNCTION: 0xf64bc
// NONMATCHING: range also holds two following functions; loop register allocation differs
long CommHandler::WriteBytes(u8 *buf, long len)
{
    return SerialCommHandler.Write(CH(this)->port, buf, len);
}

// FUNCTION: 0xf65d0
long CommHandler::SetBaud(long baud)
{
    return SerialCommHandler.SetBaud(CH(this)->port, baud);
}

// FUNCTION: 0xf65e8
// NONMATCHING: port and buffer pointer registers swapped in the inlined write loop
void CommHandler::SendModemString(char *str)
{
    char buf[0x50];

    strcpy(buf, str);
    strcat(buf, D_00128709);
    SerialCommHandler.Write(CH(this)->port, (u8 *)buf, strlen(buf));
    strcpy(CH(this)->last_cmd, buf);
}

// FUNCTION: 0xf66bc
// NONMATCHING: port and buffer pointer registers swapped in the inlined write loop
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
// NONMATCHING: stack store scheduling differs in the inlined write loop
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
// NONMATCHING: register allocation differs in the inlined write loops
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
        outp(dev->base + 4, inp(dev->base + 4) & 0xfe);
    }
    delay(50);
    SetDtr();
    SerialCommHandler.Write(CH(this)->port, (u8 *)D_0012870b, 3);
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
        outp(dev->base + 4, inp(dev->base + 4) & 0xfe);
    }
    if (CH(this)->port < 4) {
        dev = com_dev[CH(this)->port].dev;
        outp(dev->base + 4, inp(dev->base + 4) & 0xfd);
    }
    CH(this)->cur_func = 0;
    return ret;
}

// FUNCTION: 0xf6fb0
// NONMATCHING: range also holds two following functions; SerialInitData layout guessed
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
// NONMATCHING: inlined byte read uses different registers; range also holds the following function
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

// FUNCTION: 0xf7304
void CommHandler::SetInitModemFunc(long (*fn)(void))
{
    CH(this)->init_func = fn;
}

// FUNCTION: 0xf730c
// NONMATCHING: range also holds two more modem callback setters
void CommHandler::SetDialModemFunc(long (*fn)(void))
{
    CH(this)->dial_func = fn;
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
