#pragma once

#include "core/data/balance.h"

#include <array>
#include <cstdint>

namespace th
{

struct World;

// change_balance's transaction types, as listed in the bank statement (the
// game's TX_*, include/enums.h; language section 7 at the same index).
enum Transaction : uint8_t {
    kTxWages = 1,
    kTxHireStaff = 2,
    kTxBuyObject = 3,
    kTxBuildRoom = 4,
    kTxCure = 5,
    kTxBuyLand = 6,
    kTxTreat = 7,
    kTxFinalTreat = 8,
    kTxCureTreatment = 9,
    kTxTreatmentDeposit = 10,
    kTxAdvance = 11,
    kTxResearchCosts = 12,
    kTxIncomeDrinks = 13,
    kTxIncomeJukebox = 14,
    kTxCheatMoney = 15,
    kTxHeating = 16,
    kTxInsurance = 17,
    kTxBankLoan = 18,
    kTxLoanRepayment = 19,
    kTxLoanInterest = 20,
    kTxResearchBonus = 21,
    kTxDrugCost = 22,
    kTxOverdraftInterest = 23,
    kTxSeverancePay = 24,
    kTxGeneralBonus = 25,
    kTxSellObject = 26,
    kTxPersonalBonus = 27,
    kTxEmergencyBonus = 28,
    kTxVaccination = 29,
    kTxEpidemicCoverupFine = 30,
    kTxGovernmentCompensation = 31,
    kTxVipAward = 32,
    kTxEpidemicFine = 33,
    kTxYearEndBonus = 34,
    kTxTrophyBonus = 35,
    kTxReplacementMachine = 36,
};

// One line of the bank statement (the game's BankEntry).
struct BankEntry {
    int32_t amount = 0;
    uint8_t type = 0; // Transaction
    uint8_t day = 0;
    uint8_t month = 0; // 0 January
    // The insurance company or illness the line names, for kTxInsurance,
    // kTxTreat, kTxFinalTreat, kTxCureTreatment and kTxAdvance.
    uint8_t extra = 0;
};

// Lines the bank statement holds (GameControl.entries).
constexpr int kBankEntries = 0x32;

// The graph history's rows (GameControl.history's first index).
enum HistoryRow : int {
    kHistoryWages = 0,
    kHistoryIncome = 1,
    kHistoryExpenditure = 2,
    kHistoryBalance = 3,
    kHistoryVisitors = 4,
    kHistoryDeaths = 5,
    kHistoryCures = 6,
    kHistoryReputation = 7,
};

// The player's finances: the game's GameControl money fields and the
// AllHosps ones of the local player that are not the balance itself, which
// stays with the caller. Set it up with start_finance.
struct Finance {
    // This month's wages and hiring fees paid, money received and money spent
    // (positive), moved into the history at month end.
    int32_t wages_this_month = 0;
    int32_t income_this_month = 0;
    int32_t expenditure_this_month = 0;
    // Per row, the last 12 months, [11] the newest. The game keeps three
    // identical copies (one per graph scale); this is one of them.
    std::array<std::array<int32_t, 12>, 8> history{};
    // The last year's income and expenditure, totalled at year end.
    int32_t income_last_year = 0;
    int32_t expenditure_last_year = 0;
    // The bank statement, oldest first, and its opening balance, which grows
    // by the lines dropped when it is full.
    std::array<BankEntry, kBankEntries> entries{};
    uint8_t entry_count = 0;
    int32_t statement_balance = 0;
    // The balance when the year began.
    int32_t year_start_balance = 0;
    // The heating setting (1-10) and its price factor: the monthly bill is
    // radiators * heating_level * heating_price / 100.
    uint8_t heating_level = 5;
    uint16_t heating_price = 750;
    // The bank loan, and how much may be borrowed (calculate_value).
    int32_t loan = 0;
    int32_t max_loan = 0;
    // The hospital's value (calculate_value) and its land in cells.
    int32_t hospital_value = 0;
    uint16_t area = 0;
    // The level's starting cash (towns[level].start_cash).
    uint32_t start_cash = 0;
    // Set by change_balance once the balance is under 7/8 of the starting
    // cash (advisor.event_flags 0x20); the advisor reads it.
    bool low_balance = false;
};

// Sets `finance` up for a level whose starting balance is `balance`, as
// init_player and setup_new_level (src/start.cpp) do: heating 5 at price
// 750, the year and the statement opening at `balance`.
void start_finance(Finance &finance, int32_t balance, const Balance &settings, int level);

// change_balance (src/finance.cpp) for the local player: adds `amount`
// (negative to spend) to `balance` as a transaction of `type`, capped at
// 9999999; records it in the statement dated `day` `month` (merged into the
// last line if that has the same type and date), and in the month's income
// or expenditure (but kTxCheatMoney) and, for kTxWages and kTxHireStaff,
// wages. Sets low_balance as the game warns the advisor. `arg` is the
// line's illness, or for kTxInsurance the company's slot (0-2), whose
// company the caller passes in `company` (ins_control[arg], which the game
// looks up itself): that is what the line keeps.
void change_balance(Finance &finance, int32_t &balance, int day, int month, uint8_t type, int32_t amount,
                    uint16_t arg = 0, uint16_t company = 0);

// The radiators player `owner` has (allhosps.radiator_count, which the game
// counts as they are placed and sold).
int count_radiators(const World &world, uint8_t owner);

// The money part of month_end (src/finance.cpp) for the local player (player
// 0), on the month's last day `day` of `month`: appraises every hired staff
// member (appraise_staff) and pays their salaries as one kTxWages line;
// banks the month's drinks sales (World::drinks_income) and the insurers'
// payments due this month (World::hospital's insurance_claims); charges
// heating for `radiators`, the loan's monthly interest and, if overdrawn,
// twice that rate on the overdraft (towns[level].interest_rate, in
// hundredths of a percent a year); then moves the month's figures into the
// history (adjust_monthly_figures, with the balance and `reputation`).
// Jukebox takings, which complex_success does not keep, are not ported;
// the prices month_end updates last (update_cure_costs) are the caller's,
// which has the hospitals' markets. The game also adds up held
// applicants' wages, under no hospital (owner 0x63, out of its table's
// bounds): they are appraised but not paid here.
void month_end(Finance &finance, int32_t &balance, World &world, const Balance &settings, int level, int day, int month,
               int radiators, int reputation);

// The money part of year_end (src/finance.cpp): values the hospital
// (calculate_value), totals the last 12 months' income and expenditure,
// empties the statement and opens the new year and statement at `balance`.
void year_end(Finance &finance, int32_t balance, const World &world, const Balance &settings, int reputation);

// calculate_value (src/finance.cpp) for player 0: the hospital's area, its
// value (land at gbv.land_cost_per_tile a cell, 1000 per 50 reputation,
// its machines and leisure objects at their balance cost, and a base of
// 10000; rooms, of which there are none yet, would add their cost) and the
// loan limit, a third of the value plus 10000, at most 100000. The game
// counts the objects' value as they are bought, at the price of the day;
// here it is summed from the objects on the map.
void calculate_value(Finance &finance, const World &world, const Balance &settings, int reputation);

} // namespace th
