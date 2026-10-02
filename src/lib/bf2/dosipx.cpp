#include "net.h"

/* Real-mode event service routines, copied into the DOS buffer at 0x78 and
   0x80. */
static char ipxsendesr[] = { 0x2e, 0x80, 0x06, 0x88, 0x00, 0x01, 0xcb, 0x00 }; /* add byte ptr cs:[0x88], 1; retf */
static char ipxrecvesr[] = { 0x2e, 0x80, 0x06, 0x89, 0x00, 0x01, 0xcb, 0x00 }; /* add byte ptr cs:[0x89], 1; retf */

u16 Swap16(u16 v)
{
    u16 r = v >> 8;

    r |= v << 8;
    return r;
}

// Constructs an IPX handler with nothing allocated.
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
    advertise_time = clock();
    memset(IPXDEBUGSTRING, 0, 0x50);
}

// Clears the IPX handler's state (DeInit frees its memory).
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

// Sets up IPX: allocates the DOS buffer (with the real-mode event service
// routines and batch code), 32 send and 32 receive packets, finds the IPX
// driver (INT 2Fh 7A00h) and a free interrupt vector (0x60-0x65) for the
// batch code, and reads this machine's address. Returns 0, or 1 on failure.
// FUNCTION: 0xd35a0
// NONMATCHING: frame slots of the inlined DOS allocations' register blocks, and
// register choice in them
long IPXHandler::Init(void)
{
    u8 addr[10];
    u32 entry;
    RMREGS rm;

    dosbuf = (u8 *)AllocDosMem(0x950, &dosbuf_sel);
    if (dosbuf) {
        memset(dosbuf, 0, 0x950);
        rm_ss = (s32)(dosbuf + 0x15f) >> 4;
        rm_sp = (u16)(u32)dosbuf + 0x950 - (rm_ss << 4);
        send_esr = (u32)dosbuf + 0x78 - ((((u32)dosbuf >> 4) & 0xffff) << 4);
        recv_esr = (u32)dosbuf + 0x80 - ((((u32)dosbuf >> 4) & 0xffff) << 4);
        esr_size = serial_handler_code - ipx_esr_code;
        memcpy(dosbuf, ipx_esr_code, esr_size);
        memcpy(dosbuf + 0x78, ipxsendesr, 8);
        memcpy(dosbuf + 0x80, ipxrecvesr, 8);
        IPXDebug("ALLOCATED FINE HANDLER SIZE = %d\n", esr_size);
        send_packets = AllocIPXPackets(0x20, &send_sel);
        IPXDebug("IPXSEND SEG = %x, OFF = %x\n", ((u32)send_packets >> 4) & 0xffff, (u32)send_packets & 0xf);
        if (send_packets == 0) {
            strcpy(IPXDEBUGSTRING, "COULDN'T ALLOCATE FOR SEND PACKETS\n");
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            FreeDosMem(dosbuf_sel);
            dosbuf_sel = 0;
            return 1;
        }
        addrbuf = (u8 *)AllocDosMem(0xd0, &addr_sel);
        if (!addrbuf) {
            strcpy(IPXDEBUGSTRING, "COULDN'T ALLOCATE FOR BUFFER\n");
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            FreeDosMem(dosbuf_sel);
            dosbuf_sel = 0;
            FreeDosMem(send_sel);
            send_sel = 0;
            return 1;
        }
        recv_packets = AllocIPXPackets(0x20, &recv_sel);
        if (recv_packets == 0) {
            strcpy(IPXDEBUGSTRING, "COULDN'T ALLOCATE FOR SAP BUFFER\n");
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            FreeDosMem(dosbuf_sel);
            dosbuf_sel = 0;
            FreeDosMem(send_sel);
            send_sel = 0;
            FreeDosMem(addr_sel);
            addr_sel = 0;
            return 1;
        }
        memset(&rm, 0, sizeof(rm));
        rm.eax = 0x7a00;
        CallRealModeInterrupt(0x2f, &rm);
        *(u16 *)(dosbuf + 0x8a) = rm.edi;
        *(u16 *)(dosbuf + 0x8c) = rm.es;
        if ((rm.eax & 0xff) != 0xff) {
            strcpy(IPXDEBUGSTRING, "IPX DRIVER NOT INSTALLED\n");
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            FreeDosMem(dosbuf_sel);
            dosbuf_sel = 0;
            FreeDosMem(send_sel);
            send_sel = 0;
            FreeDosMem(addr_sel);
            addr_sel = 0;
            FreeDosMem(recv_sel);
            recv_sel = 0;
            return 1;
        }
        entry = rm.edi | ((u32)rm.es << 16);
        memcpy(dosbuf + 2, &entry, 4);
        IPXDebug("IPX ENTRY = %x\n", dosbuf[2]);
        if (!SetupVector()) {
            strcpy(IPXDEBUGSTRING, "COULD NOT FIND FREE VECTOR FROM 0x60 to 0x66\n");
            IPXDebug(IPXDEBUGSTRING);
            dosbuf = 0;
            FreeDosMem(dosbuf_sel);
            dosbuf_sel = 0;
            FreeDosMem(send_sel);
            send_sel = 0;
            FreeDosMem(addr_sel);
            addr_sel = 0;
            FreeDosMem(recv_sel);
            recv_sel = 0;
            IPXDebug("FAILED\n");
            return 1;
        }
        sprintf(IPXDEBUGSTRING, "INITSUCCESSFULL IPX ADDRESS = SEG %x OFF %x\n", *(u16 *)(dosbuf + 0x8c), *(u16 *)(dosbuf + 0x8a));
        IPXDebug("%s", IPXDEBUGSTRING);
        GetAddress(addr);
        batch_count = 0;
        return 0;
    } else {
        strcpy(IPXDEBUGSTRING, "COULDN'T ALLOCATE FOR DRIVER\n");
        return 1;
    }
}

// Closes the socket, restores the vector and frees the IPX DOS memory.
// FUNCTION: 0xd3eb8
void IPXHandler::DeInit(void)
{
    DPMIREGS r;

    if (dosbuf) {
        if (socket_open) {
            __3ljac5NETSOCKET listen_sock;

            listen_sock.socket = socket;
            CloseSocket(&listen_sock);
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
        FreeDosMem(dosbuf_sel);
        dosbuf_sel = 0;
        FreeDosMem(send_sel);
        send_sel = 0;
        FreeDosMem(addr_sel);
        addr_sel = 0;
        FreeDosMem(recv_sel);
        recv_sel = 0;
        IPXDebug("DEALLOCATED\n");
        strcpy(IPXDEBUGSTRING, "DEINIT SUCCESSFULL\n");
        advertise_time = clock();
        batch_count = 0;
    } else {
        strcpy(IPXDEBUGSTRING, "DEINIT FAILED\n");
    }
}

// Points the first free interrupt vector from 0x60 to 0x65 at the DOS
// buffer's batch code. Only an unset (0:0) vector counts as free: the test
// for an IRET compares a sign-extended byte with 0xcf and never succeeds.
// Returns 1, or 0 if none is free.
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
            IPXDebug("CAN USE THIS VECTOR 0x%x\n", vec);
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

// Allocates size bytes of DOS memory (DPMI), storing its selector in *sel.
// Returns its linear address, or 0.
// FUNCTION: 0x000d4150
void *IPXHandler::AllocDosMem(u32 size, u16 *sel)
{
    DPMIREGS r;

    r.x.eax = 0x100;
    r.x.ebx = (size + 15) >> 4;
    int386(0x31, (REGS *)&r, (REGS *)&r);
    if (r.x.cflag)
        return 0;
    if (sel)
        *sel = r.x.edx;
    return (void *)((r.x.eax & 0xffff) << 4);
}

// Frees the DOS memory with selector sel.
// FUNCTION: 0x000d41a0
void IPXHandler::FreeDosMem(u16 sel)
{
    DPMIREGS r;

    r.x.eax = 0x101;
    r.x.edx = sel;
    int386(0x31, (REGS *)&r, (REGS *)&r);
}

// Calls real-mode interrupt intno with registers rm (on the handler's own
// real-mode stack). Returns rm's eax.
// FUNCTION: 0xd41c8
long IPXHandler::CallRealModeInterrupt(u16 intno, RMREGS *rm)
{
    DPMIREGS r;
    SREGS s;

    memset(&r, 0, sizeof(r));
    r.x.eax = 0x300;
    r.x.ebx = intno;
    segread(&s);
    rm->ss = rm_ss;
    rm->sp = rm_sp;
    r.x.edi = (u32)rm;
    int386x(0x31, (REGS *)&r, (REGS *)&r, &s);
    return rm->eax;
}

// Reads this machine's IPX network and node address into addr (10 bytes),
// net and node. Returns the driver's result.
// FUNCTION: 0xd4348
long IPXHandler::GetAddress(u8 *addr)
{
    RMREGS rm;

    memset(&rm, 0, sizeof(rm));
    rm.ebx = 9;
    rm.es = ((u32)addrbuf >> 4) & 0xffff;
    rm.esi = (u32)addrbuf & 0xf;
    CallRealModeInterrupt(vector, &rm);
    memcpy(addr, addrbuf, 10);
    memcpy(net, addr, 4);
    memcpy(node, addr + 4, 6);
    IPXDebug("NETWORK %x%x%x%x\nNODE %02x%02x%02x%02x%02x%02x\n", net[0], net[1], net[2], net[3], node[0], node[1], node[2], node[3], node[4], node[5]);
    IPXDebug("RET = %x\n", rm.eax);
    return rm.eax;
}

// Opens IPX socket sock->socket (0: dynamic) with longevity mode, filling
// in the socket number and this machine's address. Returns the driver's
// result, 0 on success.
// FUNCTION: 0xd44a4
long IPXHandler::OpenSocket(__3ljac5NETSOCKET *sock, u16 mode)
{
    RMREGS rm;

    memset(&rm, 0, sizeof(rm));
    rm.eax = mode;
    rm.edx = Swap16(sock->socket);
    CallRealModeInterrupt(vector, &rm);
    sock->socket = rm.edx;
    memcpy(sock->net, net, 10);
    IPXDebug("OPEN SOCKET 0x%x, RETURN CODE 0x%x\n", rm.edx, rm.eax);
    return rm.eax;
}

// Closes IPX socket sock.
// FUNCTION: 0xd4584
void IPXHandler::CloseSocket(__3ljac5NETSOCKET *sock)
{
    RMREGS rm;

    IPXDebug("CLOSING SOCKET %x\n", sock->socket);
    memset(&rm, 0, sizeof(rm));
    rm.ebx = 1;
    rm.edx = sock->socket;
    CallRealModeInterrupt(vector, &rm);
    IPXDebug("SOCKET CLOSED\n");
}

// Queues len bytes of data from socket sock to dest in the next free send
// packet.
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

// Queues receive packet p to listen on socket sock.
// FUNCTION: 0xd4800
void IPXHandler::Listen(IPXPacket *p, __3ljac5NETSOCKET *sock)
{
    __2aq9p8IPXPARAMS entry;

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
    entry.cmd = 4;
    entry.seg = ((u32)p >> 4) & 0xffff;
    entry.off = (u16)(u32)p & 0xf;
    AddBatch(&entry);
}

// If receive packet p has arrived, fills from with its sender and returns
// its data length, else 0. (The socket parameter is unused.)
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

// Returns the next send packet not in use (cycling through 31).
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

// Queues ECB ecb for sending.
// FUNCTION: 0xd4998
void IPXHandler::SendEcb(ECB *ecb)
{
    __2aq9p8IPXPARAMS entry;

    entry.cmd = 3;
    entry.seg = ((u32)ecb >> 4) & 0xffff;
    entry.off = (u16)(u32)ecb & 0xf;
    AddBatch(&entry);
}

// Adds IPX call entry to the batch. The entry that would be the 30th is
// dropped and the batch run instead (with count 32 and no end entry).
// FUNCTION: 0xd4a1c
void IPXHandler::AddBatch(__2aq9p8IPXPARAMS *entry)
{
    if (++batch_count < 30) {
        memcpy(dosbuf + 0x90 + (batch_count - 1) * 6, entry, 6);
    } else {
        batch_count = 31;
        BatchProcess();
    }
}

// Runs the batched IPX calls through the real-mode batch code.
// FUNCTION: 0xd4a74
void IPXHandler::BatchProcess(void)
{
    RMREGS rm;
    __2aq9p8IPXPARAMS entry;

    if (batch_count) {
        if (++batch_count <= 31) {
            entry.cmd = 0xa;
            entry.off = 0;
            entry.seg = 0;
            memcpy(dosbuf + 0x90 + (batch_count - 1) * 6, &entry, 6);
        }
        memset(&rm, 0, sizeof(rm));
        rm.ebx = 0xffff;
        rm.eax = batch_count;
        CallRealModeInterrupt(vector, &rm);
        batch_count = 0;
    }
}

// Allocates and clears count IPX packets in DOS memory, storing the
// selector in *sel. Returns them, or 0.
// FUNCTION: 0xd4b7c
IPXPacket *IPXHandler::AllocIPXPackets(long count, u32 *sel)
{
    u16 dos_sel;
    u32 size;
    IPXPacket *p;

    size = count * sizeof(IPXPacket);
    dos_sel = 0;
    p = (IPXPacket *)AllocDosMem(size, &dos_sel);
    if (p) {
        memset(p, 0, size);
        *sel = dos_sel;
    }
    return p;
}

// Frees the IPX packets with selector sel.
// FUNCTION: 0xd4bf8
void IPXHandler::FreeIPXPackets(u32 sel)
{
    DPMIREGS r;

    r.x.eax = 0x101;
    r.x.edx = (u16)sel;
    int386(0x31, (REGS *)&r, (REGS *)&r);
}

// Opens the SAP listening socket if needed (0x452, else 0x4141) and
// broadcasts a SAP advertisement of server name of type `type` on socket
// sock. Returns 0, or 1 if no socket could be opened.
// FUNCTION: 0xd4c20
long IPXHandler::AdvertiseServer(u16 type, char *name, __3ljac5NETSOCKET *sock)
{
    u16 i;
    __3ljac5NETSOCKET listen_sock;
    IPXPacket *pk;
    __3ljac5NETSOCKET dest;
    SAPPacket pkt;

    if (!socket_open) {
        socket = 0x452;
        listen_sock.socket = 0x452;
        if (OpenSocket(&listen_sock, 0) == 0) {
            socket_open = 1;
            socket = listen_sock.socket;
            for (i = 0, pk = recv_packets; i < 32; i++, pk++)
                Listen(pk, &listen_sock);
            BatchProcess();
        } else {
            socket = 0x4141;
            listen_sock.socket = 0x4141;
            if (OpenSocket(&listen_sock, 0) == 0) {
                socket_open = 1;
                for (i = 0, pk = recv_packets; i < 32; i++, pk++)
                    Listen(pk, &listen_sock);
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

// Broadcasts SAP query `query` for servers of type `type` from socket sock.
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

// Answers SAP queries for type `type` received on the listening socket with
// server name's details, requeues the packets, and re-advertises every 4000
// clock ticks.
// FUNCTION: 0xd508c
void IPXHandler::UpdateServer(u16 type, char *name, __3ljac5NETSOCKET *sock)
{
    __3ljac5NETSOCKET from;
    u16 *query;
    SAPPacket pkt;
    __3ljac5NETSOCKET listen_sock;
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
                query = (u16 *)p->data;
                if (memcmp(p->hdr.src_node, node, 6) != 0
                    && query[0] == 0x100 && query[1] == type) {
                    dest.socket = p->hdr.src_socket;
                    memcpy(dest.net, p->hdr.src_net, 4);
                    memcpy(dest.node, p->hdr.src_node, 6);
                    Send(sock, &dest, (u8 *)&pkt, sizeof(pkt));
                }
                listen_sock.socket = socket;
                count++;
                Listen(p, &listen_sock);
            }
        }
        last_update = clock();
        if (last_update - advertise_time > 4000) {
            AdvertiseServer(type, name, sock);
            advertise_time = last_update;
        } else if (count) {
            BatchProcess();
        }
    }
}

// Debug printf; does nothing in this build.
// FUNCTION: 0xd5340 COMDAT
inline void IPXDebug(char *fmt, ...)
{
}
