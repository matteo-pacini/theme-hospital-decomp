#include "core/ui/fax.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"
#include "core/world/build.h"
#include "core/world/emergency.h"
#include "core/world/epidemic.h"
#include "core/world/mayor.h"
#include "core/world/queue.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"

#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstring>

namespace th
{

namespace
{

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks

// display_choice's states (choice.state).
constexpr int kFaxDraw = 0x5a;
constexpr int kFaxUp = 0x5b;
constexpr int kFaxClose = 0x5c;

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxYesX = 0x41;
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxFaxIn = 0xab;
constexpr uint8_t kSfxFaxYes = 0xac;
constexpr uint8_t kSfxFaxNo = 0xad;
constexpr uint8_t kSfxFax0 = 0xbe;

// The fax's layout (src/scoords.cpp): the message's box (ScreenBoxes[173]),
// the answer buttons' size (screenbuttons[0xae]: 43 x 43) and gap, and
// where the sprite covering the close button goes (ScreenCoords[0xbe]).
constexpr Rect kChoiceBox = {200, 50, 336, 270};
constexpr int kAnswerSize = 43;
constexpr int kAnswerGap = 5;
constexpr Point kNoClose = {595, 435};

// The fax sprites (fax02v): an answer button, pressed and disabled, and the
// cover of the close button.
constexpr size_t kSpriteAnswer = 17;
constexpr size_t kSpriteAnswerPressed = 18;
constexpr size_t kSpriteAnswerDisabled = 19;
constexpr size_t kSpriteNoClose = 20;

// The font of the fax's text (cache slot 0x1e).
constexpr int kFaxFont = 0x1e;

// screenbuttons 0xb8 to 0xc7 (src/scoords.cpp): the keypad, each drawn
// pressed with its sprite where it is: 0xb9-0xc2 dial 0-9, 0xb8, 0xc5 and
// 0xc6 send, 0xc3 and 0xc4 only beep, 0xc7 closes.
struct KeypadButton {
    uint8_t id;
    Rect box;
    size_t sprite;
};
constexpr std::array<KeypadButton, 16> kKeypad = {{
    {0xb8, {168, 348, 44, 10}, 1},
    {0xb9, {271, 382, 44, 12}, 12},
    {0xba, {220, 348, 43, 10}, 2},
    {0xbb, {272, 348, 44, 10}, 3},
    {0xbc, {327, 348, 43, 10}, 4},
    {0xbd, {219, 358, 44, 10}, 5},
    {0xbe, {272, 358, 43, 10}, 6},
    {0xbf, {326, 358, 44, 10}, 7},
    {0xc0, {218, 370, 44, 11}, 8},
    {0xc1, {271, 370, 44, 11}, 9},
    {0xc2, {326, 370, 44, 11}, 10},
    {0xc3, {217, 382, 45, 12}, 11},
    {0xc4, {326, 382, 45, 12}, 13},
    {0xc5, {472, 349, 87, 20}, 14},
    {0xc6, {474, 372, 91, 27}, 15},
    {0xc7, {598, 440, 26, 26}, 16},
}};
constexpr uint8_t kKeyClose = 0xc7;

// cheat_numbers (src/choice.cpp): the numbers that dial through, by the
// keypad buttons pressed.
struct FaxNumber {
    std::array<uint8_t, 5> digits;
    uint8_t count;
};
constexpr std::array<FaxNumber, 15> kCheatNumbers = {{
    {{0xc2, 0xc2, 0xc2}, 3},
    {{0xc2, 0xba, 0xba}, 3},
    {{0xc2, 0xba, 0xbb}, 3},
    {{0xba, 0xb9, 0xb9}, 3},
    {{0xba, 0xba, 0xba}, 3},
    {{0xbb, 0xbb, 0xbb}, 3},
    {{0xbc, 0xbc, 0xbc}, 3},
    {{0xbd, 0xbd, 0xbd}, 3},
    {{0xbe, 0xbe, 0xbe}, 3},
    {{0xbf, 0xbf, 0xbf}, 3},
    {{0xc0, 0xc0, 0xc0}, 3},
    {{0xc1, 0xc1, 0xc1}, 3},
    {{0xc2, 0xc2, 0xc2}, 3},
    {{0xc0, 0xbb, 0xc1, 0xc0}, 4},
    {{0xbb, 0xbd, 0xbc, 0xbb, 0xc1}, 5},
}};

// The answers to the emergency's offer and to the patients' calls
// (EV_CHOICE_RESULT values).
constexpr std::array<uint16_t, 3> kNoTreatmentAnswers = {kChoiceTreatSendHome, kChoiceTreatWait, kChoiceTreatResearch};
constexpr std::array<uint16_t, 3> kNoDiagnosisAnswers = {kChoiceDiagSendHome, kChoiceDiagGuess, kChoiceDiagWait};

// The object kind of drug treatments (OBJ_PHARMACY_CABINET).
constexpr uint8_t kObjPharmacyCabinet = 39;

uint16_t saferand(uint32_t &safeseed)
{
    safeseed = std::rotr(safeseed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(safeseed);
}

// sprintf with a string of the language file as the format.
template <typename... Args> std::string format(std::string_view fmt, Args... args)
{
    if constexpr (sizeof...(args) == 0) {
        return std::string(fmt);
    } else {
        char buf[600];
        std::snprintf(buf, sizeof buf, std::string(fmt).c_str(), args...);
        return buf;
    }
}

bool over(int x, int y, Rect r)
{
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

// calculate_buttons' search of choice_text: the answer buttons and message
// strings of `kind`, past the sections (each ended by ".") before it; the
// buttons end with "!".
void calculate_buttons_text(Fax &fax, uint8_t kind)
{
    constexpr std::array<int, 11> kSkip = {0, 0, 1, 2, 4, 5, 6, 3, 7, 8, 9};
    const LanguageText &text = *fax.text;
    int count = int(text.size(TextSection::Choice));
    int i = 0;
    for (int skip = kind < kSkip.size() ? kSkip[kind] : 0; skip > 0 && i < count; i++)
        if (text.at(TextSection::Choice, i) == ".")
            skip--;
    fax.buttons.clear();
    fax.strings.clear();
    for (; i < count && text.at(TextSection::Choice, i) != "!"; i++)
        fax.buttons.emplace_back(text.at(TextSection::Choice, i));
    for (i++; i < count && text.at(TextSection::Choice, i) != "."; i++)
        fax.strings.emplace_back(text.at(TextSection::Choice, i));
}

// The message's string `i`, empty past its last.
std::string_view str(const Fax &fax, size_t i)
{
    return i < fax.strings.size() ? std::string_view(fax.strings[i]) : std::string_view();
}

// What a hospital lacks for an illness: the room type of its treatment if
// it has none and could build one, and the first skill it needs that none
// of its staff has (listhead.rooms and staff_by_skill).
struct Missing {
    int room = 0;
    int staff = 0;
};

Missing missing_for(const Game &game, uint8_t illness)
{
    const World &world = game.world;
    const ExpertiseInfo &exp = kExpertiseData[illness % kExpertiseData.size()];
    Missing m;
    if (world.room_of_type[exp.category] == 0 && room_available(world, exp.category))
        m.room = exp.category;
    for (int i = 0; i < 9; i++) {
        if (!(exp.staff_mask & (1 << i)))
            continue;
        bool any = std::any_of(world.things.begin(), world.things.end(), [&](const Thing &t) {
            return t.type == kThingStaff && t.owner == 0 && t.state != kStaffApplicant && (t.skills & (1 << i));
        });
        if (!any) {
            m.staff = i;
            break;
        }
    }
    return m;
}

std::string illness_name(const Fax &fax, int illness)
{
    return std::string(fax.text->at(TextSection::Illness, illness));
}

// A room type's name, from room_names (`real` false) or real_room_names.
std::string room_name(const Fax &fax, int room, bool real)
{
    int id = kRoomTypeInfo[size_t(room) % kRoomTypes].name;
    return std::string(fax.text->at(real ? TextSection::RealRoomNames : TextSection::RoomNames, id));
}

std::string skill_name(const Fax &fax, int skill)
{
    return std::string(fax.text->at(TextSection::SkillNames, skill));
}

// announce_emergency's line on whether the hospital can cure the illness.
std::string announce_emergency_cure(const Fax &fax, const Game &game, uint8_t illness)
{
    if (game.world.cure_available[illness % kIllnessCount] == 1) {
        int effectiveness = 0;
        if (kExpertiseData[illness % kExpertiseData.size()].kind == kObjPharmacyCabinet)
            effectiveness = game.hospital.illnesses[illness % kIllnessCount].drug_effectiveness;
        if (effectiveness)
            return format(str(fax, 2), illness_name(fax, illness).c_str(), effectiveness);
        return format(str(fax, 3));
    }
    Missing m = missing_for(game, illness);
    if (m.room && m.staff)
        return format(str(fax, 4), room_name(fax, m.room, false).c_str(), skill_name(fax, m.staff).c_str());
    if (m.room)
        return format(str(fax, 5), room_name(fax, m.room, false).c_str());
    if (m.staff)
        return format(str(fax, 6), skill_name(fax, m.staff).c_str());
    return format(str(fax, 7));
}

// announce_emergency (0xaf600): who calls, the patients and their illness,
// whether the hospital can cure it, and the bonus. Accepting is refused
// while an emergency runs. The offer not found, the game reads past its
// table; nothing is offered here.
std::string announce_emergency(const Fax &fax, const Game &game, uint16_t &available, uint32_t &safeseed)
{
    if (game.world.emergency.state != 0)
        available &= ~1u;
    Disasters::Offer offer;
    for (const Disasters::Offer &o : game.disasters.offers)
        if (o.call == game.disasters.popup_call && o.used) {
            offer = o;
            break;
        }
    std::string name(str(fax, saferand(safeseed) % 4 + 9));
    std::string text = format(str(fax, 0), name.c_str());
    text += "\n" + format(str(fax, 1), int(offer.count), illness_name(fax, offer.illness).c_str());
    text += "\n" + announce_emergency_cure(fax, game, offer.illness);
    text += "\n" + format(str(fax, 8), int(offer.count) * int(offer.bonus));
    return text;
}

// emergency_over (0xafb34): the bonus earned out of the most there was, and
// the patients saved. With no patients the game divides by zero; no bonus
// here.
std::string emergency_over(const Fax &fax, const Game &game)
{
    const World::Emergency &e = game.world.emergency;
    int bonus = 0;
    if (e.patients && e.cured * 100 / e.patients >= e.win_percent)
        bonus = e.bonus * e.cured;
    int total = e.bonus * e.patients;
    return format(str(fax, 0), total, bonus) + "\n" + format(str(fax, 1), int(e.cured), int(e.patients));
}

// A VIP's name (mayor_names).
std::string mayor_name(const Fax &fax, int idx)
{
    return std::string(fax.text->at(TextSection::MayorNames, idx));
}

// mayor_requester (0xa0b70): the VIP's name in the offer's text (strs[0]).
std::string vips_offer(const Fax &fax, const Game &game)
{
    return format(str(fax, 0), mayor_name(fax, game.world.mayor.name).c_str());
}

// report_mayor_result (0xa17e8): the Telegram, the VIP's remark (the
// verdict's quote, mayor_result index + 5), the cash grant if there is one
// and the reputation line for the good and bad verdicts (not fair).
std::string report_mayor_result(const Fax &fax, const Game &game)
{
    const World::Mayor &m = game.world.mayor;
    int verdict = m.verdict;
    std::string text = format(str(fax, 0));
    text += "\n" + format(str(fax, 1), mayor_name(fax, m.name).c_str());
    text += "\n\n" + format(str(fax, verdict + 5));
    int value = kMayorResult[verdict % kMayorResult.size()].cash_bonus;
    if (value != 0)
        text += "\n" + format(str(fax, 2), value);
    uint8_t event = kMayorResult[verdict % kMayorResult.size()].reputation_event;
    if (event == kRepVipGood)
        text += "\n" + format(str(fax, 3));
    else if (event == kRepVipBad)
        text += "\n" + format(str(fax, 4));
    return text;
}

// declare_epidemic (0xa0240): the illness, the fine declaring it costs, and
// the two questions.
std::string declare_epidemic(const Fax &fax, const Game &game)
{
    std::string illness = illness_name(fax, game.world.epidemic.illness);
    std::string text = format(str(fax, 0), illness.c_str());
    text += "\n" + format(str(fax, 1), int(game.world.epidemic.fine));
    text += "\n" + format(str(fax, 2));
    text += "\n" + format(str(fax, 3));
    return text;
}

// epidemic_over (0xa073c): the result fax, chosen by the inspector's verdict
// (World::Epidemic::inspection[0]) and the illness it judged.
std::string epidemic_over(const Fax &fax, const Game &game)
{
    const World::Epidemic &e = game.world.epidemic;
    std::string illness = illness_name(fax, e.inspected_illness);
    switch (e.inspection[0]) {
    case 0:
        return format(str(fax, 2), illness.c_str()) + "\n" + format(str(fax, 3)) + "\n" +
               format(str(fax, 4), int(e.fine));
    case 1:
        return format(str(fax, 0), illness.c_str()) + "\n" + format(str(fax, 1)) + "\n" +
               format(str(fax, 5), int(e.fine));
    case 2:
        return format(str(fax, 0), illness.c_str()) + "\n" + format(str(fax, 1)) + "\n" +
               format(str(fax, 6), int(e.fine));
    default:
        return format(str(fax, 0), illness.c_str()) + "\n" + format(str(fax, 1)) + "\n" + format(str(fax, 7));
    }
}

// How sure the diagnosis of patient `p` is, in percent, 0-99.
int diagnosis_percent(const Thing &p)
{
    return std::clamp(p.diagnosis * 100 / 1000, 0, 99);
}

// call_no_treatment (0x18b78): the patient's illness, or its guess, what
// the hospital lacks to treat it, and the question. The research
// department's button is refused until autopsies can be done.
std::string call_no_treatment(const Fax &fax, const Game &game, uint16_t id, uint16_t &available)
{
    if (id == 0 || id >= game.world.things.size())
        return {};
    const Thing &p = game.world.things[id];
    Missing m = missing_for(game, p.illness);
    std::string lack;
    if (m.room && m.staff)
        lack = format(str(fax, 0), room_name(fax, m.room, true).c_str(), skill_name(fax, m.staff).c_str());
    else if (m.room)
        lack = format(str(fax, 1), room_name(fax, m.room, true).c_str());
    else if (m.staff)
        lack = format(str(fax, 2), skill_name(fax, m.staff).c_str());
    else
        lack = format(str(fax, 3));
    if (game.world.cure_available[kIllAutopsy] != 1)
        available &= ~4u;
    std::string text;
    if (p.person_flags & kPfGuessed)
        text = format(str(fax, 6), diagnosis_percent(p), illness_name(fax, p.illness).c_str());
    else
        text = format(str(fax, 4), illness_name(fax, p.illness).c_str());
    return text + "\n\n" + lack + "\n" + format(str(fax, 5));
}

// announce_new_illness_fax (0x190c4): the illness's name and its three
// lines of description. What the player lacks to treat it the game works
// out but never shows. Nothing for kinds 0 and 1.
std::string announce_new_illness_fax(const Fax &fax, uint16_t kind)
{
    if (kind <= 1)
        return {};
    std::string text = format(str(fax, 4), illness_name(fax, kind).c_str());
    for (size_t i = 0; i < 3; i++)
        text += "\n" + format(str(fax, kind * 3 - 1 + i));
    return text;
}

// call_no_diagnosis (src/treat.cpp): the diagnosis given up, and its best
// guess if the hospital knows and can cure that illness; guessing is
// refused otherwise.
std::string call_no_diagnosis(const Fax &fax, const Game &game, uint16_t id, uint16_t &available)
{
    if (id == 0 || id >= game.world.things.size())
        return {};
    const Thing &p = game.world.things[id];
    uint8_t illness = p.illness % kIllnessCount;
    bool known = game.world.cure_available[illness] == 1 && game.hospital.illnesses[illness].discovered == 1;
    if (game.world.cure_available[illness] != 1 || game.hospital.illnesses[illness].discovered == 0)
        available &= ~2u;
    std::string text = format(str(fax, 0)) + "\n" + format(str(fax, 1)) + "\n";
    if (known)
        text += format(str(fax, 2), diagnosis_percent(p), illness_name(fax, p.illness).c_str());
    return text;
}

// format_text (src/winlose.cpp): each line of `text` broken at its last
// space within `width` pixels in `font`.
std::string format_text(const Font &font, std::string text, int width)
{
    size_t line = 0, cursor = 0;
    for (;;) {
        while (text_width(font, std::string_view(text).substr(line, cursor - line)) <= width) {
            if (cursor == text.size())
                return text;
            cursor++;
        }
        do {
            if (cursor <= line)
                return text;
            --cursor;
        } while (text[cursor] != ' ');
        text[cursor] = '\n';
        line = ++cursor;
    }
}

// display_buttons (0xa3db4): the answer buttons, disabled ones greyed, each
// with its text left of it, centred on it.
void display_buttons(Fax &fax)
{
    int width = kChoiceBox.w - kAnswerSize;
    TextSettings settings{{kChoiceBox.x, 0, width}, {kChoiceBox.x, 0, width, kUiHeight}, kTextWrap};
    std::vector<Rect> boxes = fax_answer_buttons(fax);
    for (size_t i = 0; i < boxes.size(); i++) {
        bool on = fax.available & (1u << i);
        draw_sprite(fax.background, boxes[i].x, boxes[i].y, fax.sprites.at(on ? kSpriteAnswer : kSpriteAnswerDisabled));
        std::string text = format_text(fax.font, fax.buttons[i], width);
        // The text is centred on the button's width taken as its height.
        int y = boxes[i].y + (kAnswerSize - text_height(fax.font, text)) / 2;
        draw_text(fax.background, fax.font, 0, y, text, settings);
    }
}

// display_choice's first state: the fax drawn on its picture, the message
// and the buttons, and the close button covered if it must be answered.
void display_choice_draw(Fax &fax, const Game &game, uint32_t &safeseed, FaxResult &result)
{
    fax.available = 0xffff;
    fax.background = fax.picture;
    calculate_buttons_text(fax, fax.choice.type);
    std::string message = fax_message(fax, game, fax.available, safeseed);
    const Rect &box = kChoiceBox;
    draw_text(fax.background, fax.font, 0, 0, message, {{box.x, box.y, box.w}, box, kTextCentre});
    display_buttons(fax);
    if (!fax.choice.timeout)
        draw_sprite(fax.background, kNoClose.x, kNoClose.y, fax.sprites.at(kSpriteNoClose));
    fax.state = kFaxUp;
    fax.still_since = game.turn_count;
    fax.dialled_count = 0;
    fax.surface = fax.background;
    result.sounds.push_back(kSfxFaxIn);
}

// The answer button `button`'s handler: process_emergency_start_buttons,
// process_emergency_end_buttons, process_no_treatment_buttons,
// process_announce_new_illness_buttons, process_mayor_start_buttons,
// process_mayor_end_buttons or process_no_diagnosis_buttons.
void process_answer(Fax &fax, size_t button, FaxResult &result, Game &game)
{
    auto answer = [&](const std::array<uint16_t, 3> &values) {
        // packet_choice_result: with the fax's subject.
        if (button < values.size())
            result.event = {EV_CHOICE_RESULT, values[button], 0, fax.choice.subject};
    };
    switch (fax.choice.type) {
    case kFaxEmergency:
        if (button == 0)
            result.event = {EV_CHOICE_RESULT, kChoiceEmergencyAccept, 0, 0};
        else if (button == 1)
            result.event = {EV_CHOICE_RESULT, kChoiceEmergencyRefuse, 0, 0};
        break;
    case kFaxEpidemic:
        if (button == 0)
            result.event = {EV_CHOICE_RESULT, kChoiceEpidemicDeclare, 0, 0};
        else if (button == 1)
            result.event = {EV_CHOICE_RESULT, kChoiceEpidemicCoverUp, 0, 0};
        break;
    case kFaxEmergencyOver:
    case kFaxEpidemicOver:
    case kFaxNewIllness:
        if (button == 0)
            fax.state = kFaxClose;
        break;
    case kFaxNoTreatment:
        answer(kNoTreatmentAnswers);
        break;
    case kFaxNoDiagnosis:
        answer(kNoDiagnosisAnswers);
        break;
    case kFaxVip:
        if (button == 0) {
            game.world.mayor.postponed = 0;
            result.event = {EV_CHOICE_RESULT, kChoiceVipAccept, 0, 0};
        } else if (button == 1) {
            // process_mayor_start_buttons: a refusal is sent, then, after more
            // than five refusals in a row while the turn's random percentage
            // is below 10, an acceptance overwrites it.
            result.event = {EV_CHOICE_RESULT, kChoiceVipRefuse, 0, 0};
            if (++game.world.mayor.postponed > 5 && game.world.rand_pct < 10) {
                game.world.mayor.postponed = 0;
                result.event = {EV_CHOICE_RESULT, kChoiceVipAccept, 0, 0};
            }
        }
        break;
    case kFaxVipResult:
        if (button == 0)
            fax.state = kFaxClose;
        break;
    default:
        break;
    }
}

// check_choice_buttons (0xa3430): the enabled answer button under a held
// left button drawn pressed; one under a released left button answered.
// Returns whether the release was taken.
bool check_choice_buttons(Fax &fax, const UiMouse &m, FaxResult &result, Game &game)
{
    std::vector<Rect> boxes = fax_answer_buttons(fax);
    for (size_t i = 0; i < boxes.size(); i++) {
        if (!over(m.x, m.y, boxes[i]) || !(fax.available & (1u << i)))
            continue;
        if (m.left_released) {
            result.sounds.push_back(kSfxYesX);
            process_answer(fax, i, result, game);
        }
        if (m.left_held || m.left_released)
            draw_sprite(fax.surface, boxes[i].x, boxes[i].y, fax.sprites.at(kSpriteAnswerPressed));
        return m.left_released;
    }
    return false;
}

// process_fax_buttons (0xa4384): dials, beeps, sends the number dialled
// (a cheat's number plays SFX_FAX_YES, any other SFX_FAX_NO; what the
// cheats do is not ported) or closes a fax that may close itself.
void process_fax_buttons(Fax &fax, uint8_t id, FaxResult &result)
{
    if (id >= 0xb9 && id <= 0xc2) {
        if (fax.dialled_count < fax.dialled.size())
            fax.dialled[fax.dialled_count++] = id;
        result.sounds.push_back(uint8_t(kSfxFax0 + (id - 0xb9)));
    } else if (id == 0xc3 || id == 0xc4) {
        result.sounds.push_back(kSfxFax0);
    } else if (id == 0xb8 || id == 0xc5 || id == 0xc6) {
        bool found = std::any_of(kCheatNumbers.begin(), kCheatNumbers.end(), [&](const FaxNumber &n) {
            return n.count == fax.dialled_count &&
                   std::equal(n.digits.begin(), n.digits.begin() + n.count, fax.dialled.begin());
        });
        result.sounds.push_back(found ? kSfxFaxYes : kSfxFaxNo);
        fax.dialled_count = 0;
    } else if (id == kKeyClose && fax.choice.timeout) {
        fax.state = kFaxClose;
        result.sounds.push_back(kSfxNo4);
    }
}

// check_fax_buttons (0xa374c): the keypad button under a held left button
// drawn pressed; one under a released one pressed. The close button works
// only on a fax that may close itself.
void check_fax_buttons(Fax &fax, const UiMouse &m, FaxResult &result)
{
    for (const KeypadButton &b : kKeypad) {
        if (!over(m.x, m.y, b.box))
            continue;
        if (b.id == kKeyClose && !fax.choice.timeout)
            break;
        if (m.left_released)
            process_fax_buttons(fax, b.id, result);
        if (m.left_held || m.left_released)
            draw_sprite(fax.surface, b.box.x, b.box.y, fax.sprites.at(b.sprite));
        break;
    }
}

// display_choice's second state, the fax up.
void display_choice_up(Fax &fax, const UiMouse &m, bool escape, Game &game, FaxResult &result)
{
    fax.surface = fax.background;
    if (!check_choice_buttons(fax, m, result, game))
        check_fax_buttons(fax, m, result);
    if (!fax.choice.timeout)
        return;
    if (escape) {
        result.sounds.push_back(kSfxNo4);
        fax.state = kFaxClose;
    }
    if (fax.mouse_position != m.x + m.y) {
        fax.still_since = game.turn_count;
        fax.mouse_position = m.x + m.y;
    }
    if (game.turn_count - fax.still_since > uint32_t(fax.choice.timeout))
        fax.state = kFaxClose;
}

} // namespace

bool load_fax(Fax &fax, const std::filesystem::path &dir, const LanguageText &text, std::string *error)
{
    auto dat = load_data_file(dir, "qdata/fax01v.dat", error);
    auto pal = dat ? load_data_file(dir, "qdata/fax01v.pal", error) : std::nullopt;
    auto tab = pal ? load_data_file(dir, "qdata/fax02v.tab", error) : std::nullopt;
    auto sprites = tab ? load_data_file(dir, "qdata/fax02v.dat", error) : std::nullopt;
    if (!sprites || !load_font(dir, kFaxFont, fax.font, error))
        return false;
    std::array<uint8_t, 768> vga{};
    if (dat->size() < size_t(kUiWidth) * kUiHeight || pal->size() < vga.size() ||
        !fax.sprites.load(std::move(*tab), std::move(*sprites), SpriteFormat::Req)) {
        if (error)
            *error = "fax01v/fax02v: not the fax's picture and sprites";
        return false;
    }
    fax.picture.resize(kUiWidth, kUiHeight);
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(fax.picture.row(y), dat->data() + size_t(y) * kUiWidth, kUiWidth);
    std::memcpy(vga.data(), pal->data(), vga.size());
    palette_from_vga(vga, fax.palette);
    fax.text = &text;
    fax.surface = fax.picture;
    return true;
}

void display_choice_screen(Fax &fax, const Game::ChoiceScreen &choice)
{
    if (!choice.type || !fax.text)
        return;
    fax.open = true;
    fax.state = kFaxDraw;
    fax.choice = choice;
}

void fax_command(Fax &fax, const Event &event)
{
    if (event.type == EV_CHOICE_RESULT)
        fax.open = false;
}

FaxResult fax_frame(Fax &fax, const UiMouse &mouse, const FaxKeys &keys, double seconds, Game &game, uint32_t &safeseed)
{
    FaxResult result;
    if (!fax.open)
        return result;
    UiMouse &m = fax.pending_mouse;
    bool lp = m.left_pressed, lr = m.left_released;
    m = mouse;
    m.left_pressed |= lp;
    m.left_released |= lr;
    fax.pending_escape |= keys.escape;
    fax.frame_time += seconds;
    // The first frame runs at once: it shows the fax.
    if (fax.frame_time < kGameFrame && fax.state != kFaxDraw)
        return result;
    fax.frame_time = std::min(std::max(fax.frame_time - kGameFrame, 0.0), kGameFrame);
    if (fax.state == kFaxDraw)
        display_choice_draw(fax, game, safeseed, result);
    else if (fax.state == kFaxUp)
        display_choice_up(fax, m, fax.pending_escape, game, result);
    if (fax.state == kFaxClose)
        fax.open = false;
    m.left_pressed = m.left_released = false;
    fax.pending_escape = false;
    return result;
}

std::vector<Rect> fax_answer_buttons(const Fax &fax)
{
    std::vector<Rect> boxes;
    int n = int(fax.buttons.size());
    for (int i = 0; i < n; i++)
        boxes.push_back({kChoiceBox.x + kChoiceBox.w - kAnswerSize,
                         kChoiceBox.y + kChoiceBox.h - (n - i) * (kAnswerSize + kAnswerGap), kAnswerSize, kAnswerSize});
    return boxes;
}

std::string fax_message(const Fax &fax, const Game &game, uint16_t &available, uint32_t &safeseed)
{
    switch (fax.choice.type) {
    case kFaxEmergency:
        return announce_emergency(fax, game, available, safeseed);
    case kFaxEmergencyOver:
        return emergency_over(fax, game);
    case kFaxEpidemic:
        return declare_epidemic(fax, game);
    case kFaxEpidemicOver:
        return epidemic_over(fax, game);
    case kFaxNoTreatment:
        return call_no_treatment(fax, game, fax.choice.subject, available);
    case kFaxNewIllness:
        return announce_new_illness_fax(fax, fax.choice.subject);
    case kFaxNoDiagnosis:
        return call_no_diagnosis(fax, game, fax.choice.subject, available);
    case kFaxVip:
        return vips_offer(fax, game);
    case kFaxVipResult:
        return report_mayor_result(fax, game);
    default:
        return {};
    }
}

} // namespace th
