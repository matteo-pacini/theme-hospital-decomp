#ifndef ENUMS_H
#define ENUMS_H

/*
 * Named values of game fields and parameters. The enumerators only replace
 * literals: fields, parameters and variables keep their integer types, since
 * Watcom sizes an enum type by its range and an enum-typed declaration would
 * change both the layout and the mangled names.
 */

// Thing.type: what a thing[] entry is.
enum {
    THING_FREE = 0,
    THING_PATIENT = 2,
    THING_STAFF = 4,
    THING_OBJECT = 6,
    THING_DOOR = 8,
    THING_REAPER = 0xa,
    THING_RAT = 0xc,
    THING_DEBRIS = 0xe,
    THING_HELICOPTER = 0x10,
    THING_NUMFX = 0x12,
    THING_MAYOR = 0x14,
    THING_INSPECTOR = 0x16,
    THING_SHADOW = 0x18
};

// Thing.subtype of a staff member: their kind (language section 0).
enum {
    STAFF_NURSE = 0,
    STAFF_DOCTOR = 1,
    STAFF_HANDYMAN = 2,
    STAFF_RECEPTIONIST = 3,
    // A doctor while working in the operating theatre.
    STAFF_SURGEON = 4
};

// Thing.skills bits of a staff member. Defines, not enumerators: Watcom
// tests a literal mask against the field's low byte (test byte ptr), but loads
// the whole field for an enumerator.
#define SKILL_RECEPTIONIST 1
#define SKILL_HANDYMAN 2
#define SKILL_NURSE 4
#define SKILL_JUNIOR 8
#define SKILL_DOCTOR 0x10
#define SKILL_SURGEON 0x20
#define SKILL_PSYCHIATRIST 0x40
#define SKILL_CONSULTANT 0x80
#define SKILL_RESEARCHER 0x100

// RoomInfo.type and rooms[] index: the kind of a room. rooms[].id is its
// name in language section 52 (13 for the build menu).
enum {
    ROOM_FREE = 0,
    // One per entrance of each hospital, made at level load from
    // level->entrances: where people come in and leave.
    ROOM_ENTRANCE = 1,
    ROOM_CORRIDOR = 3,
    ROOM_RECEPTION = 4,
    ROOM_DESTROYED = 6,
    ROOM_GP = 7,
    ROOM_PSYCH = 8,
    ROOM_WARD = 9,
    ROOM_THEATRE = 10,
    ROOM_PHARMACY = 11,
    ROOM_CARDIO = 12,
    ROOM_SCANNER = 13,
    ROOM_ULTRASCAN = 14,
    ROOM_BLOOD_MACHINE = 15,
    ROOM_XRAY = 16,
    ROOM_INFLATION = 17,
    ROOM_DNA_FIXER = 18,
    ROOM_HAIR_RESTORER = 19,
    ROOM_SLACK_TONGUE = 20,
    ROOM_FRACTURE = 21,
    ROOM_TRAINING = 22,
    ROOM_ELECTROLYSIS = 23,
    ROOM_JELLY_VAT = 24,
    ROOM_STAFF_ROOM = 25,
    ROOM_GENERAL_DIAG = 27,
    ROOM_RESEARCH = 28,
    ROOM_TOILETS = 29,
    ROOM_DECONTAMINATION = 30
};

// Thing.subtype of an object or door and objects[] index: the object's kind
// (its name is language section 1, same index).
enum {
    OBJ_DESK = 1,
    OBJ_CABINET = 2,
    OBJ_DOOR = 3,
    OBJ_BENCH = 4,
    OBJ_TABLE = 5,
    OBJ_CHAIR = 6,
    OBJ_DRINKS_MACHINE = 7,
    OBJ_BED = 8,
    OBJ_INFLATOR = 9,
    OBJ_POOL_TABLE = 10,
    OBJ_RECEPTION_DESK = 11,
    OBJ_TABLE2 = 12,
    OBJ_CARDIO = 13,
    OBJ_SCANNER = 14,
    OBJ_CONSOLE = 15,
    OBJ_SCREEN = 16,
    OBJ_LITTER_BOMB = 17,
    OBJ_COUCH = 18,
    OBJ_SOFA = 19,
    OBJ_CRASH_TROLLEY = 20,
    OBJ_TV = 21,
    OBJ_ULTRASCANNER = 22,
    OBJ_DNA_FIXER = 23,
    OBJ_CAST_REMOVER = 24,
    OBJ_HAIR_RESTORER = 25,
    OBJ_SLICER = 26,
    OBJ_XRAY = 27,
    OBJ_RADIATION_SHIELD = 28,
    OBJ_XRAY_VIEWER = 29,
    OBJ_OPERATING_TABLE = 30,
    OBJ_LAMP = 31,
    OBJ_SINK = 32,
    OBJ_SINK2 = 33,
    OBJ_SINK3 = 34,
    OBJ_SURGEON_SCREEN = 35,
    OBJ_LECTURE_CHAIR = 36,
    OBJ_PROJECTOR = 37,
    OBJ_BED2 = 38,
    OBJ_PHARMACY_CABINET = 39,
    OBJ_COMPUTER = 40,
    OBJ_ATOM_ANALYSER = 41,
    OBJ_BLOOD_MACHINE = 42,
    OBJ_FIRE_EXTINGUISHER = 43,
    OBJ_RADIATOR = 44,
    OBJ_PLANT = 45,
    OBJ_ELECTROLYSER = 46,
    OBJ_JELLY_MOULDER = 47,
    OBJ_GATES_OF_HELL = 48,
    OBJ_BED3 = 49,
    OBJ_BIN = 50,
    OBJ_TOILET = 51,
    OBJ_SWING_DOOR = 52,
    OBJ_SWING_DOOR2 = 53,
    OBJ_SHOWER = 54,
    OBJ_AUTO_AUTOPSY = 55,
    OBJ_BOOKCASE = 56,
    OBJ_VIDEO_GAME = 57,
    OBJ_ENTRANCE_LEFT_DOOR = 58,
    OBJ_ENTRANCE_RIGHT_DOOR = 59,
    OBJ_SKELETON = 60,
    OBJ_COMFY_CHAIR = 61
};

// Thing.state (and next_state, resume_state, saved_state) of a patient. Each
// is named after the routine process_things runs for it.
enum {
    PS_WAITS_FOR_BUSY = 4,
    PS_GOTO_DESTINATION = 6,
    PS_RETURN_TO_QUEUE_AREA = 8,
    PS_GO_HOME = 0xc,
    PS_GONE = 0xe,
    PS_PLAY_COMPLEX_ANIM = 0x10,
    PS_ARRIVES_AT_DOOR = 0x12,
    PS_IN_QUEUE = 0x14,
    PS_GOTO_OBJECT = 0x16,
    PS_ENTERTAINS_THEMSELF = 0x18,
    PS_WAIT_FOR_COMPLEX_SLAVE = 0x1a,
    // No routine: ready as the slave of a staff member's complex (examined,
    // treated), which staff_wait_for_complex_slave and merge_slave wait for.
    PS_SLAVE_READY = 0x20,
    PS_TWO_PEEPS_INTERACT = 0x22,
    PS_GOTO_QUEUE = 0x24,
    PS_GO_FOR_TREATMENT = 0x26,
    // No routine: drawn as part of the staff member whose animation it joined
    // (merge_slave).
    PS_MERGED = 0x28,
    PS_COMPLEX_RETRY = 0x2a,
    PS_IN_BED = 0x2c,
    PS_FILLS = 0x2e,
    PS_ENERGISES = 0x30,
    PS_DEAD = 0x32,
    PS_FLYS = 0x34,
    PS_IN_BOGS = 0x36,
    PS_LEAVE_TOILETS = 0x38,
    PS_SHOWANIM = 0x3a,
    PS_STUCK = 0x3c,
    PS_BP_GOTO = 0x40,
    // No routine: the grim reaper's victim, arrived at the gates of hell.
    PS_AT_HELL_GATE = 0x42,
    PS_WAITS_FOR_DECISION = 0x44,
    PS_ARRIVED_FROM_CALL = 0x46,
    PS_NAV_DEBUG = 0xf0
};

// Thing.state (and next_state, resume_state, saved_state) of a staff member,
// named after the routine process_things runs for it.
enum {
    SS_DOCTOR_IN_OFFICE = 6,
    SS_DOCTOR_WORKS_AT_DESK = 8,
    SS_RECEPTIONIST_WORKS = 0xa,
    SS_RECEPTIONIST_FINDS_RECEPTION = 0xc,
    SS_PLAY_COMPLEX_ANIM = 0xe,
    SS_SURGEON_WAITS = 0x10,
    SS_GOTO_OBJECT = 0x12,
    SS_DOCTOR_WAITS_FOR_PATIENT = 0x14,
    // No routine: a theatre assistant ready for the lead surgeon's complex.
    SS_ASSISTANT_READY = 0x18,
    SS_TWO_PEEPS_INTERACT = 0x1a,
    SS_DOCTOR_EXAMINES = 0x1c,
    SS_RECEPT_WAITS_FOR_PATIENT = 0x20,
    SS_RECEPT_ADMITS_PATIENT = 0x22,
    SS_DOCTOR_IN_MACHINE_ROOM = 0x24,
    SS_WAIT_FOR_COMPLEX_SLAVE = 0x26,
    SS_IN_CORRIDOR = 0x28,
    SS_COMPLEX_RETRY = 0x2c,
    // On the list of job applicants until hire_expiry.
    SS_APPLICANT = 0x2e,
    SS_IN_STAFF_ROOM = 0x30,
    SS_NURSE_IN_WARD = 0x32,
    SS_WANDERS = 0x34,
    SS_EVACUATE = 0x36,
    // Back on the list of job applicants after walking off the map (sacked or
    // resigned).
    SS_BACK_TO_APPLICANTS = 0x38,
    SS_HANDYMAN_WORKS = 0x3c,
    SS_DOCTOR_RESEARCHES = 0x3e,
    SS_NURSE_VACCINATES = 0x40,
    SS_DOCTOR_IN_THEATRE = 0x42,
    // No routine: off the map, picked up or being hired by the player.
    SS_HELD = 0x44,
    SS_DOCTOR_IS_TRAINED = 0x46,
    SS_CONSULTANT_TRAINS = 0x48,
    SS_RECEPTIONIST_SETUP = 0x4a,
    SS_BP_GOTO = 0x4c,
    SS_STUCK = 0x4e,
    SS_PLAY_ANISEQ = 0x50,
    SS_NAV_DEBUG = 0xf0
};

// Thing.state of an object or door, named after the routine process_things
// runs for it.
enum {
    OS_STATIC = 2,
    // In use by the person in its use position, whose animation plays the
    // object's overlay.
    OS_IN_USE = 4,
    OS_HELL = 6,
    OS_PLANT_AGES = 0xa,
    // No routine: a plant while a handyman waters it.
    OS_BEING_WATERED = 0xe,
    OS_DOUBLE_DOOR_SWINGS = 0x10,
    // No routine: taken off the map (remove_object_from_map).
    OS_REMOVED = 0x12,
    OS_SLIDING_DOOR_CLOSED = 0x14,
    OS_SLIDING_DOOR_OPENS = 0x16,
    OS_SLIDING_DOOR_OPEN = 0x18,
    OS_SLIDING_DOOR_CLOSES = 0x1a,
    OS_BOMB_TICKS = 0x1c
};

// Thing.state (and reaper_next_state) of the grim reaper.
enum { REAPER_PLAY_ANIM = 2, REAPER_WAITS = 4, REAPER_REMOVE = 6, REAPER_GOTO_HELL = 8 };

// Thing.state of a rat.
enum { RAT_SCURRIES = 2, RAT_ARRIVES = 4, RAT_LEAVES = 6 };

// Thing.state of a helicopter (or saucer).
enum {
    HELI_LANDS = 2,
    // No routine but the animation: landed, waiting for the emergency.
    HELI_WAITS = 4,
    HELI_LEAVES = 6,
    HELI_ABDUCTS = 8
};

// Thing.state (and goto_state) of the mayor and the inspector.
enum {
    MAYOR_GOTO = 2,
    MAYOR_PAUSES = 4,
    MAYOR_GONE = 6,
    // The inspector at the reception desk (inspector_pauses).
    MAYOR_INSPECTS = 8
};

// Thing.illness and other illness (expertise) indexes: an illness, or a
// diagnosis or research item past the illnesses. Names are language
// section 3, same index (0 "None", 1 "General Practice").
enum {
    ILL_GENERAL_PRACTICE = 1,
    ILL_BLOATY_HEAD = 2,
    ILL_HAIRYITIS = 3,
    ILL_KING_COMPLEX = 4,
    ILL_INVISIBILITY = 5,
    ILL_SERIOUS_RADIATION = 6,
    ILL_SLACK_TONGUE = 7,
    ILL_ALIEN_DNA = 8,
    ILL_FRACTURED_BONES = 9,
    ILL_BALDNESS = 10,
    ILL_DISCRETE_ITCHING = 11,
    ILL_JELLYITIS = 12,
    ILL_SLEEPING_ILLNESS = 13,
    ILL_PREGNANT = 14,
    ILL_TRANSPARENCY = 15,
    ILL_UNCOMMON_COLD = 16,
    ILL_BROKEN_WIND = 17,
    ILL_SPARE_RIBS = 18,
    ILL_KIDNEY_BEANS = 19,
    ILL_BROKEN_HEART = 20,
    ILL_RUPTURED_NODULES = 21,
    ILL_TV_PERSONALITIES = 22,
    ILL_INFECTIOUS_LAUGHTER = 23,
    ILL_CORRUGATED_ANKLES = 24,
    ILL_CHRONIC_NOSEHAIR = 25,
    ILL_THIRD_DEGREE_SIDEBURNS = 26,
    ILL_FAKE_BLOOD = 27,
    ILL_GASTRIC_EJECTIONS = 28,
    ILL_THE_SQUITS = 29,
    ILL_IRON_LUNGS = 30,
    ILL_SWEATY_PALMS = 31,
    ILL_HEAPED_PILES = 32,
    ILL_GUT_ROT = 33,
    ILL_GOLF_STONES = 34,
    ILL_UNEXPECTED_SWELLING = 35,
    ILL_DIAG_SCANNER = 36,
    ILL_DIAG_BLOOD_MACHINE = 37,
    ILL_DIAG_CARDIO = 38,
    ILL_DIAG_XRAY = 39,
    ILL_DIAG_ULTRASCAN = 40,
    ILL_DIAG_GENERAL = 41,
    ILL_DIAG_WARD = 42,
    ILL_DIAG_PSYCH = 43,
    ILL_AUTOPSY = 44,
    ILL_MIXER = 45
};

// Thing.flags32 status bits of a patient (also read a byte at a time as
// flags0, flags1 and flags2: shift right by 8 or 16). The info bar text
// (language section 58) names most of them. Defines, not enumerators, as for
// the skills.
#define PF_VACCINE_REQUESTED 0x1
// A nurse is on her way to vaccinate them.
#define PF_VACCINE_NURSE_SENT 0x2
// Drop vomit, or a puddle, when the current reaction animation ends.
#define PF_VOMITS 0x4
#define PF_WETS 0x8
#define PF_AWAITING_DECISION 0x10
#define PF_CONTAGIOUS 0x20
#define PF_EMERGENCY 0x40
// Waiting for the player to build a diagnosis or treatment room.
#define PF_WAITING_FOR_ROOM 0x80
#define PF_SENT_HOME_BY_INSPECTOR 0x100
// Set by an earthquake (disaster_peeps) for kinds that react; the next time
// they are outdoors they play the reaction animation and it clears.
#define PF_QUAKE_SHAKEN 0x200
// Diagnosed as far as the hospital can, but no illness found: waiting for
// more diagnosis rooms (info bar text 17 rather than 18).
#define PF_NEEDS_DIAGNOSIS_ROOM 0x400
// The guessed diagnosis is wrong (best_guess_treatment): the treatment does
// nothing, or kills.
#define PF_WRONG_GUESS 0x800
#define PF_GUESSED 0x1000
#define PF_CURED 0x2000
#define PF_FED_UP 0x4000
#define PF_SENT_HOME 0x8000
#define PF_REFERRED 0x10000
#define PF_DIAGNOSED 0x20000
#define PF_DYING 0x40000
#define PF_NO_DIAGNOSIS 0x80000
#define PF_NO_TREATMENT 0x100000
// Counted once in this month's visitor history (history_visitors_now).
#define PF_COUNTED_VISITOR 0x200000
// Counted once in the hospital's patient statistics.
#define PF_RECORDED 0x400000
#define PF_TOO_EXPENSIVE 0x800000

// Thing.orders bits of a staff member. The low nibble holds the player's
// order from the staff window. Defines, as for the skills.
#define ORD_REST 0x1
#define ORD_TRAIN 0x4
// Leave the current room (evacuate_staff), then clear the order.
#define ORD_LEAVE_ROOM 0x8
// Set while researching or being trained, so that they go back after a rest.
#define ORD_RETURN_TO_RESEARCH 0x10
#define ORD_RESIGN 0x20
#define ORD_RETURN_TO_TRAINING 0x40

// Key codes: lbKeyOn index, lbInkey and the key parameters of the keyboard
// handlers. PC set 1 scan codes, with 0x80 added for the 0xe0-prefixed keys
// (myKInt).
enum {
    KC_ESCAPE = 1,
    KC_1 = 2,
    KC_2 = 3,
    KC_3 = 4,
    KC_4 = 5,
    KC_5 = 6,
    KC_6 = 7,
    KC_7 = 8,
    KC_8 = 9,
    KC_9 = 0xa,
    KC_0 = 0xb,
    KC_BACKSPACE = 0xe,
    KC_TAB = 0xf,
    KC_Q = 0x10,
    KC_W = 0x11,
    KC_R = 0x13,
    KC_T = 0x14,
    KC_Y = 0x15,
    KC_P = 0x19,
    KC_RETURN = 0x1c,
    KC_LCONTROL = 0x1d,
    KC_A = 0x1e,
    KC_S = 0x1f,
    KC_D = 0x20,
    KC_F = 0x21,
    KC_H = 0x23,
    KC_J = 0x24,
    KC_L = 0x26,
    KC_SEMICOLON = 0x27,
    KC_LSHIFT = 0x2a,
    KC_Z = 0x2c,
    KC_X = 0x2d,
    KC_C = 0x2e,
    KC_M = 0x32,
    KC_COMMA = 0x33,
    KC_RSHIFT = 0x36,
    KC_LALT = 0x38,
    KC_SPACE = 0x39,
    KC_F1 = 0x3b,
    KC_F2 = 0x3c,
    KC_F3 = 0x3d,
    KC_F4 = 0x3e,
    KC_F5 = 0x3f,
    KC_F6 = 0x40,
    KC_F7 = 0x41,
    KC_F8 = 0x42,
    KC_NUMPAD8 = 0x48,
    KC_NUMPAD4 = 0x4b,
    KC_NUMPAD6 = 0x4d,
    KC_NUMPAD2 = 0x50,
    KC_NUMPAD0 = 0x52,
    KC_DECIMAL = 0x53,
    KC_NUMPADENTER = 0x9c,
    KC_RCONTROL = 0x9d,
    KC_DIVIDE = 0xb5,
    KC_RALT = 0xb8,
    KC_UP = 0xc8,
    KC_LEFT = 0xcb,
    KC_RIGHT = 0xcd,
    KC_DOWN = 0xd0
};

// HospShift bits (the modifiers parameter of the key handlers).
#define MOD_SHIFT 1
#define MOD_CTRL 2
#define MOD_ALT 4

// play_sound's sfx parameter (sfx_details index). Each is named after the
// sample it plays, its file name in the SOUND-0.DAT directory; SFX_STOP_ ones
// stop that sample.
enum {
    SFX_HLIGHTP2 = 0x1,
    SFX_SCANMTR = 0x2,
    SFX_SCANHUM = 0x3,
    SFX_EXPLODE = 0x4,
    SFX_SCANDOWN = 0x5,
    SFX_SCANLVR = 0x6,
    SFX_PLAC_ST2 = 0x7,
    SFX_WRONG2 = 0x8,
    SFX_TYPING = 0x9,
    SFX_FALLOVER = 0xa,
    SFX_WETTING3 = 0xb,
    SFX_ENTANAME = 0xc,
    SFX_REAPERGO = 0xd,
    SFX_NEWSTURN = 0xe,
    SFX_SWEEP = 0xf,
    SFX_DELANAME = 0x10,
    SFX_DOOROPE3 = 0x11,
    SFX_DOORCLO2 = 0x12,
    SFX_OCABSHRT = 0x13,
    SFX_CCABSHRT = 0x14,
    SFX_SCLICK = 0x15,
    SFX_SQUEAK1 = 0x16,
    SFX_JELFLOOR = 0x17,
    SFX_KNOCK = 0x18,
    SFX_SCRATCH = 0x19,
    SFX_POUR = 0x1a,
    SFX_PHCABO = 0x1b,
    SFX_PHCABCL = 0x1c,
    SFX_PHTAP = 0x1d,
    SFX_BUILDCLK = 0x1e,
    SFX_SIT = 0x1f,
    SFX_GETUP = 0x20,
    SFX_SWALLOW = 0x21,
    SFX_SWOOSH = 0x22,
    SFX_SCNBEEP1 = 0x23,
    SFX_JELLOPEN = 0x24,
    SFX_JELLCLOS = 0x25,
    SFX_COINSLT2 = 0x26,
    SFX_CANDROP = 0x27,
    SFX_SCNBEEP2 = 0x28,
    SFX_SCNBEEP3 = 0x29,
    SFX_TONGUE1 = 0x2a,
    SFX_TONGCHOP = 0x2b,
    SFX_TFLUSH = 0x2c,
    SFX_POOPLOP3 = 0x2d,
    SFX_NEWSFOLD = 0x2e,
    SFX_JELLPOUR = 0x2f,
    SFX_BONESUP = 0x30,
    SFX_XRAY2 = 0x31,
    SFX_WATER = 0x32,
    SFX_POP = 0x33,
    SFX_SPARK = 0x34,
    SFX_SSPARK = 0x35,
    SFX_YAWN1 = 0x36,
    SFX_GETHEAD = 0x37,
    SFX_HEADFILL = 0x38,
    SFX_WEE = 0x39,
    SFX_PUKE = 0x3a,
    SFX_VOMSPLAT = 0x3b,
    SFX_TOILETO = 0x3c,
    SFX_TOILETC = 0x3d,
    SFX_OPENFLAP = 0x3e,
    SFX_SHOTGUN = 0x3f,
    SFX_DEADRAT2 = 0x40,
    SFX_YESX = 0x41,
    SFX_NO4 = 0x42,
    SFX_SELECTX = 0x43,
    SFX_HLIGHT5 = 0x44,
    SFX_CASHREG = 0x45,
    SFX_PLACE_R = 0x46,
    SFX_UPDOWN = 0x47,
    SFX_QUAKE2 = 0x48,
    SFX_CHALKCUE = 0x49,
    SFX_POOLSHT2 = 0x4a,
    SFX_BALLHIT2 = 0x4b,
    SFX_BALLHIT = 0x4c,
    SFX_GLASSTAP = 0x4d,
    SFX_SCANMTR2 = 0x4e,
    SFX_ELEDOOR2 = 0x4f,
    SFX_CHOPPER2 = 0x50,
    SFX_HAMMER = 0x51,
    SFX_FOOTTAP = 0x52,
    SFX_VGM001 = 0x55,
    SFX_VGM002 = 0x56,
    SFX_IGNITE2 = 0x57,
    SFX_TORCH2 = 0x58,
    SFX_PROJDOWN = 0x59,
    SFX_PROJUP = 0x5a,
    SFX_ADDITEMJ = 0x5b,
    SFX_DELITEMJ = 0x5c,
    SFX_REAPCOME = 0x5d,
    SFX_WATER4 = 0x5e,
    SFX_TROLLEY = 0x5f,
    SFX_FLAP_1 = 0x60,
    SFX_FLAP_2 = 0x61,
    SFX_FLAP_3 = 0x62,
    SFX_LAVA001 = 0x63,
    SFX_LAVA002 = 0x64,
    SFX_AUT_OUT = 0x65,
    SFX_AUT_OPEN = 0x66,
    SFX_AUT_ROCK = 0x67,
    SFX_AUT_KLNK = 0x68,
    SFX_BALLCUSH = 0x69,
    SFX_BM_SQSH = 0x6a,
    SFX_BM_PUMP = 0x6b,
    SFX_BM_TV = 0x6c,
    SFX_BM_OPEN2 = 0x6d,
    SFX_BM_CHAIR = 0x6e,
    SFX_MORPH = 0x6f,
    SFX_CNTDOWN = 0x70,
    SFX_SHUTFLAP = 0x71,
    SFX_PUKE2 = 0x72,
    SFX_APPEAR = 0x73,
    SFX_FRACSLAM = 0x74,
    SFX_RESEARCH = 0x75,
    SFX_FARTONE = 0x76,
    SFX_FARTTWO = 0x77,
    SFX_FARTTHRE = 0x78,
    SFX_FARTFOUR = 0x79,
    SFX_FARTFIVE = 0x7a,
    SFX_ANGEL = 0x7b,
    SFX_REAPSWIP = 0x7c,
    SFX_BM_LEVER = 0x7d,
    SFX_CLOTHES = 0x7e,
    SFX_FRAC_MTR = 0x7f,
    SFX_ALIENSHP = 0x80,
    SFX_BUCKUP = 0x81,
    SFX_BUCKET = 0x82,
    SFX_FRACTAP2 = 0x83,
    SFX_PROJON = 0x84,
    SFX_CARDIO1 = 0x85,
    SFX_CARDIO2 = 0x86,
    SFX_ULTRAHUM = 0x87,
    SFX_SHAKHNDS = 0x88,
    SFX_TAPTURN = 0x89,
    SFX_VGM003 = 0x8a,
    SFX_VGM004 = 0x8b,
    SFX_VGM005 = 0x8c,
    SFX_VGMEND = 0x8d,
    SFX_HAIRMTR = 0x8e,
    SFX_JELLBUZZ = 0x8f,
    SFX_SLICESQ1 = 0x90,
    SFX_SLICESQ2 = 0x91,
    SFX_PROJPILE = 0x92,
    SFX_PROJSLID = 0x93,
    SFX_PROJTAP = 0x94,
    SFX_SMFLAP = 0x95,
    SFX_OPARM = 0x96,
    SFX_DE_BUILD = 0x97,
    SFX_BALLCUS2 = 0x98,
    SFX_PICKUP = 0x99,
    SFX_DECONTAM = 0x9b,
    SFX_BOOKPAGE = 0x9d,
    SFX_BOOKBACK = 0x9f,
    SFX_LAVAFALL = 0xa0,
    SFX_BRE_IN = 0xa1,
    SFX_BRE_OUT = 0xa2,
    SFX_THERM_IN = 0xa3,
    SFX_SM_OBJ = 0xa4,
    SFX_AUCTION = 0xa5,
    SFX_POOTHUD = 0xa6,
    SFX_PAGETUR2 = 0xa8,
    SFX_TICKTOCK = 0xa9,
    SFX_SACK = 0xaa,
    SFX_FAX_IN = 0xab,
    SFX_FAX_YES = 0xac,
    SFX_FAX_NO = 0xad,
    SFX_BOO = 0xae,
    SFX_CHEER = 0xaf,
    SFX_BONUSAL2 = 0xb4,
    SFX_LAVA003 = 0xb5,
    SFX_LAVA004 = 0xb6,
    SFX_LAVA005 = 0xb7,
    SFX_BONUS2 = 0xb8,
    SFX_BELL = 0xb9,
    SFX_PICKUPSM = 0xba,
    SFX_CAMCLICK = 0xbb,
    SFX_FAX_0 = 0xbe,
    SFX_FAX_1 = 0xbf,
    SFX_FAX_2 = 0xc0,
    SFX_FAX_3 = 0xc1,
    SFX_FAX_4 = 0xc2,
    SFX_FAX_5 = 0xc3,
    SFX_FAX_6 = 0xc4,
    SFX_FAX_7 = 0xc5,
    SFX_FAX_8 = 0xc6,
    SFX_FAX_9 = 0xc7,
    SFX_ATMOS = 0xc8,
    SFX_MACHWARN = 0xd0,
    SFX_FUSE = 0xd1,
    SFX_DICEYFIN = 0xd2,
    SFX_UZIBRST = 0xd3,
    SFX_ROCKET = 0xd4,
    SFX_BOMB = 0xd5,
    SFX_DNA = 0xd8,
    SFX_FANFARE = 0xd9,
    SFX_SNAPSHOT = 0xda,
    SFX_DICE122M = 0xdc,
    SFX_NEWFAX = 0xdd,
    SFX_VACCIN = 0xde,
    SFX_DEVACCIN = 0xdf,
    SFX_INUSE006 = 0xe3,
    SFX_STOP_DNA = 0xe8,
    SFX_STOP_TORCH2 = 0xec,
    SFX_STOP_WATER4 = 0xed
};

// change_balance's type: the kind of transaction, as listed in the bank
// statement (language section 7, same index).
enum {
    TX_WAGES = 1,
    TX_HIRE_STAFF = 2,
    TX_BUY_OBJECT = 3,
    TX_BUILD_ROOM = 4,
    TX_CURE = 5,
    TX_BUY_LAND = 6,
    TX_TREAT = 7,
    TX_FINAL_TREAT = 8,
    TX_CURE_TREATMENT = 9,
    TX_TREATMENT_DEPOSIT = 10,
    TX_ADVANCE = 11,
    TX_RESEARCH_COSTS = 12,
    TX_INCOME_DRINKS = 13,
    TX_INCOME_JUKEBOX = 14,
    TX_CHEAT_MONEY = 15,
    TX_HEATING = 16,
    TX_INSURANCE = 17,
    TX_BANK_LOAN = 18,
    TX_LOAN_REPAYMENT = 19,
    TX_LOAN_INTEREST = 20,
    TX_RESEARCH_BONUS = 21,
    TX_DRUG_COST = 22,
    TX_OVERDRAFT_INTEREST = 23,
    TX_SEVERANCE_PAY = 24,
    TX_GENERAL_BONUS = 25,
    TX_SELL_OBJECT = 26,
    TX_PERSONAL_BONUS = 27,
    TX_EMERGENCY_BONUS = 28,
    TX_VACCINATION = 29,
    TX_EPIDEMIC_COVERUP_FINE = 30,
    TX_GOVERNMENT_COMPENSATION = 31,
    TX_VIP_AWARD = 32,
    TX_EPIDEMIC_FINE = 33,
    TX_YEAR_END_BONUS = 34,
    TX_TROPHY_BONUS = 35,
    TX_REPLACEMENT_MACHINE = 36
};

// Event.type: a player's command for the turn (player_events), carried out by
// process_turn. Named after the routine that carries it out; ORDER_ ones set
// a staff member's orders.
enum {
    EV_NONE = 0,
    EV_OPEN_ROOM = 1,
    EV_MODIFY_BLUEPRINT = 2,
    EV_PLACE_BP_DOOR = 3,
    EV_COMMIT_DOORS = 4,
    EV_PLACE_STAFF = 5,
    EV_PLACE_OBJECT = 7,
    // Updates the blueprint under the mouse (modify_blueprint, not dragging).
    EV_HOVER_BLUEPRINT = 8,
    EV_QUEUE_PERSON_TO_NEW_QUEUE = 9,
    // Answers a pending call to the player (value: the call's kind).
    EV_ANSWER_PLAYERCALL = 0xa,
    EV_PLACE_SMALL_THING = 0xb,
    EV_SAVE_GAME = 0xc,
    EV_LOAD_GAME = 0xd,
    EV_VARY_SPEED = 0xe,
    EV_COMMIT_BLUEPRINT = 0x12,
    EV_UNCOMMIT_BLUEPRINT = 0x13,
    EV_TEMP_PLACE_DOOR = 0x1a,
    EV_QUIT_GAME = 0x1c,
    EV_GOTO_COLUMN_ROW = 0x1d,
    // Loads save value when a level starts from a saved game, then centres on an entrance.
    EV_LOAD_GAME_AT_START = 0x1e,
    EV_REQ1_RESET_ROOMS = 0x20,
    // Pauses or unpauses the game.
    EV_PAUSE = 0x22,
    EV_BUY_OBJECT = 0x25,
    EV_CLOSE_REQUESTERS = 0x26,
    EV_FADE_TO_BLACK = 0x29,
    EV_FADE_IN = 0x2a,
    EV_PLACE_NEXT_OBJECT = 0x2b,
    EV_OPEN_HIRE_STAFF = 0x2e,
    // Hire staff window: shows applicants of staff kind value.
    EV_HIRE_SELECT_KIND = 0x2f,
    EV_MARK_EMPLOYING = 0x30,
    // Hire staff window: next (value 1) or previous applicant.
    EV_HIRE_NEXT_APPLICANT = 0x32,
    // Opens a full-screen window (value: chart_type).
    EV_OPEN_SCREEN = 0x33,
    EV_CLOSE_SCREEN = 0x34,
    EV_INIT_BLUEPRINT = 0x35,
    EV_BUY_LAND = 0x36,
    EV_OPEN_BUILD_ROOM = 0x37,
    // Drops what is held and closes the build and placing requesters.
    EV_CANCEL_BUILD = 0x38,
    EV_OPEN_QUEUE_WINDOW = 0x3b,
    EV_FRONT_QUEUE = 0x3c,
    EV_BACK_QUEUE = 0x3d,
    // Opens the patient, staff or object window for thing value.
    EV_OPEN_THING_WINDOW = 0x42,
    EV_CLOSE_QUERY = 0x43,
    EV_PICK_UP_OBJECT = 0x44,
    EV_EDIT_BLUEPRINT = 0x45,
    EV_INIT_EDIT_ROOM = 0x46,
    EV_SACK_STAFF = 0x47,
    EV_PAY_STAFF_BONUS = 0x48,
    EV_OPEN_JUKEBOX = 0x49,
    // Starts the countdown of an accepted emergency.
    EV_START_EMERGENCY = 0x4a,
    EV_NET_TEXT_DISPLAY = 0x4b,
    EV_NET_TEXT_DELETE = 0x4c,
    EV_NET_TEXT_KEYPRESS = 0x4d,
    EV_NET_TEXT_START = 0x4e,
    EV_SET_HEATING = 0x4f,
    EV_ALTER_RESEARCH = 0x50,
    EV_QUEUE_PERSON_GO_HOME = 0x51,
    EV_QUEUE_CAPACITY_UP = 0x53,
    EV_QUEUE_CAPACITY_DOWN = 0x54,
    EV_QUEUE_PERSON_GO_RECEPTION = 0x55,
    EV_STAFF_JOIN_NEW_EMPLOYER = 0x56,
    // Multiplayer: refers a queued patient to another player's hospital.
    EV_REFER_TO_PLAYER = 0x57,
    EV_SETUP_TOWN_BID = 0x58,
    EV_PLOT_BID_MONEY = 0x59,
    EV_INCREASE_STAFF_SALARIES = 0x5a,
    EV_FURNISH_EDITED_ROOM = 0x5b,
    EV_BACK_TO_PLACE_OBJECT = 0x5c,
    EV_ORDER_REST = 0x5e,
    EV_ORDER_TRAIN = 0x5f,
    EV_ORDER_LEAVE_ROOM = 0x61,
    EV_OPEN_FURNISH_CORRIDOR = 0x62,
    EV_SELL_OBJECT = 0x63,
    EV_CANCEL_BUY_OBJECTS = 0x64,
    EV_CHEAT_END_MONTH = 0x65,
    EV_TAKE_LOAN = 0x66,
    EV_REPAY_LOAN = 0x67,
    EV_PICK_UP_STAFF = 0x68,
    EV_STAFF_PICKED_UP = 0x69,
    EV_SHOOT_RAT = 0x6a,
    // A resigning staff member's demand refused: they may still leave.
    EV_REFUSE_STAFF_DEMAND = 0x6b,
    EV_RAISE_STAFF_SALARY = 0x6c,
    EV_GIVE_STAFF_BONUS = 0x6d,
    EV_PRICE_UP = 0x6e,
    EV_PRICE_DOWN = 0x6f,
    EV_RETURN_FROM_SCREEN = 0x70,
    // Sets a policy slider (value: which, arg: setting).
    EV_SET_POLICY = 0x71,
    EV_CONCENTRATE_RESEARCH = 0x72,
    EV_ADJUST_HANDYMAN_VALUES = 0x73,
    EV_PERSON_GO_HOME = 0x74,
    EV_THINGPAGE_REQUEST = 0x76,
    EV_OPEN_YES_NO = 0x77,
    EV_CLOSE_YES_NO = 0x78,
    EV_DELETE_ROOM = 0x79,
    EV_CANCEL_EDIT_ROOM = 0x7b,
    EV_REFER_PATIENT = 0x7c,
    // A fax answer (value: CH_*, arg: choice_spare_data).
    EV_CHOICE_RESULT = 0x7d,
    // Value 1: the player leaves a network game, losing.
    EV_LEAVE_NET_GAME = 0x7e,
    EV_PATIENT_WAITS_FOR_ROOM = 0x7f,
    // Sends the patient to research (treatment ILL_AUTOPSY).
    EV_PATIENT_TO_RESEARCH = 0x80,
    EV_POACH_STAFF = 0x81,
    EV_TOGGLE_VACCINATION = 0x82,
    EV_SETUP_PLACE_OBJECT_REQUESTER = 0x83,
    EV_CLOSE_QUEUE_WINDOW = 0x84,
    EV_FORCE_CURE = 0x86,
    // Leaves a full-screen window and opens the window for thing arg.
    EV_RETURN_TO_THING_WINDOW = 0x87,
    EV_CANCEL_SACK = 0x88,
    EV_CHEAT_END_YEAR = 0x89,
    EV_REPLACE_MACHINE = 0x8a,
    // Ends the opening countdown (the panel watch clicked): the hospital opens
    // now.
    EV_OPEN_HOSPITAL = 0x8b,
    EV_CLOSE_SAVE_YES_NO = 0x8c,
    EV_NETWORK_LOST_FOCUS = 0x8d,
    EV_NETWORK_GAINED_FOCUS = 0x8e,
    EV_NET_WIN = 0x90,
    EV_RESTART_LEVEL = 0x91
};

// set_icon_value and reset_icon_value: the status icon shown over a person
// or machine (Thing.icon_id). HEALTH_1 to HEALTH_5 show falling health
// (change_health); GUESSED, DIAGNOSED and NO_DIAGNOSIS go over the doctor
// after a diagnosis; CALLING over staff calling a patient; QUEUE over the
// members of a flagged queue; NEEDS_REPAIR over a machine paged for repair.
enum {
    ICON_HEALTH_1 = 1,
    ICON_HEALTH_2 = 2,
    ICON_HEALTH_3 = 3,
    ICON_HEALTH_4 = 4,
    ICON_HEALTH_5 = 5,
    ICON_EMERGENCY = 6,
    ICON_FED_UP = 7,
    ICON_UNHAPPY = 8,
    ICON_LEAVING = 9,
    ICON_TIRED = 0xa,
    ICON_TOILET = 0xb,
    ICON_THIRSTY = 0xc,
    ICON_CONTAGIOUS = 0xd,
    ICON_VACCINE_REQUESTED = 0xe,
    ICON_VACCINE_NURSE_SENT = 0xf,
    ICON_VACCINATED = 0x10,
    ICON_TOO_HOT = 0x11,
    ICON_TOO_COLD = 0x12,
    ICON_TOO_EXPENSIVE = 0x13,
    ICON_GUESSED = 0x14,
    ICON_DIAGNOSED = 0x15,
    ICON_KILLED = 0x16,
    ICON_CURED = 0x17,
    ICON_NO_DIAGNOSIS = 0x18,
    ICON_CALLING = 0x19,
    ICON_QUEUE = 0x1a,
    ICON_AWAITING_DECISION = 0x1b,
    ICON_NEEDS_REPAIR = 0x1c
};

// EV_CHOICE_RESULT's value: the fax button the player answered with, from
// the fax's button handler (process_*_buttons).
enum {
    // No treatment available: send home, wait, or send to research.
    CH_TREAT_SEND_HOME = 0,
    CH_TREAT_WAIT = 1,
    CH_TREAT_RESEARCH = 2,
    // Epidemic: declare it (pay the fine) or try to cover it up.
    CH_EPIDEMIC_DECLARE = 0xa,
    CH_EPIDEMIC_COVER_UP = 0xb,
    CH_VIP_ACCEPT = 0x14,
    CH_VIP_REFUSE = 0x15,
    CH_EMERGENCY_REFUSE = 0x1e,
    CH_EMERGENCY_ACCEPT = 0x1f,
    // No diagnosis: send home, treat the best guess, or wait.
    CH_DIAG_SEND_HOME = 0x28,
    CH_DIAG_GUESS = 0x29,
    CH_DIAG_WAIT = 0x2a
};

// reputation_affected's event (and MayorPoints.reputation_event), named
// after its caller. The comment gives what it adds to the reputation totals;
// % is of allhosps.rep_events.
enum {
    // +1 (effect_illness).
    REP_CURED = 0,
    REP_GO_FOR_TREATMENT = 2,
    // -1 (person_dies).
    REP_DIED = 3,
    REP_DIAGNOSED = 4,
    // -5%: diagnosis given up, the patient killed or sent for autopsy.
    REP_DIAGNOSIS_FAILED = 5,
    // +1: a computer hospital's patient diagnosed.
    REP_COMPUTER_DIAGNOSED = 6,
    REP_NO_TREATMENT_FOUND = 7,
    // +5% (item_researched).
    REP_RESEARCHED = 9,
    REP_NEXT_TREATMENT = 10,
    // -10% (machine_disaster).
    REP_MACHINE_EXPLODED = 11,
    // +2%.
    REP_EMERGENCY_SUCCESS = 12,
    // -(4 * amount)%, amount being the patients not cured.
    REP_EMERGENCY_FAILED = 13,
    // -2%: an epidemic declared and its fine paid.
    REP_EPIDEMIC_DECLARED = 14,
    // -10%: the inspector found a covered-up epidemic.
    REP_EPIDEMIC_COVER_UP_FOUND = 15,
    // VIP verdicts (mayor_result): +4%, +2%, -4%.
    REP_VIP_GOOD = 16,
    REP_VIP_FAIR = 17,
    REP_VIP_BAD = 18,
    // -20% (person_autopsied).
    REP_AUTOPSY = 19,
    // +1: a patient's real illness diagnosed (diagnose_patient).
    REP_ILLNESS_FOUND = 21,
    // Counts one case in rep_events (record_person_as_patient).
    REP_CASE = 22,
    // +amount% (set_trophy_value, set_award_value).
    REP_TROPHY = 23,
    REP_AWARD = 24
};

// Thing.subtype of a patient (anim_starts index) while infected with alien
// DNA; Thing.orig_subtype keeps the kind to restore.
enum { PK_ALIEN = 0xe };

// mainmenu.state (draw_main_menu). 0x11-0x16 animate the highlight moving
// between two entries.
enum {
    MMS_ENTER = 0,
    // Highlighting an entry (tri_test).
    MMS_HOVER_CONTINUE = 3,
    MMS_HOVER_LOAD = 4,
    MMS_HOVER_NEW_GAME = 5,
    MMS_HOVER_NETWORK = 6,
    MMS_HOVER_QUIT = 7,
    // An entry chosen (process_mouse_main_menu).
    MMS_NEW_GAME = 8,
    // Continues from the autosave (slot 0x63).
    MMS_CONTINUE = 9,
    MMS_NETWORK = 0xa,
    MMS_QUIT = 0xc,
    MMS_LOAD_CONTINUE = 0xd,
    // Loads the menu screen and plays its intro.
    MMS_SHOW_MENU = 0xe,
    MMS_LOAD_GAME = 0xf,
    MMS_INTRO = 0x17,
    MMS_TITLE = 0x18,
    MMS_ATTRACT = 0x19,
    MMS_TO_NICKNAME = 0x1a,
    MMS_CONTINUE_FAILED = 0x1b,
    MMS_CREDITS = 0x1c,
    MMS_CREDITS_SCROLL = 0x1d,
    MMS_CREDITS_DONE = 0x1e,
    MMS_UNLOAD_GAME = 0x1f
};

// Map.flags: set by adjust_big_cell_flag while the tile holds a multi-tile
// object or a person using an object, so drawing works out what covers what.
#define MAP_BIG_CELL 0x20
// Map.owner_flags: an outdoor tile (the low bits are 1 << owner).
#define MAP_OUTDOOR 0x10

// RoomInfo.state values past 0 (in use) and 1; negative is a free slot.
enum {
    // Still a blueprint being placed.
    RS_BLUEPRINT = 3,
    // Being edited by the player (init_edit_room); staff leave it.
    RS_EDITED = 4,
    // A handyman answered the room's repair page.
    RS_REPAIR_DUE = 5,
    // Closed to patients: under repair with no doctor inside, or destroyed
    // (machine_disaster).
    RS_CLOSED = 6
};

// announce_this's type (gamevar.announce index): an announcement the advisor
// makes once (advisor_text, with the value as its argument where it has one).
enum {
    // Someone has got stuck.
    ANN_STUCK = 1,
    ANN_AUTOPSY_DISCOVERED = 2,
    // A research category (value) is complete.
    ANN_CATEGORY_DONE = 3,
    // A doctor gained the qualification whose skill bit index is value.
    ANN_PROMOTED = 4,
    // Object value has been researched.
    ANN_OBJECT_RESEARCHED = 5,
    // A drug for illness value has been researched.
    ANN_NEW_DRUG = 6,
    ANN_DRUG_IMPROVED = 7,
    ANN_OBJECT_IMPROVED = 8,
    ANN_OBJECT_AVAILABLE = 9,
    ANN_JUNIOR_TO_DOCTOR = 0xa,
    ANN_STAFF_POACHED = 0xb,
    // Hospital value opened in the area.
    ANN_RIVAL_OPENED = 0xc,
    ANN_DOCTOR_TO_CONSULTANT = 0xd
};

// Research categories: AllHosps.research_alloc index and do_research's
// category (names: language section 16, same index).
enum { RC_CURE = 0, RC_DIAGNOSIS = 1, RC_DRUGS = 2, RC_IMPROVEMENTS = 3, RC_SPECIALISATION = 4 };

// set_mouse_mode's mode (kept in gamevar.mouse_mode): what a click on the map does.
enum {
    MM_NORMAL = 0,
    // Dragging out a room's blueprint.
    MM_BLUEPRINT = 1,
    // Placing a blueprint's doors (place_bp_door, temp_place_door).
    MM_PLACE_DOOR = 2,
    MM_PLACE_STAFF = 3,
    MM_PLACE_OBJECT = 5,
    MM_PLACE_SMALL_THING = 6,
    // Full-screen menus and screens (pointer sprite 0).
    MM_SCREEN = 9,
    MM_TOWN_MAP = 0xa,
    // Picking up an object of the room being edited.
    MM_PICK_UP_OBJECT = 0xb,
    MM_EDIT_ROOM = 0xc,
    MM_NAV_DEBUG = 0xd,
    // Waiting for the room being edited to empty (right click cancels).
    MM_EDIT_ROOM_WAIT = 0xe,
    // Choosing patients to vaccinate.
    MM_VACCINATE = 0xf,
    // Moving a patient from the queue window to another room's queue.
    MM_MOVE_QUEUE_PERSON = 0x12,
    MM_MAIN_MENU = 0x14,
    // Playing a level's briefing video from the area map.
    MM_AREA_SMK = 0x15,
    MM_STAFF_LIST = 0x21,
    MM_HELP = 0x25,
    MM_WINLOSE = 0x2a
};

// Thing.numfx_type and create_numfx's type: how a floating number is drawn
// (mapwho_numfx).
enum { NUMFX_NUMBER = 0, NUMFX_TEXT = 1, NUMFX_FORMATTED = 2, NUMFX_MONEY = 3 };

// hosp_play_smk's idx: animation_table index, named after the video file.
// Entries 7 to 0x15 are the level briefings (area01v.smk...), played as
// level + 7.
enum {
    ANIM_LOSE1 = 0,
    ANIM_LOSE2 = 1,
    ANIM_LOSE3 = 2,
    ANIM_LOSE4 = 3,
    ANIM_LOSE5 = 4,
    ANIM_LOSE6 = 5,
    ANIM_WIN_LEVEL = 6,
    ANIM_MAIN_MENU1 = 0x16,
    ANIM_MAIN_MENU2 = 0x17,
    ANIM_WIN_GAME = 0x18,
    // The intro for slower CD drives (intro.sm2) and for faster ones (intro.sm4).
    ANIM_INTRO_SM2 = 0x19,
    ANIM_INTRO_SM4 = 0x1a,
    ANIM_ATTRACT = 0x1b
};

// set_mouse_sprite's idx and othervar.pointer_sprite: a sprite of the
// in-game pointer set (pointer[]).
enum {
    PTR_DEFAULT = 1,
    // Dragging out a blueprint, outside the current one.
    PTR_RESIZE_ROOM = 3,
    PTR_EDIT_ROOM = 4,
    // Over a blueprint's y0 or y1 edge, its x0 or x1 edge, or a corner;
    // inside it.
    PTR_RESIZE_Y_EDGE = 5,
    PTR_RESIZE_X_EDGE = 6,
    PTR_RESIZE_CORNER = 7,
    PTR_MOVE_ROOM = 8,
    // Shown while a network player has lost focus.
    PTR_SLEEP = 9,
    // Near a rat, and on one.
    PTR_RAT_NEAR = 0xa,
    PTR_RAT_AIM = 0xb,
    // Vaccinating: no patient to vaccinate under the pointer, and one.
    PTR_VACCINATE = 0xc,
    PTR_VACCINATE_TARGET = 0xd,
    PTR_STAFF = 0x10,
    // Over an object that has its own window (a machine).
    PTR_MACHINE = 0x11,
    PTR_PATIENT = 0x12,
    // Over a reception desk in use or a room's door: opens the queue.
    PTR_QUEUE = 0x13
};

// play_sound's sfx parameter, for sound effects whose sample is NULL.WAV
// (silent); named after where the game plays them.
enum {
    // Opening a staff member's demand requester.
    SFX_STAFF_DEMAND = 0x9a,
    // A zoom box opening or closing.
    SFX_ZOOM_OPEN = 0x9c,
    SFX_ZOOM_CLOSE = 0x9e,
    SFX_EPIDEMIC_START = 0xcd,
    SFX_EPIDEMIC_END = 0xce,
    // A research category has nothing left to research.
    SFX_RESEARCH_EXHAUSTED = 0xdb,
    // Each character of the win or lose text as it is typed out.
    SFX_WINLOSE_TYPE = 0xee
};

// crequester.id, irequester_slots[].kind and open_requester's idx: a requester
// window, its window_list index (its button sprites are hires_req_filenames,
// same index). Named after the routine that handles its clicks.
enum {
    // req4: a doctor's, nurse's or receptionist's window.
    REQ_STAFF = 0,
    // req4 with the handyman's priority buttons.
    REQ_HANDYMAN = 1,
    // req3.
    REQ_PATIENT = 2,
    // req5.
    REQ_MACHINE = 3,
    // req13.
    REQ_YES_NO = 4,
    // req14.
    REQ_EDIT_ROOM = 5,
    // req7.
    REQ_QUEUE = 6,
    // req10.
    REQ_BUILD_ROOM = 9,
    // req11.
    REQ_BUY_OBJECTS = 0xa,
    // req2.
    REQ_HIRE_STAFF = 0xb,
    // req12: a staff member's demand (open_staff_resign_requester).
    REQ_STAFF_DEMAND = 0xc,
    // req6.
    REQ_JUKEBOX = 0xd,
    // No requester open.
    REQ_NONE = 0x63
};

// display_mode: what the main loop (game()) runs each frame.
enum {
    // The hospital view.
    DM_GAME = 0,
    // A full-screen window (do_charts).
    DM_CHARTS = 1,
    DM_MAIN_MENU = 2,
    DM_AREA_SELECT = 3,
    // The town map (draw_town_select).
    DM_TOWN_MAP = 4,
    DM_NETWORK_SELECT = 5,
    DM_LOAD_GAME = 6,
    DM_WINLOSE = 7,
    DM_YEAR_END = 8,
    // A fax (display_choice).
    DM_CHOICE = 9,
    DM_NICKNAME = 10,
    // The hospital view, stopped under a requester that must be answered
    // (yes/no, a staff member's demand).
    DM_MODAL = 11,
    // The win or lose screen after the year end.
    DM_YEAR_END_WINLOSE = 12,
    // A network player has lost focus (process_lost_focus_display).
    DM_LOST_FOCUS = 13,
    // Bidding for a plot in the town map.
    DM_TOWN_BID = 14
};

// load_ingame_pic's idx: a full-screen picture, its hiresscreens and
// screenpalettes index (the comment gives its file in qdata).
enum {
    // town01v.
    SCR_TOWN = 1,
    // drugn01v.
    SCR_DRUGS = 2,
    // stat01v: the bank statement.
    SCR_STATEMENT = 3,
    // graph01v.
    SCR_GRAPHS = 4,
    // pol01v.
    SCR_POLICY = 5,
    // res01v.
    SCR_RESEARCH = 6,
    // bank01v: the bank manager.
    SCR_BANK_MANAGER = 7,
    // staff01v: the staff list.
    SCR_STAFF_LIST = 8,
    // bid01v: the town map while bidding for a plot.
    SCR_TOWN_BID = 10,
    // pref01v, for the nickname and network screens.
    SCR_NETWORK_PREFS = 0xc,
    // main01m.
    SCR_MAIN_MENU = 0xd,
    // fame01v: the hall of fame.
    SCR_HALL_OF_FAME = 0xe,
    // award01v and award02v: the year-end awards.
    SCR_AWARDS1 = 0xf,
    SCR_AWARDS2 = 0x10,
    // pref01v, for the load game screen.
    SCR_LOAD_GAME = 0x11,
    // title01v.
    SCR_TITLE = 0x12,
    // fax01v.
    SCR_FAX = 0x13,
    // rep01v: the patient overview (display_peep_overview).
    SCR_OVERVIEW = 0x14,
    // load01v: the loading screen.
    SCR_LOADING = 0x15,
    // cred01v.
    SCR_CREDITS = 0x17,
    // No picture (empty file name): only the screen pointers load.
    SCR_NONE = 0x18
};

// play_sound and play_sample_using_heap source: a fixed sample source for
// sounds that do not come from a thing (manage_sound keeps only sources
// above 0xe, the things, positioned on screen).
enum {
    // Sound effects at full volume in the centre.
    SNDSRC_GENERAL = 0xa,
    // The ambient sample queue (do_ambients).
    SNDSRC_AMBIENT = 0xb,
    // The ticking clock: the emergency countdown, and the bank statement.
    SNDSRC_CLOCK = 0xd
};

// SfxDetail.type: how play_sound picks the sample of a sound effect.
enum {
    SFXT_NONE = 0,
    SFXT_SAMPLE = 1,
    // Stops the sample instead.
    SFXT_STOP = 2,
    // The sample at a random pitch of pitch_group[group].
    SFXT_RANDOM_PITCH = 3,
    // A random sample of sample_group[group].
    SFXT_RANDOM_SAMPLE = 4,
    // The sample at a random pitch in the range of pitch_group[group].
    SFXT_PITCH_RANGE = 5
};

#endif
