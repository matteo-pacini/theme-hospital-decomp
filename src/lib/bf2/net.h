#ifndef BF2_NET_H
#define BF2_NET_H

#include "bflib.h"

extern "C" {
void *memcpy(void *dst, const void *src, unsigned size);
unsigned strlen(const char *s);
char *strcpy(char *dst, const char *src);
int memcmp(const void *a, const void *b, unsigned size);
#pragma intrinsic(memcpy, strlen, strcpy, memcmp)
void fn_000c20c0(char *fmt, ...);
void *fn_000ed344(u32 size);
}

struct TbNetworkCallbackData;

// A provider message's type (its stub header, EncodeMessageStub), named
// after the ServiceProvider::Receive callback it goes to. Also passed to
// SendMessage as the delivery mode (0 not guaranteed, 1 guaranteed).
typedef enum {
    // Exchange data for the multiPlayer callback.
    MSG_TYPE_MULTIPLAYER,
    MSG_TYPE_ADD_PLAYER,
    MSG_TYPE_DELETE_PLAYER,
    MSG_TYPE_SYSTEM_USER = 4,
    MSG_TYPE_REQ_EXCHANGE_DATA,
    MSG_TYPE_REQ_COMPOSITE_EXCHANGE_DATA,
    MSG_TYPE_UNIDIRECTIONAL,
    MSG_TYPE_8
} TbMessageType;

// ServiceProvider::Receive flags: which message types to take from the
// queue (RECV_ALL: every type).
#define RECV_MULTIPLAYER 0x1
#define RECV_ADD_PLAYER 0x2
#define RECV_DELETE_PLAYER 0x4
#define RECV_ALL 0x8
#define RECV_REQ_EXCHANGE_DATA 0x10
#define RECV_REQ_COMPOSITE_EXCHANGE_DATA 0x20
#define RECV_UNIDIRECTIONAL 0x40
#define RECV_SYSTEM_USER 0x80
#define RECV_MSG_TYPE_8 0x100

struct TbNetworkPlayerEntry {
    u8 unknown0;
    u32 id;
    u32 local;
    u32 host;
    char name[0x20];
};

struct __ic76sReceiveCallbacks {
    void (__stdcall *addMsg)(u32 id, char *name, void *data);
    void (__stdcall *deleteMsg)(u32 id, void *data);
    void (__stdcall *hostMsg)(u32 id, void *data);
    void (__stdcall *unknownc)(void *data);
    void *(__stdcall *multiPlayer)(u32 id, u32 size, u32 seq, void *data);
    void (__stdcall *mpReqExDataMsg)(u32 id, u32 seq, void *data);
    void (__stdcall *mpReqCompsExDataMsg)(u32 id, u32 seq, void *data);
    void *(__stdcall *unidirectionalMsg)(u32 id, u32 size, void *data);
    void (__stdcall *systemUserMsg)(u32 id, void *msg, u32 size, void *data);
    void (__stdcall *unknown24)(u32 id, void *msg);
};

struct ServiceProvider {
    u32 local_id;
    TbNetworkSessionNameEntry sessions[32];
    u32 init_count;
    u32 in_session;
    u32 player_count;
    TbNetworkPlayerEntry players[32];
    char name[0x20];
    __ic76sReceiveCallbacks *callbacks;
    u32 started;
    void *callback_data;

    ServiceProvider();
    virtual ~ServiceProvider();
    virtual TbError Start(TbNetworkSessionNameEntry *session, char *name, void *data) = 0;
    virtual TbError Start(char *session, char *name, u32 max_players, void *data) = 0;
    virtual TbError Stop(void) = 0;
    virtual TbError Enumerate(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data) = 0;
    virtual TbError Enumerate(TbNetworkSessionNameEntry *session, void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data) = 0;
    virtual TbError Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *callbacks, void *data) = 0;
    virtual TbError Release(void);
    virtual TbError ChangeSettings(u32 setting, void *data) = 0;
    virtual TbError EnableNewPlayers(u32 allow);
    virtual u32 ReadMessage(u32 *from, void *buf, u32 *size) = 0;
    virtual u32 PeekMessage(u32 *from, void *buf, u32 *size) = 0;
    virtual TbError SendMessage(u32 to, void *buf, TbMessageType type) = 0;

    TbError Initialise(__ic76sReceiveCallbacks *callbacks, void *data);
    TbError AddPlayer(u32 id, char *name, u32 is_local, u32 is_host);
    void ClearPlayers(void);
    TbError DeletePlayer(u32 id);
    void ClearSessions(void);
    TbNetworkSessionNameEntry *AddSession(u32 id, char *name);
    TbError EnumeratePlayers(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data);
    u32 GetAddPlayerMsgSize(char *name);
    void EncodeAddPlayerMsg(u8 *buf, u32 id, char *name);
    u32 DecodeAddPlayerMsg(u8 *buf, u32 &id, char *name);
    TbError CheckForDeletedHost(u8 *msg);
    TbError Receive(u32 flags);
    TbError Send(u32 to, void *buf);
    TbError LookForSystemMessages(void);
    TbError BroadcastSystemMessage(u8 *msg);
    TbError SystemAddPlayerHandler(u8 *msg);
    TbError SystemDeletePlayerHandler(u8 *msg);
    TbError EnumeratePlayersForSessionRunning(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data);
};

u32 GetRequestCompositeExchangeDataMsgSize(void);
void EncodeDeletePlayerMsg(u8 *buf, u32 id);
u32 DecodeRequestExchangeDataMsg(u8 *buf, u32 &id);
u32 DecodeRequestCompositeExchangeDataMsg(u8 *buf, u32 &id);
void EncodeRequestExchangeDataMsg(u8 *buf, u32 id, u32 seq);
void EncodeRequestCompositeExchangeDataMsg(u8 *buf, u32 id, u32 seq);
void StrCpyTrunc(char *dst, char *src, u32 size);
void EncodeMessageStub(void *buf, u32 size, TbMessageType type, u32 seq);
void DecodeMessageStub(void *buf, u32 *size, TbMessageType *type, u32 *seq);



struct __4kqpm7ClientDataEntry {
    u32 id;
    u32 active;
    u8 unknown8[0x24];
};

struct __4dn7ojSerialInitData {
    u32 port;
    u32 baud;
    s32 irq;
    u32 mode;
    char *init_str;
    char *dial_str;
    char *hangup_str;
    char *reset_str;
};

struct __iobuf;

struct CommHandlerData {
    char response[0x50];
    long timeout;
    long dial_timeout;
    char init_str[0x50];
    char dial_str[0x50];
    char hangup_str[0x50];
    char reset_str[0x50];
    long port;
    long response_timeout;
    char last_cmd[0x50];
    u8 online;
    long (*init_func)(void);
    long (*dial_func)(void);
    long (*hangup_func)(void);
    long (*other_func)(void);
    long (*cur_func)(void);
};

#define CH(p) ((CommHandlerData *)(p))

struct CommHandler {
    u8 unknown0[0x205];

    CommHandler();
    ~CommHandler();
    long Init(long port);
    long DeInit(void);
    long ReadByte(void);
    u32 ReadBytes(void *buf, u32 len);
    long WriteBytes(u8 *buf, long len);
    long WriteBytesNoWait(u8 *buf, long len);
    void EmptyOutputBuffer(void);
    long SetBaud(long baud);
    void SendModemString(char *str);
    long InitModem(void);
    long DialModem(char *number);
    long HangUpModem(void);
    long AnswerModem(void);
    long ReadConfigFile(char *filename);
    void read_a_line(__iobuf *f, char *p);
    long RunModemFunc(long (*fn)(void));
    void ReadSerialDev(__4dn7ojSerialInitData *data);
    long GetModemResponse(long timeout);
    void SetInitModemFunc(long (*fn)(void));
    void SetDialModemFunc(long (*fn)(void));
    void SetHangUpModemFunc(long (*fn)(void));
    void SetAnswerModemFunc(long (*fn)(void));
    void SetModemResponseTimeOut(long seconds);
    void SetDtr(void);
    void SetRts(void);
    void ClearDtr(void);
    void ClearRts(void);
};

enum ParserState {
    PS_HEADER,
    PS_SEQUENCE,
    PS_SIZE,
    PS_DATA,
    PS_CHECKSUM,
    PS_COMPLETE
};

struct Parser {
    u32 active;
    ParserState state;
    u32 count;
    u8 msg[0x407];
    u8 header[4];
    u8 seq;
    u32 size;

    Parser();
    void Init(void);
    u32 InUse(void);
    void TakeDown(void);
    u8 *GetMessageBuffer(void);
    u32 GotCompleteMessage(void);
    u8 GetFirstHeaderByte(void);
    u8 GetSequenceNumber(void);
    TbError InputByte(u8 byte);
};

struct SerialSP : ServiceProvider {
    __tibcaGUID guid;
    u32 is_host;
    CommHandler *comm;
    u32 port;
    u32 baud;
    __4dn7ojSerialInitData serial_data;
    u8 send_seq;
    u8 recv_seq;
    u32 awaiting_ack;
    u8 unknownb46[4];
    Parser parsers[10];

    SerialSP();
    SerialSP(__4dn7ojSerialInitData *init_data);
    TbError Release(void);
    TbError Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *callbacks, void *data);
    TbError Start(char *session, char *name, u32 max_players, void *data);
    TbError Start(TbNetworkSessionNameEntry *session, char *name, void *data);
    TbError Stop(void);
    void SendAck(u8 seq);
    u32 PrivateReceive(void *buf, u32 *size, TbMessageType mode);
    u32 ProcessAvailableMsg(void *buf, u32 *size, u32 remove);
    TbError Enumerate(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data);
    TbError Enumerate(TbNetworkSessionNameEntry *session, void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data);
    TbError ChangeSettings(u32 setting, void *data);
    u32 ReadMessage(u32 *from, void *buf, u32 *size);
    u32 PeekMessage(u32 *from, void *buf, u32 *size);
    TbError SendMessage(u32 to, void *buf, TbMessageType type);
};

struct ModemSP : SerialSP {
    u8 unknown3444[0x34d4 - 0x3444];

    ModemSP(__4dn7ojSerialInitData *init_data);
    TbError Release(void);
    TbError Init(__tibcaGUID guid, __ic76sReceiveCallbacks *callbacks, void *data);
    TbError SendMessage(u32 to, void *buf, TbMessageType type);
    TbError Stop(void);
    TbError Start(char *session, char *name, u32 max_players, void *data);
    TbError Start(TbNetworkSessionNameEntry *session, char *name, void *data);
    u32 ReadMessage(u32 *from, void *buf, u32 *size);
    u32 PeekMessage(u32 *from, void *buf, u32 *size);
};

extern ServiceProvider *spPtr;
extern u32 runningTwoPlayerModel;
extern __ic76sReceiveCallbacks receiveCallbacks;
extern u32 remotePlayerIndex;
extern __4kqpm7ClientDataEntry clientDataTable[];
extern u32 gotCompositeData;
extern u32 exchangeSize;
extern u32 localPlayerIndex;
extern u8 *exchangeBuffer;
extern u32 sequenceNumber;
extern u32 remotePlayerId;
extern u32 localPlayerId;
extern u32 hostId;
extern u32 exchangeTimeout;
extern s32 startTime;
extern u32 basicTimeout;
extern u32 timeCount;
extern u8 lastMessage[];
extern u8 lastButOneMessage[];
extern u8 deletePlayerBuffer[];
extern u8 requestExchangeDataBuffer[];
extern u8 addPlayerBuffer[];
extern u8 ackBuffer[8];
extern "C" {
extern u32 D_0018ba9c;
extern u8 D_0018d428[];
}

/* __stdcall (arguments on the stack, callee pops). */
void *TwoPlayerCallback(u32 id, u32 size, u32 seq, void *data);
#pragma aux TwoPlayerCallback parm routine [] modify [eax ecx edx]
void TwoPlayerReqExDataMsgCallback(u32 id, u32 seq, void *data);
#pragma aux TwoPlayerReqExDataMsgCallback parm routine [] modify [eax ecx edx]

struct __3ljac5NETSOCKET {
    u16 socket;
    u8 unknown2[4];
    u8 net[4];
    u8 node[6];
};

struct ECB;

struct IPXHeader {
    u16 checksum;
    u16 length;
    u8 transport;
    u8 type;
    u8 dest_net[4];
    u8 dest_node[6];
    u16 dest_socket;
    u8 src_net[4];
    u8 src_node[6];
    u16 src_socket;
};

struct IPXPacket {
    u32 link;
    u16 esr_off;
    u16 esr_seg;
    u8 in_use;
    u8 completion;
    u16 socket;
    u8 workspace[0x10];
    u8 immediate[6];
    u16 frag_count;
    u16 frag_off;
    u16 frag_seg;
    u16 frag_size;
    IPXHeader hdr;
    u8 data[0x222];
};

struct __2aq9p8IPXPARAMS {
    u16 cmd;
    u16 off;
    u16 seg;
};

struct SAPPacket {
    u16 op;
    u16 type;
    char name[0x30];
    u8 net[4];
    u8 node[6];
    u16 socket;
    u16 hops;
};

struct __1epnl9RMREGS {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 reserved;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u16 flags;
    u16 es;
    u16 ds;
    u16 fs;
    u16 gs;
    u16 ip;
    u16 cs;
    u16 sp;
    u16 ss;
};
typedef __1epnl9RMREGS RMREGS;

typedef REGS DPMIREGS;

extern "C" {
void IPXDebug(char *fmt, ...);
#pragma aux IPXDebug "fn_000d5340_"
extern u8 D_00110000[];
extern u8 D_00110068[];
}

extern char IPXDEBUGSTRING[];

struct IPXHandler {
    u32 stamp;
    u32 last_update;
    u32 socket_open;
    u8 net[4];
    u8 node[6];
    IPXPacket *send_packets;
    IPXPacket *recv_packets;
    u8 *dosbuf;
    u8 *addrbuf;
    u32 esr_size;
    u16 dosbuf_sel;
    u32 send_sel;
    u32 recv_sel;
    u16 addr_sel;
    u16 rm_ss;
    u16 rm_sp;
    u16 send_esr;
    u16 recv_esr;
    u16 vector;
    u16 old_seg;
    u16 old_off;
    u16 next_packet;
    u16 batch_count;
    u16 socket;

    IPXHandler();
    ~IPXHandler();
    long Init(void);
    void DeInit(void);
    void *AllocDosMem(u32 size, u16 *sel);
    void FreeDosMem(u16 sel);
    long CallRealModeInterrupt(u16 intno, RMREGS *rm);
    long SetupVector(void);
    long GetAddress(u8 *addr);
    long OpenSocket(__3ljac5NETSOCKET *sock, u16 mode);
    void CloseSocket(__3ljac5NETSOCKET *sock);
    void Send(__3ljac5NETSOCKET *sock, __3ljac5NETSOCKET *dest, u8 *data, long len);
    void Listen(IPXPacket *packet, __3ljac5NETSOCKET *sock);
    long RecPkt(IPXPacket *packet, __3ljac5NETSOCKET *sock, __3ljac5NETSOCKET *from);
    void BatchProcess(void);
    IPXPacket *AllocIPXPackets(long count, u32 *sel);
    void FreeIPXPackets(u32 sel);
    long AdvertiseServer(u16 type, char *name, __3ljac5NETSOCKET *sock);
    void QueryServer(u16 type, u16 query, __3ljac5NETSOCKET *sock);
    void UpdateServer(u16 type, char *name, __3ljac5NETSOCKET *sock);
    IPXPacket *GetNextSendPkt(void);
    void SendEcb(ECB *ecb);
    void AddBatch(__2aq9p8IPXPARAMS *cmd);
};


struct IPXPlayerEntry {
    u32 id;
    char name[0x20];
    __3ljac5NETSOCKET addr;
    u16 ack;
    u16 abort_acked;
};

struct IPXSessionInfo {
    u32 magic;
    u8 unknown4[8];
    __tibcaGUID guid;
    char name[0x20];
    u32 local_id;
    u32 host_id;
    u16 num_players;
    u16 max_players;
    u32 page;
    u32 unknown4c;
};

struct __5303ctCLIENTINFOPACKET {
    u32 magic;
    u8 unknown4[8];
    __tibcaGUID guid;
    char name[0x20];
    u32 session_id;
    u32 local_id;
    u32 host_id;
    u16 num_players;
    u16 max_players;
    u32 page;
    u32 unknown50;
};

struct __35ct5sCLIENTJOINPACKET {
    u32 magic;
    u8 unknown4[8];
    __tibcaGUID guid;
    u8 unknown1c[0x3c];
    char name[0x20];
    __3ljac5NETSOCKET addr;
    u32 unknown88;
};

struct __1p5pi1IPXSYSTEMMESSAGE {
    u32 magic;
    u32 host;
    u8 unknown8[2];
    u8 seq;
    u8 unknownb;
    __tibcaGUID guid;
    IPXPlayerEntry player;
};

struct __6eas9nIPXDATAMESSAGE {
    u32 magic;
    u32 from;
    u8 unknown8[2];
    u8 seq;
    u8 dseq;
    u8 data[0x212];
};

struct IPXAckMessage {
    u32 magic;
    u32 from;
    u8 unknown8[2];
    u8 seq;
    u8 unknownb;
};

struct __6gjpddIPXSPDATAMESSAGE {
    u8 data[0x216];
    u16 used;
    u32 from;
};

typedef __6gjpddIPXSPDATAMESSAGE IPXQueueEntry;

struct __407tigIPXMESSAGEBLOCK {
    u32 magic;
    u8 unknown4[8];
    __tibcaGUID guid;
};

struct IPXSessionEntry {
    u8 address[16];
    char name[0x20];
    s32 time;
};

struct IPXServiceProvider : ServiceProvider {
    IPXHandler ipx;
    IPXPacket *packets;
    IPXPlayerEntry *players;
    IPXSessionEntry session_list[32];
    __3ljac5NETSOCKET socket;
    TbNetworkSessionNameEntry entry;
    IPXSessionInfo session;
    __5303ctCLIENTINFOPACKET remote;
    __tibcaGUID guid;
    IPXQueueEntry *queue;
    __6eas9nIPXDATAMESSAGE out;
    u32 packets_sel;
    char server_name[0x1515 - 0x14e5];
    u16 session_count;
    u16 host_state;
    u8 sys_seq;
    u8 acked_a[0x20];
    u8 acked_b[0x20];
    u32 accepting;

    virtual ~IPXServiceProvider();
    TbError Initialise(__ic76sReceiveCallbacks *callbacks, void *data);
    TbError BullfrogIPXServer(char *name);
    TbError SendPlayerInfo(__5303ctCLIENTINFOPACKET *info, __3ljac5NETSOCKET *to);
    TbError AddPlayerToSession(__35ct5sCLIENTJOINPACKET *join, __3ljac5NETSOCKET *from);
    u32 CheckIfPlayerWantingToJoinIsInSession(__35ct5sCLIENTJOINPACKET *join, __3ljac5NETSOCKET *from);
    TbError EnumerateAndAddPlayers(__3ljac5NETSOCKET *server);
    TbError BroadcastSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg);
    TbError RemoveAbortedPlayer(void);
    TbError ResponsedToSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg, __3ljac5NETSOCKET *from);
    u32 CheckForSystemMessages(void);
    TbError SendMessageToPlayerId(u32 id, void *buf, u32 size);
    u32 HaveIGotAckDataMessage(u8 seq, u32 id);
    u32 ProcessDataMessage(__6eas9nIPXDATAMESSAGE *msg);
    inline void AddToReceiveBuffer(__6eas9nIPXDATAMESSAGE *msg);
    void BatchProcess(u8 delay);
    TbError Update(void);
    TbError Release(void);
    TbError Stop(void);
    TbError Start(char *session, char *name, u32 max_players, void *data);
    TbError Start(TbNetworkSessionNameEntry *entry, char *name, void *data);
    TbError Enumerate(TbNetworkCallbackFunc callback, void *data);
    TbError Enumerate(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data);
    __6gjpddIPXSPDATAMESSAGE *GetNextEmptyReceiveBuffer(void);
    TbError ChangeSettings(u32 setting, void *data);
    TbError EnableNewPlayers(u32 allow);
    TbError Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *callbacks, void *data);
    u32 ReadMessage(u32 *from, void *buf, u32 *size);
    u32 PeekMessage(u32 *from, void *buf, u32 *size);
    TbError SendMessage(u32 to, void *buf, TbMessageType type);
    void ConvertGuidToString(__tibcaGUID *guid, char *str);
    void ConvertStringToGuid(char *str, __tibcaGUID *guid);
    TbError AllocateSpStuff(void);
    void FreeAllocSpStuff(void);
    void ClearSpStuff(void);
    TbError BullfrogIPXMessage(__407tigIPXMESSAGEBLOCK *msg, __tibcaGUID *id);
    void ProcessSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg);
};

extern "C" {
u32 fn_000d6598(IPXServiceProvider *self, u32 *from, void *buf, u32 *size, u8 peek);
#pragma aux fn_000d6598 "W?PrivateReceive$:IPXServiceProvider$n(pnulpnvpnul$$TE$$)ul"
}

#endif
