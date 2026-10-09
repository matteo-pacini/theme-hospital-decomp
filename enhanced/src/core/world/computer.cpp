#include "core/world/computer.h"

#include "core/data/text.h"
#include "core/world/patient_tables.h"
#include "core/world/rooms.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstring>
#include <string_view>

namespace th
{

namespace
{

// OBJ_PHARMACY_CABINET: the object of the illnesses drugs cure.
constexpr uint8_t kObjPharmacyCabinet = 39;
// ComputerPlayer entries in Balance::computer.
constexpr int kComputerPlayers = 20;
// Candidates tried for a random computer player before giving up.
constexpr int kRandomTries = 1000;

// The hospital's name field with `text` (cut to fit) in it.
void set_name(RivalHospital &rival, std::string_view text)
{
    rival.name = {};
    std::memcpy(rival.name.data(), text.data(), std::min(text.size(), rival.name.size() - 1));
}

// Makes hospital `rival` the computer player `index` and starts its script
// (the common end of both loops in setup_computer_players).
void assign_player(RivalHospital &rival, const Balance &settings, uint8_t index, uint16_t area)
{
    rival.player_type = 2;
    rival.ai_index = index;
    rival.strategy = settings.computer[index].strategy;
    rival.strategy_step = int16_t(find_strategy_start(rival.strategy));
    rival.area = area;
}

// setup_computer_players' first loop: each computer player marked as playing
// takes the next free hospital. Returns the index of the last hospital given
// out, or one past the end if a player found none free.
size_t setup_computer_players_playing(std::array<RivalHospital, kHospitalCount> &rivals, const Balance &settings,
                                      uint16_t area, std::array<bool, kComputerPlayers> &used)
{
    size_t k = 1;
    for (uint8_t j = 0; j < kComputerPlayers; j++) {
        if (!settings.computer[j].playing)
            continue;
        for (k = 1; k < rivals.size(); k++) {
            if (rivals[k].player_type == 0) {
                assign_player(rivals[k], settings, j, area);
                used[j] = true;
                break;
            }
        }
        if (k >= rivals.size())
            break;
    }
    return k;
}

// setup_computer_players' second loop for hospital `rival`: a random unused
// computer player, tried up to kRandomTries times.
void setup_computer_players_random(World &world, RivalHospital &rival, const Balance &settings, uint16_t area,
                                   uint32_t mouse_sum, std::array<bool, kComputerPlayers> &used)
{
    rival.player_type = 2;
    set_name(rival, "DEF 64");
    uint8_t j = 0;
    for (int i = 0; i < kRandomTries; i++) {
        j = uint8_t((mouse_sum + old_newrand(world.seed)) % kComputerPlayers);
        rival.ai_index = j;
        if (!used[j]) {
            rival.name = {};
            used[j] = true;
            break;
        }
    }
    assign_player(rival, settings, j, area);
}

} // namespace

uint32_t find_strategy_start(uint8_t type)
{
    for (size_t i = 0; i < kComStrategy.size(); i++) {
        if (kComStrategy[i].type == 9)
            break;
        if (kComStrategy[i].type == 5 && kComStrategy[i].arg == type)
            return uint32_t(i + 1);
    }
    return 0;
}

void init_rival(RivalHospital &rival, const Balance &settings, int32_t start_cash)
{
    rival = {};
    rival.balance = start_cash;
    rival.cure_available[0] = 1;
    for (size_t i = 0; i < size_t(kRivalObjects); i++)
        rival.object_stats[i].cost = settings.objects[i].start_cost;
    for (size_t i = 0; i < size_t(kRivalIllnesses); i++) {
        rival.discovered[i] = settings.expertise[i].known;
        if (kExpertiseData[i].kind == kObjPharmacyCabinet) {
            rival.drug_effectiveness[i] = uint8_t(settings.gbv.start_rating);
            rival.drug_cost[i] = settings.gbv.start_cost;
        }
    }
    rival.heating_level = 5;
    rival.max_loan = 15000;
    rival.year_start_balance = start_cash;
    rival.score = rival.start_score = 10000;
    rival.market_share = 25;
}

void start_rivals(World &world, const Balance &settings, int32_t start_cash)
{
    world.rivals = {};
    for (size_t owner = 1; owner < world.rivals.size(); owner++)
        init_rival(world.rivals[owner], settings, start_cash);
}

void setup_computer_players(World &world, const Balance &settings, int level_number, uint16_t area, int mouse_x,
                            int mouse_y)
{
    if (kTownExtras[level_number > 14 ? 1 : size_t(level_number)].special_kind == 1)
        return;
    std::array<bool, kComputerPlayers> used{};
    size_t k = setup_computer_players_playing(world.rivals, settings, area, used);
    // k is where the playing loop stopped: the hospitals before it have their
    // player, and k at the end means a player found none free.
    const uint32_t mouse_sum = uint32_t(mouse_x) + uint32_t(mouse_y);
    for (; k < world.rivals.size(); k++)
        if (world.rivals[k].player_type == 0)
            setup_computer_players_random(world, world.rivals[k], settings, area, mouse_sum, used);
}

void name_computer_players(std::array<RivalHospital, kHospitalCount> &rivals, const LanguageText &text)
{
    for (RivalHospital &rival : rivals)
        if (rival.player_type == 2 && rival.name[0] == '\0')
            set_name(rival, text.at(TextSection::ComputerNames, rival.ai_index));
}

} // namespace th
