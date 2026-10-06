#include "core/world/finance.h"

#include "core/ui/charts.h"

#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>

namespace th
{

namespace
{

constexpr int32_t kMaxBalance = 9999999;

// ObjectInfo.category values the hospital's statistics count
// (inc_hardware_stats, src/objects.cpp).
constexpr uint8_t kCategoryMachine = 1;
constexpr uint8_t kCategoryDiagnosisMachine = 2;
constexpr uint8_t kCategoryLeisure = 3;
constexpr uint8_t kCategoryRadiator = 6;

// The game's town table has one entry per level; past it, nothing.
const Balance::Town &town(const Balance &settings, int level)
{
    static const Balance::Town none;
    return size_t(level) < settings.towns.size() ? settings.towns[size_t(level)] : none;
}

// adjust_monthly_figures (src/finance.cpp): shifts each history row one
// month and stores this month's figures in the newest. Deaths and cures are
// not counted yet. The visitors row is shifted but its newest month kept,
// as in the game.
void adjust_monthly_figures(Finance &f, int32_t balance, int reputation)
{
    for (auto &row : f.history)
        std::copy(row.begin() + 1, row.end(), row.begin());
    f.history[kHistoryBalance][11] = balance;
    f.history[kHistoryIncome][11] = f.income_this_month;
    f.history[kHistoryExpenditure][11] = f.expenditure_this_month;
    f.history[kHistoryWages][11] = f.wages_this_month;
    f.history[kHistoryDeaths][11] = 0;
    f.history[kHistoryCures][11] = 0;
    f.history[kHistoryReputation][11] = reputation;
    f.income_this_month = 0;
    f.expenditure_this_month = 0;
    f.wages_this_month = 0;
}

} // namespace

void start_finance(Finance &finance, int32_t balance, const Balance &settings, int level)
{
    finance = {};
    finance.start_cash = town(settings, level).start_cash;
    finance.year_start_balance = balance;
    finance.statement_balance = balance;
}

void change_balance(Finance &f, int32_t &balance, int day, int month, uint8_t type, int32_t amount, uint16_t arg,
                    uint16_t company)
{
    bool keeps_arg = type == kTxInsurance || type == kTxTreat || type == kTxFinalTreat || type == kTxCureTreatment ||
                     type == kTxAdvance;
    trace_event("balance %u amount=%d arg=%u", unsigned(type), int(amount), keeps_arg ? unsigned(arg) : 0u);

    int count = f.entry_count;
    BankEntry *last = count ? &f.entries[size_t(count - 1)] : nullptr;
    if (last && last->type == type && last->day == day && last->month == month) {
        last->amount += amount;
    } else {
        BankEntry &e = f.entries[size_t(count)];
        e.day = uint8_t(day);
        e.month = uint8_t(month);
        e.type = type;
        e.amount = amount;
        switch (type) {
        case kTxInsurance:
            e.extra = uint8_t(company);
            break;
        case kTxTreat:
        case kTxFinalTreat:
        case kTxCureTreatment:
        case kTxAdvance:
            e.extra = uint8_t(arg);
            break;
        }
        if (count < 0x31) {
            f.entry_count++;
        } else {
            // Full: the oldest ten lines go into the opening balance. The
            // line just written stays at 49, past the 40 kept.
            for (int i = 0; i < 40; i++) {
                if (i < 10)
                    f.statement_balance += f.entries[size_t(i)].amount;
                f.entries[size_t(i)] = f.entries[size_t(i + 10)];
            }
            f.entry_count -= 9;
        }
    }
    if (type == kTxWages || type == kTxHireStaff)
        f.wages_this_month -= amount;
    balance += amount;
    if (type != kTxCheatMoney) {
        if (amount > 0)
            f.income_this_month += amount;
        else if (amount < 0)
            f.expenditure_this_month -= amount;
    }
    // Unsigned, as the game compares it: a balance above the starting cash
    // wraps round and usually sets it too.
    if ((f.start_cash - uint32_t(balance)) * 8u > f.start_cash)
        f.low_balance = true;
    balance = std::min(balance, kMaxBalance);
}

int count_radiators(const World &world, uint8_t owner)
{
    int n = 0;
    for (const Thing &t : world.things)
        if (t.type == kThingObject && t.owner == owner && t.subtype < kObjectKinds &&
            kObjects[t.subtype].category == kCategoryRadiator)
            n++;
    return n;
}

void month_end(Finance &f, int32_t &balance, World &world, const Balance &settings, int level, int day, int month,
               int radiators, int reputation)
{
    int32_t wages = 0;
    for (size_t i = 1; i < world.things.size(); i++) {
        Thing &t = world.things[i];
        if (t.type == kThingStaff && t.state != kStaffApplicant) {
            if (t.owner == 0)
                wages += t.salary;
            appraise_staff(t, settings);
        }
    }
    if (wages != 0)
        change_balance(f, balance, day, month, kTxWages, -wages);
    if (world.drinks_income != 0) {
        change_balance(f, balance, day, month, kTxIncomeDrinks, world.drinks_income);
        world.drinks_income = 0;
    }
    if (world.hospital) {
        for (uint16_t j = 0; j < 3; j++) {
            int32_t &claims = world.hospital->insurance_claims[j][size_t(month)];
            if (claims != 0) {
                int32_t amount = claims;
                claims = 0;
                change_balance(f, balance, day, month, kTxInsurance, amount, j, world.ins_control[j]);
            }
        }
    }
    int32_t heating = radiators * f.heating_level * f.heating_price / 100;
    if (heating != 0)
        change_balance(f, balance, day, month, kTxHeating, -heating);
    const Balance::Town &t = town(settings, level);
    if (f.loan != 0)
        change_balance(f, balance, day, month, kTxLoanInterest, -(t.interest_rate * f.loan / 10000 / 12));
    if (balance < 0)
        change_balance(f, balance, day, month, kTxOverdraftInterest, t.interest_rate * balance * 2 / 10000 / 12);
    adjust_monthly_figures(f, balance, reputation);
}

void year_end(Finance &f, int32_t balance, const World &world, const Balance &settings, int reputation)
{
    calculate_value(f, world, settings, reputation);
    f.income_last_year = 0;
    f.expenditure_last_year = 0;
    for (int i = 0; i < 12; i++) {
        f.income_last_year += f.history[kHistoryIncome][size_t(i)];
        f.expenditure_last_year += f.history[kHistoryExpenditure][size_t(i)];
    }
    f.entry_count = 0;
    f.year_start_balance = balance;
    f.statement_balance = balance;
}

void calculate_value(Finance &f, const World &world, const Balance &settings, int reputation)
{
    int32_t object_value = 0;
    for (const Thing &t : world.things) {
        if (t.type != kThingObject || t.owner != 0 || t.subtype >= kObjectKinds)
            continue;
        uint8_t category = kObjects[t.subtype].category;
        if (category == kCategoryMachine || category == kCategoryDiagnosisMachine || category == kCategoryLeisure)
            object_value += settings.objects[t.subtype].start_cost;
    }
    int32_t rep_value = reputation / 50 * 1000;
    // count_hospital_area
    int area = 0;
    for (const MapCell &cell : world.map)
        if (cell.owner_flags & 1)
            area++;
    f.area = uint16_t(area);
    f.hospital_value = f.area * settings.gbv.land_cost_per_tile + object_value + rep_value + 10000;
    f.hospital_value = std::max(f.hospital_value, 0);
    f.max_loan = std::clamp(f.hospital_value / 3 + 10000, 0, 100000);
}

} // namespace th
