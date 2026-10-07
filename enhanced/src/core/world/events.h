#pragma once

#include <cstdint>

namespace th
{

// A player's command, as the game's Event (include/hosp.h): the interface
// sets it, and the game carries it out at the start of its next frame
// (process_turn, src/io.cpp) — the one way the player changes the game, so
// that network games and replays stay in step. One command at a time: a
// new one replaces one not yet carried out, as player_events[player] does.
struct Event {
    uint8_t type = 0;   // EventType
    uint16_t value = 0; // main argument
    // View scroll requests: 1 up, 2 down, 4 left, 8 right; 0x10 recentres.
    uint8_t flags = 0;
    uint16_t arg = 0; // second argument
};

// The commands, with the game's names and numbers (include/enums.h).
enum EventType : uint8_t {
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
    // The player declines a win offered at a quarter (the win/lose screen's
    // decline button); the replay carries it as this command, which the
    // instrumented original accepts in process_turn.
    EV_DECLINE_WIN = 0x8f,
    EV_NET_WIN = 0x90,
    EV_RESTART_LEVEL = 0x91
};

} // namespace th
