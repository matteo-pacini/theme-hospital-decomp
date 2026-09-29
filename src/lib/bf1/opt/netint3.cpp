#include "net.h"

struct __4p8ei9UnidirectionalHeader {
    u8 stub[4];
    u32 type;
};

struct __68fguuUnidirectionalRTSMessage {
    u8 stub[4];
    u32 type;
    u32 size;
    u32 block_size;
    u32 window;
};

struct __2unp93UnidirectionalDataMessage {
    u8 stub[4];
    u32 type;
    u32 block;
    union {
        u8 data[0x200];
        struct {
            u32 last;
            u32 window;
        };
    };
};

struct UnidirectionalRequestMessage {
    u8 stub[4];
    u32 type;
    u32 first;
    u32 last;
};

extern __68fguuUnidirectionalRTSMessage rtsMessage;
extern __2unp93UnidirectionalDataMessage dataMessage;
extern __4p8ei9UnidirectionalHeader endMessage;
extern __4p8ei9UnidirectionalHeader abortMessage;
extern __2unp93UnidirectionalDataMessage incomingUnidirectionalMessage;
extern u32 unidirectionalMsgReceived;

extern "C" {
extern char D_001242c8[];
extern char D_00124310[];
extern char D_0012435f[];
extern char D_001243a6[];
extern char D_001243da[];
extern char D_00124416[];
extern char D_00124454[];
extern char D_00124493[];
extern char D_001244d4[];
extern char D_00124504[];
extern char D_00124555[];
extern char D_00124596[];
extern char D_001245d6[];
extern char D_00124617[];
extern char D_00124665[];
extern char D_001246a6[];
}

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000caac0
TbError LbNetwork_Send(u32 player, void *buf, u32 len)
{
    __2unp93UnidirectionalDataMessage *msg;
    u32 last_block;
    u32 id;
    s32 start;
    u32 running;
    u8 *ptr;
    u32 remaining;
    u32 block;
    u32 n;
    s32 elapsed;
    TbError ret;

    ret = Lb_FAIL;
    spPtr->Receive(6);
    if (player >= maximumPlayers) {
        fn_000c20c0(D_001242c8, player, maximumPlayers - 1);
        return Lb_FAIL;
    }
    if (!clientDataTable[player].active)
        return Lb_FAIL;
    id = clientDataTable[player].id;
    rtsMessage.type = 0;
    rtsMessage.size = len;
    rtsMessage.block_size = 0x200;
    rtsMessage.window = 3;
    EncodeMessageStub(&rtsMessage, 0x10, MSG_TYPE_UNIDIRECTIONAL, 0);
    if (spPtr->Send(id, &rtsMessage) != Lb_OK) {
        fn_000c20c0(D_001243da);
        return ret;
    }
    start = LbTimerClock();
    last_block = (len - 1) >> 9;
    running = 1;
    do {
        spPtr->Receive(8);
        if (unidirectionalMsgReceived) {
            unidirectionalMsgReceived = 0;
            switch (incomingUnidirectionalMessage.type) {
            case 1:
                msg = &incomingUnidirectionalMessage;
                if (msg->block > last_block || last_block < msg->last) {
                    fn_000c20c0(D_00124310, msg->block, msg->last);
                    running = 0;
                    break;
                }
                block = msg->block;
                ptr = (u8 *)buf + (block << 9);
                remaining = len - (block << 9);
                for (; block <= msg->last; block++) {
                    n = remaining < 0x200 ? remaining : 0x200;
                    memcpy(dataMessage.data, ptr, n);
                    dataMessage.block = block;
                    dataMessage.type = 2;
                    EncodeMessageStub(&dataMessage, n + 8, MSG_TYPE_UNIDIRECTIONAL, 0);
                    if (spPtr->Send(id, &dataMessage) == Lb_OK) {
                        start = LbTimerClock();
                    } else {
                        fn_000c20c0(D_0012435f, block);
                        running = 0;
                    }
                    remaining -= 0x200;
                    ptr += 0x200;
                }
                break;
            case 3:
                running = 0;
                ret = Lb_OK;
                break;
            case 4:
                running = 0;
                ret = Lb_FAIL;
                break;
            }
        } else {
            elapsed = LbTimerClock() - start;
            if ((elapsed < 0 ? -elapsed : elapsed) > 30000) {
                fn_000c20c0(D_001243a6);
                running = 0;
            }
        }
    } while (running);
    return ret;
}

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000cade4
u32 LbNetwork_Receive(u32 player, void *buf, u32 len)
{
    u32 pending[17];
    UnidirectionalRequestMessage req;
    u32 result;
    u32 id;
    u32 block_size;
    s32 start;
    u32 last_block;
    u32 next;
    u32 window;
    u32 total;
    s32 block_time;
    u32 running;
    u32 first;
    u32 last;
    u32 waiting;
    u32 got_rts;
    u32 block;
    u32 n;
    u32 i;
    s32 elapsed;

    start = LbTimerClock();
    waiting = 1;
    got_rts = 0;
    spPtr->Receive(6);
    result = 0;
    if (player >= maximumPlayers)
        return 0;
    if (!clientDataTable[player].active)
        return 0;
    id = clientDataTable[player].id;
    do {
        spPtr->Receive(8);
        if (!unidirectionalMsgReceived) {
            elapsed = LbTimerClock() - start;
            if ((elapsed < 0 ? -elapsed : elapsed) > 30000) {
                fn_000c20c0(D_001244d4);
                waiting = 0;
            }
        } else {
            unidirectionalMsgReceived = 0;
            if (incomingUnidirectionalMessage.type == 0) {
                total = incomingUnidirectionalMessage.block;
                block_size = incomingUnidirectionalMessage.last;
                window = incomingUnidirectionalMessage.window;
                if (block_size > 0x200)
                    fn_000c20c0(D_00124416, block_size);
                else if (len < total)
                    fn_000c20c0(D_00124454, total);
                else
                    got_rts = 1;
                waiting = 0;
                if (!got_rts) {
                    abortMessage.type = 4;
                    EncodeMessageStub(&abortMessage, 4, MSG_TYPE_UNIDIRECTIONAL, 0);
                    if (spPtr->Send(id, &abortMessage) != Lb_OK) {
                        fn_000c20c0(D_00124493);
                        waiting = 0;
                    }
                }
            }
        }
    } while (waiting);
    if (got_rts) {
        running = 1;
        last_block = (total - 1) / block_size;
        first = 0;
        do {
            if (first > last_block) {
                running = 0;
                result = total;
                continue;
            }
            last = first + window;
            if (last > last_block)
                last = last_block;
            req.type = 1;
            req.first = first;
            req.last = last;
            EncodeMessageStub(&req, 0xc, MSG_TYPE_UNIDIRECTIONAL, 0);
            if (spPtr->Send(id, &req) != Lb_OK) {
                fn_000c20c0(D_00124617, first, last);
                running = 0;
                abortMessage.type = 4;
                EncodeMessageStub(&abortMessage, 4, MSG_TYPE_UNIDIRECTIONAL, 0);
                if (spPtr->Send(id, &abortMessage) != Lb_OK)
                    fn_000c20c0(D_00124665);
                continue;
            }
            waiting = 1;
            for (i = 0; i <= window; i++)
                pending[i] = 1;
            block_time = LbTimerClock();
            next = last + 1;
            while (waiting) {
                spPtr->Receive(8);
                if (unidirectionalMsgReceived) {
                    unidirectionalMsgReceived = 0;
                    if (incomingUnidirectionalMessage.type == 2) {
                        block = incomingUnidirectionalMessage.block;
                        if (block < first || block > last) {
                            fn_000c20c0(D_00124504, block, first, last);
                            abortMessage.type = 4;
                            waiting = 0;
                            EncodeMessageStub(&abortMessage, 4, MSG_TYPE_UNIDIRECTIONAL, 0);
                            running = 0;
                            if (spPtr->Send(id, &abortMessage) != Lb_OK)
                                fn_000c20c0(D_00124555);
                        } else {
                            n = total - block_size * block;
                            if (n > block_size)
                                n = block_size;
                            memcpy((u8 *)buf + block_size * block, incomingUnidirectionalMessage.data, n);
                            pending[block - first] = 0;
                            waiting = 0;
                            for (i = first; i <= last; i++) {
                                if (pending[i - first])
                                    waiting = 1;
                            }
                            if (waiting)
                                block_time = LbTimerClock();
                            else
                                first = next;
                        }
                    } else {
                        fn_000c20c0(D_00124596, incomingUnidirectionalMessage.type);
                    }
                } else {
                    elapsed = LbTimerClock() - block_time;
                    if ((elapsed < 0 ? -elapsed : elapsed) > 15000) {
                        abortMessage.type = 4;
                        waiting = 0;
                        EncodeMessageStub(&abortMessage, 4, MSG_TYPE_UNIDIRECTIONAL, 0);
                        running = 0;
                        if (spPtr->Send(id, &abortMessage) != Lb_OK)
                            fn_000c20c0(D_001245d6);
                    }
                }
            }
        } while (running);
        if (result == total) {
            endMessage.type = 3;
            EncodeMessageStub(&endMessage, 4, MSG_TYPE_UNIDIRECTIONAL, 0);
            if (spPtr->Send(id, &endMessage) != Lb_OK)
                fn_000c20c0(D_001246a6);
        }
    }
    return result;
}
