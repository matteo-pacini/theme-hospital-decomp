#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/data/text.h"
#include "core/ui/layer.h"
#include "core/world/events.h"
#include "core/world/finance.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace th
{

class GameData;
struct World;

// An entry of the game's advice table (advice[], src/advisor.cpp): a
// situation the advisor may speak about, how long its message stays, and
// how often it may come back.
struct Advice {
    // The situation find_advice tests; 0 ends the table.
    uint16_t id = 0;
    // Game turns the message stays up.
    uint16_t duration = 0;
    // The larger, the longer the advice waits, per time given, before it
    // may be given again.
    uint16_t repeat_weight = 0;
    uint8_t times_given = 0;
    // The game turn it was last given.
    uint32_t time = 0;
};

// Entries of the advice table, the unused first and the terminating one
// included.
constexpr int kAdviceCount = 85;

// Advisor::flags bits (advisor.flags).
constexpr uint8_t kAdvisorTutorial = 0x1;
// The object being placed may not go where it is (set while the object
// cursor is drawn).
constexpr uint8_t kAdvisorBadPlacement = 0x2;

// Advisor::event_flags bits (advisor.event_flags): what happened since the
// advice about it was last given.
constexpr uint32_t kAdvisorEventResign = 0x1;

// The game's speech bubble limits in high resolution
// (resvars.advisor_min_segments, advisor_max_segments): the middle pieces
// the bubble has at least and at most.
constexpr int kAdvisorMinSegments = 3;
constexpr int kAdvisorMaxSegments = 0x20;

// The advisor's animation states (advisor.anim_state): 0 away, 1 coming,
// 2 and 3 up, 4 leaving, 9 and 10 the up states' occasional specials; 5 to
// 8 are never entered. Per state, its animation (advisor_anims, an
// AnimData::starts index), the state its animation leads to
// (next_advisor_state) and whether he is up, with his bubble shown and
// right-clickable (advisor_up_states); src/tables.cpp.
inline constexpr std::array<uint16_t, 11> kAdvisorAnims = {0, 219, 230, 2374, 220, 1647, 1648, 1680, 1649, 2374, 2374};
inline constexpr std::array<uint8_t, 11> kNextAdvisorState = {1, 2, 3, 4, 0, 6, 7, 8, 0, 2, 2};
inline constexpr std::array<bool, 11> kAdvisorUpStates = {false, false, true,  true, false, false,
                                                          false, true,  false, true, true};

// The advisor and his speech bubble (the game's Advisor, advisor in
// src/advisor.cpp and src/advtext.cpp), with the advice table's history
// and the tutorial's progress. Set it up with start_advisor.
struct Advisor {
    // The advice table entry shown (an `advice` index), 0 for none.
    int16_t advice_id = 0;
    // An event's message to give next (language section 0x14), set by the
    // game where it happens; 0 for none.
    uint16_t forced_advice_id = 0;
    // A mouse-help message to give (section 0x15); 0 for none.
    int16_t mouse_advice_id = 0;
    // His animation: the frame shown and the state (kAdvisorAnims index).
    uint16_t anim_frame = 0;
    uint8_t anim_state = 0;
    // The state was moved on by the frame logic rather than by its animation.
    bool state_advanced = false;
    // Frames until he leaves once his message is gone.
    int16_t time_left = 0;
    // The message, wrapped into lines ('\n') once `rewrap` is cleared.
    std::string text;
    // kAdvisorTutorial and kAdvisorBadPlacement.
    uint8_t flags = 0;
    // The panel icon or requester control the tutorial points at, 0 for
    // none (not drawn yet).
    uint16_t tutorial_highlight = 0;
    // The wrapped message's lines (1-3; 0 before it is wrapped) and the
    // middle pieces its bubble has beyond kAdvisorMinSegments.
    uint8_t line_count = 0;
    uint8_t extra_segments = 0;
    // The message is new and must be wrapped before it is drawn.
    bool rewrap = false;
    // One past the bubble width last tried in wrapping (the game's
    // no_pieces); the bubble is drawn only up to kAdvisorMaxSegments.
    uint8_t no_pieces = 0;
    // Left edge of the bubble as last drawn, for right clicks.
    int16_t box_left = 0;
    // kAdvisorEvent* bits.
    uint32_t event_flags = 0;
    // The statistics pass found patients (not ported: never set).
    bool has_patients = false;
    // The player's advisor setting (myplayer.advisor_flags[20]).
    bool enabled = true;
    // Messages given once per game (myplayer.advisor_flags[0..19]).
    std::array<uint8_t, 20> given{};
    // A right click on him or his bubble asked him to go
    // (othervar.dismiss_advisor).
    bool dismiss = false;
    // The tutorial's step (advisor.tutorial_step) and per message, 0 not
    // shown, 1 shown, 2 done (tutorial_display_flags).
    uint8_t tutorial_step = 0;
    std::array<uint8_t, 0x26> tutorial_shown{};
    // The advice table (advice[]): the game's durations and weights, with
    // when each was last given.
    std::array<Advice, kAdviceCount> advice{};

    // What the last advisor_frame drew: he was drawn (with anim_frame as it
    // was then) and his bubble was.
    bool drawn = false;
    uint16_t drawn_frame = 0;
    bool bubble = false;
};

// What the advisor looks at in a frame.
struct AdvisorInputs {
    // The game turn (Game::turn_count) and whether the game is paused.
    uint32_t game_turn = 0;
    bool paused = false;
    // A requester other than the edit room one is open (crequester.id): the
    // message shown is held.
    bool requester_open = false;
    // The level, the month of the year (0-11), whether the hospital is
    // open, its balance and reputation.
    int level = 0;
    int month = 0;
    bool open = false;
    int32_t balance = 0;
    int reputation = 500;
    const Finance *finance = nullptr;
    const Balance *settings = nullptr;
    const World *world = nullptr;
    // The language's text.
    const LanguageText *text = nullptr;
    // For the tutorial: the command waiting to be carried out this frame
    // (player_events[player]) and the one carried out last frame
    // (old_packet[player]); whether staff held may be put down where the
    // pointer is (gamevar.staff_place_ok); while an object is being placed,
    // whether it may go where it is.
    Event pending;
    Event last;
    bool staff_place_ok = false;
    std::optional<bool> object_place_ok;
};

// Sets the advisor up for a new level, as setup_new_level (reset_advisor
// and the tutorial's flags, src/start.cpp) and the area screen
// (tutorial step 0x1d, src/area.cpp) do: away, no message, no history,
// with the tutorial on or off. Keeps `enabled` and `given`.
void start_advisor(Advisor &advisor, bool tutorial);

// display_advisor (src/advisor.cpp) without its drawing, run once a frame
// while the game is shown, paused or not, before the frame's commands are
// carried out. If enabled: in the tutorial, its current step's message
// (tutorial); otherwise the message shown expires after its duration or a
// right click (dismiss), and the first advice due whose situation holds
// (find_advice) becomes the message. He comes when there is one, goes once
// it is gone, and animates (advisor_character, drawing on `safeseed`, the
// game's saferand); `drawn`, `drawn_frame` and `bubble` say what to draw.
//
// The situations tested are those the game can be in so far: the balance
// low (0x48) or near the level's money target (0x14), the reputation and
// hospital value targets (0x42, 0x43), too many staff for no staff room
// (0x49), a reception desk with no receptionist on level 1 (0x69), a staff
// member threatening to resign (0x1c, kAdvisorEventResign), and the event
// and mouse-help messages (forced_advice_id 1 and 3-0x11 but 2 and 8, and
// mouse_advice_id). The others need what is not ported yet (rooms, the
// statistics pass, research, emergencies, epidemics, rats, earthquakes,
// cures and deaths, prices) and are never given.
//
// The tutorial is ported to the "build a GP's office" message (steps 0x1d
// to 0x24 and 0x20): the reception desk and the receptionist. Its hints that
// the game shows only as help text in high resolution leave him hidden.
void advisor_frame(Advisor &advisor, const AdvisorInputs &in, const AnimData &anims, uint32_t &safeseed);

// monthly_update_of_adivsor_counts (src/advisor.cpp), at month end: the
// advice about how close the level is to being won or lost may be given
// again.
void monthly_update_of_advisor_counts(Advisor &advisor);

// process_advisor_text (src/advtext.cpp): wraps the message, in `font`
// (font slot 0x1d), into the narrowest bubble that holds it in at most three
// lines, breaking at the last space or punctuation that fits (the break
// character is dropped). If it fits in none, the text stays as it is and
// no_pieces ends past kAdvisorMaxSegments.
void wrap_advisor_text(Advisor &advisor, const Font &font);

// Whether a right click at (x, y) (640 x 480 coordinates, above the panel)
// is on him or his bubble while he is up, as process_mouse tests it; the
// caller then sets `dismiss` and gives the click to nothing else.
bool advisor_hit(const Advisor &advisor, int x, int y);

// Draws what the last advisor_frame showed into `layer` (640 x 480
// coordinates), over the right of the panel, as advisor_character and
// advisor_text_box (src/advtext.cpp) draw them: him at the panel's top 560
// pixels across, and his speech bubble (the panel sprites 37 to 39) left of
// him with the message in `font` (font slot 0x1d). A new message is wrapped
// first (wrap_advisor_text); one that fits no bubble is not drawn. Sets
// Advisor::box_left.
void draw_advisor(UiLayer &layer, Advisor &advisor, const GameData &data, const Font &font);

} // namespace th
