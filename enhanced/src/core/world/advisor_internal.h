// What the advisor's source files share: the helpers of find_advice and
// the situations it tests. Private to core/world/advisor*.cpp.

#pragma once

#include "core/world/advisor.h"

#include "core/world/staff.h"
#include "core/world/world.h"

#include <cstdio>
#include <string>
#include <string_view>

namespace th
{
inline namespace advisor_detail
{

// Win and lose criteria (WinLose.criteria).
constexpr uint16_t kCriterionReputation = 1;
constexpr uint16_t kCriterionBalance = 2;
constexpr uint16_t kCriterionCures = 4;
constexpr uint16_t kCriterionDeaths = 5;
constexpr uint16_t kCriterionValue = 6;

// The game's OBJ_RECEPTION_DESK.
constexpr uint8_t kObjectReceptionDesk = 11;

// sprintf(advisor.text, format, ...) with a string of the language file.
template <typename... Args> void say(Advisor &a, std::string_view format, Args... args)
{
    if constexpr (sizeof...(args) == 0) {
        a.text = format;
    } else {
        char buf[270];
        std::snprintf(buf, sizeof buf, std::string(format).c_str(), args...);
        a.text = buf;
    }
}

// The player has a reception (listhead.rooms[ROOM_RECEPTION], which placing
// the first reception desk sets).
bool has_reception(const World &world);

// is_relevant_criterion (src/sim.cpp): the last entry of `table` for
// `criterion`, 0 for none (entry 0 is never found).
int relevant_criterion(uint16_t criterion, const std::array<Balance::WinLose, 10> &table);

// saferand (src/advisor.cpp), drawing on `safeseed`.
uint16_t saferand(uint32_t &safeseed);

std::string_view advisor_text(const AdvisorInputs &in, int i);

std::string_view extra_advice(const AdvisorInputs &in, int i);

// The game's OBJ_* kinds and ROOM_* types the advice tests.
constexpr uint8_t kObjectBench = 4;
constexpr uint8_t kObjectDrinksMachine = 7;
constexpr uint8_t kObjectToilet = 51;
constexpr uint8_t kRoomGpOffice = 7;
constexpr uint8_t kRoomTrainingRoom = 22;
constexpr uint8_t kRoomStaffRoom = 25;
constexpr uint8_t kRoomResearchRoom = 28;
constexpr uint8_t kRoomPsychiatry = 8;
constexpr uint8_t kRoomWard = 9;
constexpr uint8_t kRoomTheatre = 10;
constexpr uint8_t kRoomPharmacy = 11;
constexpr uint8_t kRoomToilets = 29;

// What find_advice's situations of the statistics pass read: hospital 0's
// figures (World::counts) and staff, and where the advice goes.
struct Situation {
    Advisor &a;
    const AdvisorInputs &in;
    World &world;
    const AdCounts &c;
    StaffCounts staff;
    uint32_t &safeseed;

    bool has_patients() const { return c.has_patients; }
    int handymen() const { return staff.by_kind[kStaffHandyman]; }
    std::string_view text(int i) const { return advisor_text(in, i); }
    std::string_view extra(int i) const { return extra_advice(in, i); }
    std::string_view situation(int i) const { return in.text->at(TextSection::AdvisorSituations, i); }
    // play_ambient of an announcement, for the caller to play.
    void ambient(uint8_t index) const
    {
        if (in.ambient)
            *in.ambient = index;
    }
    // A message given once per game (myplayer.advisor_flags[flag]) or else
    // `otherwise`.
    void once(int flag, int situation_text, std::string_view otherwise)
    {
        if (!a.given[size_t(flag)]) {
            say(a, situation(situation_text));
            a.given[size_t(flag)] = 1;
        } else {
            say(a, otherwise);
        }
    }
};

// find_advice's situations of advisor_situations.cpp, each returning whether
// it gave advice for `situation`.
bool advise_no_visitors(Situation &s, int id, bool cures);
bool advise_room_built(Situation &s, uint16_t situation);
bool advise_patients(Situation &s, uint16_t situation);
bool advise_staff(Situation &s, uint16_t situation);
bool advise_litter_and_plants(Situation &s, int id, uint16_t situation);
bool advise_hospital(Situation &s, int id, uint16_t situation);
bool advise_events(Situation &s, uint16_t situation);

// find_advice (src/advisor.cpp) for the situations ported (see
// advisor_frame), drawing on `safeseed` (saferand).
bool find_advice(Advisor &a, int id, const AdvisorInputs &in, uint32_t &safeseed);

// tutorial (src/tutorial.cpp), in high resolution. Returns 1 when a message
// was just given, 0 when he is to be hidden, -1 otherwise.
int tutorial(Advisor &a, const AdvisorInputs &in);

} // namespace advisor_detail

} // namespace th
