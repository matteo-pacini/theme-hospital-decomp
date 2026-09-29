#include "net.h"

u16 Swap16(u16 v)
{
    u16 r = v >> 8;

    r |= v << 8;
    return r;
}

// FUNCTION: 0xd3500
IPXHandler::IPXHandler()
{
    dosbuf = 0;
    send_packets = 0;
    vector = 0;
    old_seg = 0;
    old_off = 0;
    next_packet = 0;
    batch_count = 0;
    socket_open = 0;
    stamp = clock();
    memset(IPXDEBUGSTRING, 0, 0x50);
}

// FUNCTION: 0xd3558
IPXHandler::~IPXHandler()
{
    dosbuf = 0;
    send_packets = 0;
    old_seg = 0;
    old_off = 0;
    next_packet = 0;
    batch_count = 0;
    socket_open = 0;
    memset(IPXDEBUGSTRING, 0, 0x50);
}

// FUNCTION: 0xd35a0
// NONMATCHING: frame slots of the inlined DOS allocations' register blocks, and
// register choice in them
long IPXHandler::Init(void)
{
    u8 addr[10];
    u32 entry;
    RMREGS rm;

    dosbuf = fn_000d4150(this, 0x950, &dosbuf_sel);
    if (dosbuf) {
        memset(dosbuf, 0, 0x950);
        rm_ss = (s32)(dosbuf + 0x15f) >> 4;
        rm_sp = (u16)(u32)dosbuf + 0x950 - (rm_ss << 4);
        send_esr = (u32)dosbuf + 0x78 - ((((u32)dosbuf >> 4) & 0xffff) << 4);
        recv_esr = (u32)dosbuf + 0x80 - ((((u32)dosbuf >> 4) & 0xffff) << 4);
        esr_size = D_00110068 - D_00110000;
        memcpy(dosbuf, D_00110000, esr_size);
        memcpy(dosbuf + 0x78, ipxsendesr, 8);
        memcpy(dosbuf + 0x80, ipxrecvesr, 8);
        IPXDebug(D_00125e70, esr_size);
        send_packets = AllocIPXPackets(0x20, &send_sel);
        IPXDebug(D_00125e92, ((u32)send_packets >> 4) & 0xffff, (u32)send_packets & 0xf);
        if (send_packets == 0) {
            strcpy(IPXDEBUGSTRING, D_00125eae);
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            fn_000d41a0(this, dosbuf_sel);
            dosbuf_sel = 0;
            return 1;
        }
        addrbuf = fn_000d4150(this, 0xd0, &addr_sel);
        if (!addrbuf) {
            strcpy(IPXDEBUGSTRING, D_00125ed2);
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            fn_000d41a0(this, dosbuf_sel);
            dosbuf_sel = 0;
            fn_000d41a0(this, send_sel);
            send_sel = 0;
            return 1;
        }
        recv_packets = AllocIPXPackets(0x20, &recv_sel);
        if (recv_packets == 0) {
            strcpy(IPXDEBUGSTRING, D_00125ef0);
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            fn_000d41a0(this, dosbuf_sel);
            dosbuf_sel = 0;
            fn_000d41a0(this, send_sel);
            send_sel = 0;
            fn_000d41a0(this, addr_sel);
            addr_sel = 0;
            return 1;
        }
        memset(&rm, 0, sizeof(rm));
        rm.eax = 0x7a00;
        SimulateInt(0x2f, &rm);
        *(u16 *)(dosbuf + 0x8a) = rm.edi;
        *(u16 *)(dosbuf + 0x8c) = rm.es;
        if ((rm.eax & 0xff) != 0xff) {
            strcpy(IPXDEBUGSTRING, D_00125f12);
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            fn_000d41a0(this, dosbuf_sel);
            dosbuf_sel = 0;
            fn_000d41a0(this, send_sel);
            send_sel = 0;
            fn_000d41a0(this, addr_sel);
            addr_sel = 0;
            fn_000d41a0(this, recv_sel);
            recv_sel = 0;
            return 1;
        }
        entry = rm.edi | ((u32)rm.es << 16);
        memcpy(dosbuf + 2, &entry, 4);
        IPXDebug(D_00125f2c, dosbuf[2]);
        if (!SetupVector()) {
            strcpy(IPXDEBUGSTRING, D_00125f3c);
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            fn_000d41a0(this, dosbuf_sel);
            dosbuf_sel = 0;
            fn_000d41a0(this, send_sel);
            send_sel = 0;
            fn_000d41a0(this, addr_sel);
            addr_sel = 0;
            fn_000d41a0(this, recv_sel);
            recv_sel = 0;
            IPXDebug(D_00125f6a);
            return 1;
        }
        sprintf(IPXDEBUGSTRING, D_00125f72, *(u16 *)(dosbuf + 0x8c), *(u16 *)(dosbuf + 0x8a));
        IPXDebug(D_00125f9f, IPXDEBUGSTRING);
        GetAddress(addr);
        batch_count = 0;
        return 0;
    } else {
        strcpy(IPXDEBUGSTRING, D_00125fa2);
        return 1;
    }
}

// FUNCTION: 0xd3eb8
void IPXHandler::DeInit(void)
{
    DPMIREGS r;

    if (dosbuf) {
        if (socket_open) {
            __3ljac5NETSOCKET s;

            s.socket = socket;
            CloseSocket(&s);
            socket_open = 0;
        }
        if (vector) {
            memset(&r, 0, sizeof(r));
            r.x.eax = 0x201;
            r.x.ebx = vector;
            r.x.ecx = old_seg;
            r.x.edx = old_off;
            int386(0x31, (REGS *)&r, (REGS *)&r);
        }
        dosbuf = 0;
        fn_000d41a0(this, dosbuf_sel);
        dosbuf_sel = 0;
        fn_000d41a0(this, send_sel);
        send_sel = 0;
        fn_000d41a0(this, addr_sel);
        addr_sel = 0;
        fn_000d41a0(this, recv_sel);
        recv_sel = 0;
        IPXDebug(D_00125fe2);
        strcpy(IPXDEBUGSTRING, D_00125fef);
        stamp = clock();
        batch_count = 0;
    } else {
        strcpy(IPXDEBUGSTRING, D_00126003);
    }
}

// FUNCTION: 0xd422c
// NONMATCHING: register choice: the target copies used to ebx and builds the
// real-mode address in eax
long IPXHandler::SetupVector(void)
{
    DPMIREGS r;
    int vec;
    u32 used;

    for (vec = 0x60; vec < 0x66; vec++) {
        memset(&r, 0, sizeof(r));
        r.w.ax = 0x200;
        r.h.bl = vec;
        int386(0x31, (REGS *)&r, (REGS *)&r);
        if (r.w.cx || r.w.dx)
            used = 1;
        else
            used = 0;
        if (used) {
            if (*(s8 *)(((r.x.ecx & 0xffff) << 4) | (r.x.edx & 0xffff)) == 0xcf)
                used = 0;
        }
        if (!used) {
            IPXDebug(D_00126012, vec);
            vector = vec;
            old_seg = r.w.cx;
            old_off = r.w.dx;
            memset(&r, 0, sizeof(r));
            r.x.eax = 0x201;
            r.x.ebx = vector;
            r.x.ecx = ((u32)dosbuf >> 4) & 0xffff;
            r.x.edx = (u32)dosbuf - ((((u32)dosbuf >> 4) & 0xffff) << 4);
            int386(0x31, (REGS *)&r, (REGS *)&r);
            return 1;
        }
    }
    return 0;
}

extern "C" {

// FUNCTION: 0x000d4150
u8 *fn_000d4150(IPXHandler *self, u32 size, u16 *sel)
{
    DPMIREGS r;

    r.x.eax = 0x100;
    r.x.ebx = (size + 15) >> 4;
    int386(0x31, (REGS *)&r, (REGS *)&r);
    if (r.x.cflag)
        return 0;
    if (sel)
        *sel = r.x.edx;
    return (u8 *)((r.x.eax & 0xffff) << 4);
}

// FUNCTION: 0x000d41a0
void fn_000d41a0(IPXHandler *self, u16 sel)
{
    DPMIREGS r;

    r.x.eax = 0x101;
    r.x.edx = sel;
    int386(0x31, (REGS *)&r, (REGS *)&r);
}

}

extern "C" {

// FUNCTION: 0xd41c8
u32 fn_000d41c8(IPXHandler *self, u16 intno, RMREGS *rm)
{
    DPMIREGS r;
    SREGS s;

    memset(&r, 0, sizeof(r));
    r.x.eax = 0x300;
    r.x.ebx = intno;
    segread(&s);
    rm->ss = self->rm_ss;
    rm->sp = self->rm_sp;
    r.x.edi = (u32)rm;
    int386x(0x31, (REGS *)&r, (REGS *)&r, &s);
    return rm->eax;
}

}

// FUNCTION: 0xd4348
long IPXHandler::GetAddress(u8 *addr)
{
    RMREGS rm;

    memset(&rm, 0, sizeof(rm));
    rm.ebx = 9;
    rm.es = ((u32)addrbuf >> 4) & 0xffff;
    rm.esi = (u32)addrbuf & 0xf;
    SimulateInt(vector, &rm);
    memcpy(addr, addrbuf, 10);
    memcpy(net, addr, 4);
    memcpy(node, addr + 4, 6);
    IPXDebug(D_0012602c, net[0], net[1], net[2], net[3], node[0], node[1], node[2], node[3], node[4], node[5]);
    IPXDebug(D_0012605c, rm.eax);
    return rm.eax;
}

// FUNCTION: 0xd44a4
long IPXHandler::OpenSocket(__3ljac5NETSOCKET *sock, u16 mode)
{
    RMREGS rm;

    memset(&rm, 0, sizeof(rm));
    rm.eax = mode;
    rm.edx = Swap16(sock->socket);
    SimulateInt(vector, &rm);
    sock->socket = rm.edx;
    memcpy(sock->net, net, 10);
    IPXDebug(D_00126066, rm.edx, rm.eax);
    return rm.eax;
}

// FUNCTION: 0xd4584
void IPXHandler::CloseSocket(__3ljac5NETSOCKET *sock)
{
    RMREGS rm;

    IPXDebug(D_00125fc0, sock->socket);
    memset(&rm, 0, sizeof(rm));
    rm.ebx = 1;
    rm.edx = sock->socket;
    SimulateInt(vector, &rm);
    IPXDebug(D_00125fd3);
}

// FUNCTION: 0xd4634
// NONMATCHING: this and dest parameter spill slots swapped
void IPXHandler::Send(__3ljac5NETSOCKET *sock, __3ljac5NETSOCKET *dest, u8 *data, long len)
{
    IPXPacket *p;

    p = GetNextSendPkt();
    memset(p, 0, sizeof(IPXPacket));
    p->link = 0;
    p->esr_off = send_esr;
    p->esr_seg = ((u32)dosbuf >> 4) & 0xffff;
    p->socket = sock->socket;
    p->frag_count = 1;
    p->frag_off = (u16)(u32)&p->hdr & 0xf;
    p->in_use = 1;
    p->frag_seg = ((u32)&p->hdr >> 4) & 0xffff;
    p->completion = 0;
    p->hdr.length = 0x1e00;
    p->frag_size = len + 0x1e;
    p->hdr.type = 0;
    p->hdr.dest_socket = dest->socket;
    memcpy(p->immediate, dest->node, 6);
    memcpy(p->hdr.dest_node, dest->node, 6);
    memcpy(p->hdr.dest_net, dest->net, 4);
    memcpy(p->data, data, len);
    SendEcb((ECB *)p);
}

// FUNCTION: 0xd4800
void IPXHandler::Listen(IPXPacket *p, __3ljac5NETSOCKET *sock)
{
    __2aq9p8IPXPARAMS c;

    memset(p, 0, sizeof(IPXPacket));
    p->link = 0;
    p->esr_off = recv_esr;
    p->esr_seg = ((u32)dosbuf >> 4) & 0xffff;
    p->socket = sock->socket;
    p->frag_count = 1;
    p->frag_size = 0x240;
    p->in_use = 1;
    p->completion = 0;
    p->frag_off = (u16)(u32)&p->hdr & 0xf;
    p->hdr.length = 0x1e00;
    p->frag_seg = ((u32)&p->hdr >> 4) & 0xffff;
    c.cmd = 4;
    c.seg = ((u32)p >> 4) & 0xffff;
    c.off = (u16)(u32)p & 0xf;
    AddBatch(&c);
}

// FUNCTION: 0xd48f0
// NONMATCHING: register choice (the target keeps p in ebp, from in ebx and the
// swapped length in edx) and the flag bytes are loaded with movzx, not xor/mov
long IPXHandler::RecPkt(IPXPacket *p, __3ljac5NETSOCKET *sock, __3ljac5NETSOCKET *from)
{
    long len;

    len = 0;
    if (p->in_use == 0 && p->completion == 0) {
        len = Swap16(p->hdr.length) - 0x1e;
        from->socket = p->hdr.src_socket;
        memcpy(from->net, p->hdr.src_net, 4);
        memcpy(from->node, p->hdr.src_node, 6);
    }
    return len;
}

// FUNCTION: 0xd4964
IPXPacket *IPXHandler::GetNextSendPkt(void)
{
    IPXPacket *p;

    do {
        next_packet++;
        if (next_packet > 31)
            next_packet = 1;
        p = &send_packets[next_packet - 1];
    } while (p->in_use);
    return p;
}

// FUNCTION: 0xd4998
void IPXHandler::SendEcb(ECB *ecb)
{
    __2aq9p8IPXPARAMS c;

    c.cmd = 3;
    c.seg = ((u32)ecb >> 4) & 0xffff;
    c.off = (u16)(u32)ecb & 0xf;
    AddBatch(&c);
}

// FUNCTION: 0xd4a1c
void IPXHandler::AddBatch(__2aq9p8IPXPARAMS *c)
{
    if (++batch_count < 30) {
        memcpy(dosbuf + 0x90 + (batch_count - 1) * 6, c, 6);
    } else {
        batch_count = 31;
        BatchProcess();
    }
}

// FUNCTION: 0xd4a74
void IPXHandler::BatchProcess(void)
{
    RMREGS rm;
    __2aq9p8IPXPARAMS c;

    if (batch_count) {
        if (++batch_count <= 31) {
            c.cmd = 0xa;
            c.off = 0;
            c.seg = 0;
            memcpy(dosbuf + 0x90 + (batch_count - 1) * 6, &c, 6);
        }
        memset(&rm, 0, sizeof(rm));
        rm.ebx = 0xffff;
        rm.eax = batch_count;
        SimulateInt(vector, &rm);
        batch_count = 0;
    }
}

// FUNCTION: 0xd4b7c
IPXPacket *IPXHandler::AllocIPXPackets(long count, u32 *sel)
{
    u16 s;
    u32 size;
    IPXPacket *p;

    size = count * sizeof(IPXPacket);
    s = 0;
    p = (IPXPacket *)fn_000d4150(this, size, &s);
    if (p) {
        memset(p, 0, size);
        *sel = s;
    }
    return p;
}

// FUNCTION: 0xd4bf8
void IPXHandler::FreeIPXPackets(u32 sel)
{
    DPMIREGS r;

    r.x.eax = 0x101;
    r.x.edx = (u16)sel;
    int386(0x31, (REGS *)&r, (REGS *)&r);
}

// FUNCTION: 0xd4c20
long IPXHandler::AdvertiseServer(u16 type, char *name, __3ljac5NETSOCKET *sock)
{
    u16 i;
    __3ljac5NETSOCKET s;
    IPXPacket *pk;
    __3ljac5NETSOCKET dest;
    SAPPacket pkt;

    if (!socket_open) {
        socket = 0x452;
        s.socket = 0x452;
        if (OpenSocket(&s, 0) == 0) {
            socket_open = 1;
            socket = s.socket;
            for (i = 0, pk = recv_packets; i < 32; i++, pk++)
                Listen(pk, &s);
            BatchProcess();
        } else {
            socket = 0x4141;
            s.socket = 0x4141;
            if (OpenSocket(&s, 0) == 0) {
                socket_open = 1;
                for (i = 0, pk = recv_packets; i < 32; i++, pk++)
                    Listen(pk, &s);
                BatchProcess();
            }
        }
    }
    if (socket_open) {
        memset(&dest, 0xff, sizeof(dest));
        dest.socket = socket;
        memset(&pkt, 0, sizeof(pkt));
        pkt.op = 0x200;
        pkt.type = type;
        pkt.hops = 0x100;
        pkt.socket = sock->socket;
        memcpy(pkt.net, net, 4);
        memcpy(pkt.node, node, 6);
        strcpy(pkt.name, name);
        Send(sock, &dest, (u8 *)&pkt, sizeof(pkt));
        BatchProcess();
        return 0;
    }
    return 1;
}

// FUNCTION: 0xd5010
void IPXHandler::QueryServer(u16 type, u16 query, __3ljac5NETSOCKET *sock)
{
    __3ljac5NETSOCKET dest;
    u16 pkt[2];

    memset(&dest, 0xff, sizeof(dest));
    dest.socket = 0x5204;
    pkt[0] = type;
    pkt[1] = query;
    Send(sock, &dest, (u8 *)pkt, 4);
    dest.socket = 0x4141;
    Send(sock, &dest, (u8 *)pkt, 4);
    BatchProcess();
}

// FUNCTION: 0xd508c
void IPXHandler::UpdateServer(u16 type, char *name, __3ljac5NETSOCKET *sock)
{
    __3ljac5NETSOCKET from;
    u16 *d;
    SAPPacket pkt;
    __3ljac5NETSOCKET s;
    IPXPacket *p;
    u16 count;
    __3ljac5NETSOCKET dest;

    count = 0;
    if (socket_open) {
        memset(&pkt, 0, sizeof(pkt));
        pkt.op = 0x200;
        pkt.type = type;
        pkt.hops = 0x100;
        pkt.socket = sock->socket;
        memcpy(pkt.net, net, 4);
        memcpy(pkt.node, node, 6);
        strcpy(pkt.name, name);
        for (p = recv_packets; p < recv_packets + 32; p++) {
            if (RecPkt(p, sock, &from)) {
                d = (u16 *)p->data;
                if (memcmp(p->hdr.src_node, node, 6) != 0
                    && d[0] == 0x100 && d[1] == type) {
                    dest.socket = p->hdr.src_socket;
                    memcpy(dest.net, p->hdr.src_net, 4);
                    memcpy(dest.node, p->hdr.src_node, 6);
                    Send(sock, &dest, (u8 *)&pkt, sizeof(pkt));
                }
                s.socket = socket;
                count++;
                Listen(p, &s);
            }
        }
        unknown4 = clock();
        if (unknown4 - stamp > 4000) {
            AdvertiseServer(type, name, sock);
            stamp = unknown4;
        } else if (count) {
            BatchProcess();
        }
    }
}
