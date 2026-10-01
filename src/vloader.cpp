#include "hosp.h"

struct VariableTable {
    char name[0x30];
    u16 flags;
    u32 address;
    u16 stride;
    u16 max_index;
    s32 min;
    s32 max;
};

struct QuakeControl;
struct ComputerPlayer;
extern WinLose win_criteria[];
extern WinLose lose_criteria[];
extern WinLoseNet net_criteria[];
extern QuakeControl quake_control[];
extern ComputerPlayer computer[];

extern "C" {
extern u16 visuals_available[];
extern u8 emergency_control[];
}

VariableTable balance_variables[0xcb] = {
    { "expertise.StartPrice", 0x12, (u32)((u8 *)expertise + 0x2), 19, 47, 0, 1000000 },
    { "expertise.ContRate", 0x11, (u32)((u8 *)expertise + 0xa), 19, 47, 0, 100 },
    { "expertise.RschReqd", 0x14, (u32)((u8 *)expertise + 0xc), 19, 47, 0, 1000000 },
    { "expertise.MaxDiagDiff", 0x12, (u32)((u8 *)expertise + 0x10), 19, 47, 1, 1000 },
    { "expertise.Known", 0x11, (u32)((u8 *)expertise + 0x12), 19, 47, 0, 1 },
    { "objects.StartCost", 0x14, (u32)((u8 *)objects + 0x8), 37, 62, 0, 1000000 },
    { "objects.StartAvail", 0x11, (u32)((u8 *)objects + 0x7), 37, 62, 0, 1 },
    { "objects.WhenAvail", 0x12, (u32)((u8 *)objects + 0xc), 37, 62, 0, 240 },
    { "objects.StartStrength", 0x11, (u32)((u8 *)objects + 0x1d), 37, 62, 1, 255 },
    { "objects.AvailableForLevel", 0x11, (u32)((u8 *)objects + 0x1c), 37, 62, 0, 1 },
    { "towns.StartCash", 0x14, (u32)towns, 29, 12, 0, 100000 },
    { "towns.IllRate", 0x14, (u32)((u8 *)towns + 0x7), 29, 12, 0, 100 },
    { "towns.InterestRate", 0x12, (u32)((u8 *)towns + 0x10), 29, 12, 0, 10000 },
    { "towns.InflationRate", 0x12, (u32)((u8 *)towns + 0xe), 29, 12, 0, 10000 },
    { "rooms.Cost", 0xc, (u32)((u8 *)rooms + 0x4), 42, 31, 0, 50000 },
    { "staff.MinSalary", 0x12, (u32)((u8 *)staff + 0x2), 13, 10, 0, 1000 },
    { "gbv.LandCostPerTile", 0x12, (u32)((u8 *)&gbv + 0x7c), 2, 1, 0, 10000 },
    { "gbv.SalaryAdd", 0xa, (u32)((u8 *)&gbv + 0xb2), 2, 9, -1000, 1000 },
    { "gbv.SalaryAbilityDivisor", 0x12, (u32)((u8 *)&gbv + 0xc4), 2, 1, 0, 100 },
    { "gbv.ResearchPointsDivisor", 0x12, (u32)((u8 *)&gbv + 0xc6), 2, 1, 0, 100 },
    { "gbv.StartRating", 0x12, (u32)((u8 *)&gbv + 0xc8), 2, 1, 1, 100 },
    { "gbv.StartCost", 0x12, (u32)((u8 *)&gbv + 0xca), 2, 1, 1, 10000 },
    { "gbv.MinDrugCost", 0x12, (u32)((u8 *)&gbv + 0xcc), 2, 1, 1, 5000 },
    { "gbv.HowContagious", 0x12, (u32)((u8 *)&gbv + 0xce), 2, 1, 1, 1000 },
    { "gbv.ReduceContMonths", 0x12, (u32)((u8 *)&gbv + 0xd0), 2, 1, 0, 1000 },
    { "gbv.ReduceContPeepCount", 0x12, (u32)((u8 *)&gbv + 0xd2), 2, 1, 0, 1000 },
    { "gbv.ReduceContRate", 0x12, (u32)((u8 *)&gbv + 0xd4), 2, 1, 0, 100 },
    { "gbv.HoldVisualMonths", 0x12, (u32)((u8 *)&gbv + 0xd6), 2, 1, 0, 1000 },
    { "gbv.HoldVisualPeepCount", 0x12, (u32)((u8 *)&gbv + 0xd8), 2, 1, 0, 1000 },
    { "gbv.RestStanding", 0x12, (u32)((u8 *)&gbv + 0xda), 2, 1, 1, 5000 },
    { "gbv.RestSofa", 0x12, (u32)((u8 *)&gbv + 0xdc), 2, 1, 1, 5000 },
    { "gbv.RestGame", 0x12, (u32)((u8 *)&gbv + 0xde), 2, 1, 1, 5000 },
    { "gbv.RestSnooker", 0x12, (u32)((u8 *)&gbv + 0xe0), 2, 1, 1, 5000 },
    { "gbv.WorkLight", 0x12, (u32)((u8 *)&gbv + 0xe2), 2, 1, 1, 5000 },
    { "gbv.ModifyFreq", 0x12, (u32)((u8 *)&gbv + 0xe4), 2, 1, 1, 5000 },
    { "gbv.NotTired", 0x12, (u32)((u8 *)&gbv + 0xe6), 2, 1, 1, 10000 },
    { "gbv.Tired", 0x12, (u32)((u8 *)&gbv + 0xe8), 2, 1, 1, 10000 },
    { "gbv.VeryTired", 0x12, (u32)((u8 *)&gbv + 0xea), 2, 1, 1, 10000 },
    { "gbv.CrackUpTired", 0x12, (u32)((u8 *)&gbv + 0xec), 2, 1, 1, 10000 },
    { "gbv.RecoveryFactor", 0x12, (u32)((u8 *)&gbv + 0xee), 2, 1, 1, 10000 },
    { "gbv.RecoveryMinimum", 0x12, (u32)((u8 *)&gbv + 0xf0), 2, 1, 1, 10000 },
    { "gbv.MaxObjectStrength", 0x12, (u32)((u8 *)&gbv + 0xf2), 2, 1, 1, 500 },
    { "gbv.ResearchIncrement", 0x12, (u32)((u8 *)&gbv + 0xf4), 2, 1, 1, 500 },
    { "gbv.QPoints", 0x12, (u32)((u8 *)&gbv + 0x100), 2, 1, 0, 50 },
    { "gbv.DistPoints", 0x12, (u32)((u8 *)&gbv + 0x102), 2, 1, 0, 50 },
    { "gbv.NoStaffPoints", 0x12, (u32)((u8 *)&gbv + 0x104), 2, 1, 0, 50 },
    { "gbv.LitterDrop", 0xa, (u32)((u8 *)&gbv + 0x106), 2, 1, 0, 500 },
    { "gbv.LeaveMax", 0xa, (u32)((u8 *)&gbv + 0x108), 2, 1, 0, 1000 },
    { "gbv.ResignMax", 0xa, (u32)((u8 *)&gbv + 0x10a), 2, 1, 0, 1000 },
    { "gbv.Happy", 0xa, (u32)((u8 *)&gbv + 0x10c), 2, 1, 0, 100 },
    { "gbv.Unhappy", 0xa, (u32)((u8 *)&gbv + 0x10e), 2, 1, 0, 100 },
    { "gbv.VeryUnhappy", 0xa, (u32)((u8 *)&gbv + 0x110), 2, 1, 0, 100 },
    { "gbv.DrinkHappy", 0xa, (u32)((u8 *)&gbv + 0x112), 2, 1, 0, 50 },
    { "gbv.ToiletHappy", 0xa, (u32)((u8 *)&gbv + 0x114), 2, 1, 0, 50 },
    { "gbv.BowelFull", 0xa, (u32)((u8 *)&gbv + 0x116), 2, 1, 0, 200 },
    { "gbv.BowelOverflows", 0xa, (u32)((u8 *)&gbv + 0x118), 2, 1, 0, 500 },
    { "gbv.VomitLimit", 0xa, (u32)((u8 *)&gbv + 0x11a), 2, 1, 0, 500 },
    { "gbv.LitterRandom", 0x12, (u32)((u8 *)&gbv + 0x11c), 2, 1, 0, 60000 },
    { "gbv.SalaryTooLow", 0xa, (u32)((u8 *)&gbv + 0x11e), 2, 1, -50, 50 },
    { "gbv.SalaryTooHigh", 0xa, (u32)((u8 *)&gbv + 0x120), 2, 1, 0, 50 },
    { "gbv.MaxDiagDiff", 0x12, (u32)((u8 *)&gbv + 0x122), 2, 1, 1, 1000 },
    { "gbv.ScoreMaxInc", 0x12, (u32)((u8 *)&gbv + 0x124), 2, 1, 1, 10000 },
    { "gbv.VacCost", 0x12, (u32)((u8 *)&gbv + 0x126), 2, 1, 1, 1000 },
    { "gbv.TrainingValue", 0x12, (u32)((u8 *)&gbv + 0xf6), 2, 3, 1, 1000 },
    { "gbv.DisasterLaunch", 0x12, (u32)((u8 *)&gbv + 0x12c), 2, 1, 0, 10000 },
    { "gbv.EpidemicFine", 0x12, (u32)((u8 *)&gbv + 0x12e), 2, 1, 1, 20000 },
    { "gbv.EpidemicCompLo", 0x12, (u32)((u8 *)&gbv + 0x130), 2, 1, 0, 10000 },
    { "gbv.EpidemicCompHi", 0x12, (u32)((u8 *)&gbv + 0x132), 2, 1, 0, 20000 },
    { "gbv.MinimumAbductTime", 0x12, (u32)((u8 *)&gbv + 0x134), 2, 1, 0, 500 },
    { "gbv.AbductionsPerYear", 0x12, (u32)((u8 *)&gbv + 0x136), 2, 1, 0, 500 },
    { "gbv.AutopsyRschPercent", 0x12, (u32)((u8 *)&gbv + 0x138), 2, 1, 0, 100 },
    { "gbv.AutopsyRepHitPercent", 0x12, (u32)((u8 *)&gbv + 0x13a), 2, 1, 0, 100 },
    { "gbv.AllocRand", 0x11, (u32)((u8 *)&gbv + 0x13c), 1, 1, 1, 255 },
    { "gbv.AllocTotalRep", 0x11, (u32)((u8 *)&gbv + 0x13d), 1, 1, 1, 255 },
    { "gbv.AllocIndRep", 0x11, (u32)((u8 *)&gbv + 0x13e), 1, 1, 1, 255 },
    { "gbv.AllocDelay", 0x11, (u32)((u8 *)&gbv + 0x13f), 1, 1, 0, 255 },
    { "gbv.TrainingRate", 0x12, (u32)((u8 *)&gbv + 0x141), 2, 1, 0, 1000 },
    { "gbv.PromoDoc", 0x11, (u32)((u8 *)&gbv + 0x143), 1, 1, 1, 255 },
    { "gbv.PromoCon", 0x11, (u32)((u8 *)&gbv + 0x144), 1, 1, 1, 255 },
    { "gbv.DoctorThreshold", 0x12, (u32)((u8 *)&gbv + 0x156), 2, 1, 0, 1000 },
    { "gbv.ConsultantThreshold", 0x12, (u32)((u8 *)&gbv + 0x158), 2, 1, 0, 1000 },
    { "gbv.DrugImproveRate", 0x11, (u32)((u8 *)&gbv + 0x140), 1, 1, 1, 100 },
    { "gbv.ContagiousSpreadFactor", 0x11, (u32)((u8 *)&gbv + 0x145), 1, 1, 0, 100 },
    { "gbv.MayorLaunch", 0x12, (u32)((u8 *)&gbv + 0x128), 2, 1, 1, 500 },
    { "win_criteria.Criteria", 0x12, (u32)win_criteria, 22, 10, 0, 50 },
    { "win_criteria.MaxMin", 0x12, (u32)((u8 *)win_criteria + 0x2), 22, 10, 0, 1 },
    { "win_criteria.Value", 0xc, (u32)((u8 *)win_criteria + 0x4), 22, 10, -9000000, 9000000 },
    { "win_criteria.Bound", 0xc, (u32)((u8 *)win_criteria + 0x8), 22, 10, -9000000, 9000000 },
    { "win_criteria.Group", 0x12, (u32)((u8 *)win_criteria + 0xc), 22, 10, 0, 50 },
    { "lose_criteria.Criteria", 0x12, (u32)lose_criteria, 22, 10, 0, 50 },
    { "lose_criteria.MaxMin", 0x12, (u32)((u8 *)lose_criteria + 0x2), 22, 10, 0, 1 },
    { "lose_criteria.Value", 0xc, (u32)((u8 *)lose_criteria + 0x4), 22, 10, -5000000, 5000000 },
    { "lose_criteria.Bound", 0xc, (u32)((u8 *)lose_criteria + 0x8), 22, 10, -5000000, 5000000 },
    { "lose_criteria.Group", 0x12, (u32)((u8 *)lose_criteria + 0xc), 22, 10, 0, 50 },
    { "net_criteria.Criteria", 0x12, (u32)net_criteria, 12, 20, 0, 50 },
    { "net_criteria.Value", 0xc, (u32)((u8 *)net_criteria + 0x4), 12, 20, -5000000, 5000000 },
    { "net_criteria.Month", 0x12, (u32)((u8 *)net_criteria + 0x8), 12, 20, 0, 120 },
    { "net_criteria.TimeToDo", 0x12, (u32)((u8 *)net_criteria + 0xa), 12, 20, 0, 120 },
    { "popn.Month", 0x12, (u32)popn, 4, 30, 0, 500 },
    { "popn.Change", 0xa, (u32)((u8 *)popn + 0x2), 4, 30, -50, 50 },
    { "staff_levels.Month", 0x12, (u32)staff_levels, 19, 30, 0, 500 },
    { "staff_levels.Nurses", 0x12, (u32)((u8 *)staff_levels + 0x2), 19, 30, 0, 30 },
    { "staff_levels.Doctors", 0x12, (u32)((u8 *)staff_levels + 0x4), 19, 30, 0, 30 },
    { "staff_levels.Handymen", 0x12, (u32)((u8 *)staff_levels + 0x6), 19, 30, 0, 30 },
    { "staff_levels.Receptionists", 0x12, (u32)((u8 *)staff_levels + 0x8), 19, 30, 0, 30 },
    { "staff_levels.ShrkRate", 0x11, (u32)((u8 *)staff_levels + 0xa), 19, 30, 1, 255 },
    { "staff_levels.SurgRate", 0x11, (u32)((u8 *)staff_levels + 0xb), 19, 30, 0, 255 },
    { "staff_levels.RschRate", 0x11, (u32)((u8 *)staff_levels + 0xc), 19, 30, 0, 255 },
    { "staff_levels.ConsRate", 0x11, (u32)((u8 *)staff_levels + 0xd), 19, 30, 0, 255 },
    { "staff_levels.JrRate", 0x11, (u32)((u8 *)staff_levels + 0xe), 19, 30, 0, 255 },
    { "staff_levels.Seed", 0x14, (u32)((u8 *)staff_levels + 0xf), 19, 30, 0, 9999999 },
    { "emergency_control.StartMonth", 0x12, (u32)emergency_control, 10, 20, 0, 500 },
    { "emergency_control.EndMonth", 0x12, (u32)((u8 *)emergency_control + 0x2), 10, 20, 0, 500 },
    { "emergency_control.Min", 0x11, (u32)((u8 *)emergency_control + 0x4), 10, 20, 0, 20 },
    { "emergency_control.Max", 0x11, (u32)((u8 *)emergency_control + 0x5), 10, 20, 0, 20 },
    { "emergency_control.Illness", 0x11, (u32)((u8 *)emergency_control + 0x6), 10, 20, 0, 36 },
    { "emergency_control.PercWin", 0x11, (u32)((u8 *)emergency_control + 0x7), 10, 20, 0, 100 },
    { "emergency_control.Bonus", 0x12, (u32)((u8 *)emergency_control + 0x8), 10, 20, 0, 20000 },
    { "quake_control.StartMonth", 0x12, (u32)quake_control, 6, 16, 0, 500 },
    { "quake_control.EndMonth", 0x12, (u32)((u8 *)quake_control + 0x2), 6, 16, 0, 500 },
    { "quake_control.Severity", 0x12, (u32)((u8 *)quake_control + 0x4), 6, 16, 0, 20 },
    { "computer.Skill", 0x11, (u32)computer, 9, 20, 1, 5 },
    { "computer.StaffLevels", 0x11, (u32)((u8 *)computer + 0x1), 9, 20, 0, 9 },
    { "computer.Luck", 0x11, (u32)((u8 *)computer + 0x2), 9, 20, 0, 4 },
    { "computer.Speed", 0x11, (u32)((u8 *)computer + 0x3), 9, 20, 0, 100 },
    { "computer.Comfort", 0x11, (u32)((u8 *)computer + 0x4), 9, 20, 0, 9 },
    { "computer.GuessAt", 0x11, (u32)((u8 *)computer + 0x5), 9, 20, 0, 100 },
    { "computer.Playing", 0x11, (u32)((u8 *)computer + 0x8), 9, 20, 0, 1 },
    { "awards_trophies.RatKillsAbsolute", 0x11, (u32)&awards_trophies, 1, 1, 0, 255 },
    { "awards_trophies.RatKillsAbsoluteBonus", 0x12, (u32)((u8 *)&awards_trophies + 0x1), 2, 1, 0, 60000 },
    { "awards_trophies.CansofCoke", 0x11, (u32)((u8 *)&awards_trophies + 0x3), 1, 1, 0, 255 },
    { "awards_trophies.CansofCokeBonus", 0x12, (u32)((u8 *)&awards_trophies + 0x4), 2, 1, 0, 60000 },
    { "awards_trophies.Reputation", 0xa, (u32)((u8 *)&awards_trophies + 0x6), 2, 1, 0, 1000 },
    { "awards_trophies.TrophyReputationBonus", 0x12, (u32)((u8 *)&awards_trophies + 0x8), 2, 1, 0, 60000 },
    { "awards_trophies.Plant", 0x11, (u32)((u8 *)&awards_trophies + 0xa), 1, 1, 0, 100 },
    { "awards_trophies.PlantBonus", 0x11, (u32)((u8 *)&awards_trophies + 0xb), 1, 1, 0, 255 },
    { "awards_trophies.TrophyStaffHappiness", 0x11, (u32)((u8 *)&awards_trophies + 0xc), 1, 1, 0, 100 },
    { "awards_trophies.TrophyStaffHappinessBonus", 0x11, (u32)((u8 *)&awards_trophies + 0xd), 1, 1, 0, 255 },
    { "awards_trophies.RatKillsPercentage", 0x11, (u32)((u8 *)&awards_trophies + 0xe), 1, 1, 0, 100 },
    { "awards_trophies.RatKillsPercentageBonus", 0x12, (u32)((u8 *)&awards_trophies + 0xf), 2, 1, 0, 60000 },
    { "awards_trophies.TrophyCuresBonus", 0x12, (u32)((u8 *)&awards_trophies + 0x11), 2, 1, 0, 60000 },
    { "awards_trophies.TrophyDeathBonus", 0x12, (u32)((u8 *)&awards_trophies + 0x13), 2, 1, 0, 60000 },
    { "awards_trophies.TrophyMayorBonus", 0x11, (u32)((u8 *)&awards_trophies + 0x16), 1, 1, 0, 255 },
    { "awards_trophies.TrophyMayor", 0x11, (u32)((u8 *)&awards_trophies + 0x15), 1, 1, 0, 100 },
    { "awards_trophies.CuresAward", 0x11, (u32)((u8 *)&awards_trophies + 0x17), 1, 1, 0, 255 },
    { "awards_trophies.CuresPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x18), 1, 1, 0, 255 },
    { "awards_trophies.DeathsAward", 0x11, (u32)((u8 *)&awards_trophies + 0x19), 1, 1, 0, 255 },
    { "awards_trophies.DeathsPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x1a), 1, 1, 0, 255 },
    { "awards_trophies.PopulationPercentageAward", 0x11, (u32)((u8 *)&awards_trophies + 0x1b), 1, 1, 0, 100 },
    { "awards_trophies.PopulationPercentagePoor", 0x11, (u32)((u8 *)&awards_trophies + 0x1c), 1, 1, 0, 100 },
    { "awards_trophies.CuresVDeathsAward", 0x11, (u32)((u8 *)&awards_trophies + 0x1d), 1, 1, 0, 255 },
    { "awards_trophies.CuresVDeathsPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x1e), 1, 1, 0, 255 },
    { "awards_trophies.ReputationAward", 0x12, (u32)((u8 *)&awards_trophies + 0x1f), 2, 1, 0, 60000 },
    { "awards_trophies.ReputationPoor", 0x12, (u32)((u8 *)&awards_trophies + 0x21), 2, 1, 0, 60000 },
    { "awards_trophies.HospValueAward", 0x14, (u32)((u8 *)&awards_trophies + 0x23), 4, 1, 0, 2000000 },
    { "awards_trophies.HospValuePoor", 0x14, (u32)((u8 *)&awards_trophies + 0x27), 4, 1, 0, 2000000 },
    { "awards_trophies.CleanlinessAward", 0x11, (u32)((u8 *)&awards_trophies + 0x2b), 1, 1, 1, 255 },
    { "awards_trophies.CleanlinessPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x2c), 1, 1, 1, 255 },
    { "awards_trophies.EmergencyAward", 0x11, (u32)((u8 *)&awards_trophies + 0x2d), 1, 1, 0, 255 },
    { "awards_trophies.EmergencyPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x2e), 1, 1, 0, 255 },
    { "awards_trophies.StaffHappinessAward", 0x11, (u32)((u8 *)&awards_trophies + 0x2f), 1, 1, 0, 255 },
    { "awards_trophies.StaffHappinessPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x30), 1, 1, 0, 255 },
    { "awards_trophies.PeepHappinessAward", 0x11, (u32)((u8 *)&awards_trophies + 0x31), 1, 1, 0, 255 },
    { "awards_trophies.PeepHappinessPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x32), 1, 1, 0, 255 },
    { "awards_trophies.WaitingTimesAward", 0x11, (u32)((u8 *)&awards_trophies + 0x33), 1, 1, 0, 255 },
    { "awards_trophies.WaitingTimesPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x34), 1, 1, 0, 255 },
    { "awards_trophies.WellKeptTechAward", 0x11, (u32)((u8 *)&awards_trophies + 0x35), 1, 1, 0, 255 },
    { "awards_trophies.WellKeptTechPoor", 0x11, (u32)((u8 *)&awards_trophies + 0x36), 1, 1, 0, 255 },
    { "awards_trophies.CuresBonus", 0xa, (u32)((u8 *)&awards_trophies + 0x3f), 2, 1, -32000, 32000 },
    { "awards_trophies.CuresPenalty", 0xa, (u32)((u8 *)&awards_trophies + 0x41), 2, 1, -32000, 32000 },
    { "awards_trophies.DeathsBonus", 0xa, (u32)((u8 *)&awards_trophies + 0x43), 2, 1, -32000, 32000 },
    { "awards_trophies.DeathsPenalty", 0xa, (u32)((u8 *)&awards_trophies + 0x45), 2, 1, -32000, 32000 },
    { "awards_trophies.PopulationPercentageBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x47), 1, 1, -127, 127 },
    { "awards_trophies.PopulationPercentagePenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x48), 1, 1, -127, 127 },
    { "awards_trophies.CuresVDeathsBonus", 0xa, (u32)((u8 *)&awards_trophies + 0x49), 2, 1, -32000, 32000 },
    { "awards_trophies.CuresVDeathsPenalty", 0xa, (u32)((u8 *)&awards_trophies + 0x4b), 2, 1, -32000, 32000 },
    { "awards_trophies.AwardReputationBonus", 0xa, (u32)((u8 *)&awards_trophies + 0x4d), 2, 1, -32000, 32000 },
    { "awards_trophies.AwardReputationPenalty", 0xa, (u32)((u8 *)&awards_trophies + 0x4f), 2, 1, -32000, 32000 },
    { "awards_trophies.HospValueBonus", 0xa, (u32)((u8 *)&awards_trophies + 0x51), 2, 1, -127, 127 },
    { "awards_trophies.HospValuePenalty", 0xa, (u32)((u8 *)&awards_trophies + 0x53), 2, 1, -127, 127 },
    { "awards_trophies.CleanlinessBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x55), 1, 1, -127, 127 },
    { "awards_trophies.CleanlinessPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x56), 1, 1, -127, 127 },
    { "awards_trophies.EmergencyBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x57), 1, 1, -127, 127 },
    { "awards_trophies.EmergencyPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x58), 1, 1, -127, 127 },
    { "awards_trophies.AwardStaffHappinessBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x59), 1, 1, -127, 127 },
    { "awards_trophies.AwardStaffHappinessPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x5a), 1, 1, -127, 127 },
    { "awards_trophies.PeepHappinessBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x5b), 1, 1, -127, 127 },
    { "awards_trophies.PeepHappinessPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x5c), 1, 1, -127, 127 },
    { "awards_trophies.WaitingTimesBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x5d), 1, 1, -127, 127 },
    { "awards_trophies.WaitingTimesPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x5e), 1, 1, -127, 127 },
    { "awards_trophies.WellKeptTechBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x5f), 1, 1, -127, 127 },
    { "awards_trophies.WellKeptTechPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x60), 1, 1, -127, 127 },
    { "awards_trophies.NewTechAward", 0x14, (u32)((u8 *)&awards_trophies + 0x37), 4, 1, 0, 4000000 },
    { "awards_trophies.NewTechPoor", 0x14, (u32)((u8 *)&awards_trophies + 0x3b), 4, 1, 0, 4000000 },
    { "awards_trophies.ResearchBonus", 0x9, (u32)((u8 *)&awards_trophies + 0x61), 1, 1, -127, 127 },
    { "awards_trophies.ResearchPenalty", 0x9, (u32)((u8 *)&awards_trophies + 0x62), 1, 1, -127, 127 },
    { "non_visuals", 0x12, (u32)non_visuals, 2, 20, 0, 10000 },
    { "visuals", 0x12, (u32)visuals, 2, 14, 0, 10000 },
    { "visuals_available", 0x12, (u32)visuals_available, 2, 14, 0, 1000 },
    { "gbv.AbilityThreshold", 0x12, (u32)((u8 *)&gbv + 0x146), 2, 3, 0, 1000 },
    { "gbv.RschImproveCostPercent", 0x11, (u32)((u8 *)&gbv + 0x15a), 1, 1, 0, 100 },
    { "gbv.RschImproveIncrementPercent", 0x11, (u32)((u8 *)&gbv + 0x15b), 1, 1, 0, 100 },
    { "gbv.RemoveRatHoleChance", 0x14, (u32)((u8 *)&gbv + 0x15c), 4, 1, 1, 10000 }
};
extern char *file_buffer;
extern char current_line[];

extern "C" {
extern s32 file_len;
extern char var_front[];
extern char var_back[];
extern u8 __IsTable[];
#pragma aux __IsTable "*"

int strcmp(const char *a, const char *b);
char *strncpy(char *dst, const char *src, u32 n);
char *strncat(char *dst, const char *src, u32 n);
char *strstr(const char *s, const char *sub);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strtok(char *s, const char *delim);
s32 atol(const char *s);
void fn_000c93bf(char *dst, char *src);
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"
}

s32 variable_address(char *name);
u16 array_index(char *name);
u8 number_of_members(char *name);
void strip_formatting(char *s);
char *read_line_from_buf(char *line, s32 max, char *src);

#define isspace(c) (__IsTable[(u8)((c) + 1)] & 2)

// FUNCTION: 0x00099020
s8 assign_value(char *name, u32 value, u16 index)
{
    s32 var_index;
    u32 addr;
    char buf[0x80];

    var_index = 0;
    var_index = variable_address(name);
    if (var_index >= 0 && var_index < 0xcb) {
        if (index <= balance_variables[var_index].max_index) {
            addr = balance_variables[var_index].address;
            addr += index * balance_variables[var_index].stride;
            if ((balance_variables[var_index].flags & 1) && (balance_variables[var_index].flags & 0x10)) {
                if ((s32)value >= balance_variables[var_index].min && value <= balance_variables[var_index].max) {
                    *(u8 *)addr = value;
                    return 1;
                }
                sprintf(buf, "Value %d for %s - Outside Specified Bounds (%d,%d)\n", value, balance_variables[var_index].name,
                    balance_variables[var_index].min, balance_variables[var_index].max);
                return -1;
            }
            if ((balance_variables[var_index].flags & 1) && (balance_variables[var_index].flags & 8)) {
                if ((s32)value >= balance_variables[var_index].min && (s32)value <= balance_variables[var_index].max) {
                    *(u8 *)addr = value;
                    return 1;
                }
                sprintf(buf, "Value %d for %s - Outside Specified Bounds (%d,%d)\n", value, balance_variables[var_index].name,
                    balance_variables[var_index].min, balance_variables[var_index].max);
                return -1;
            }
            if ((balance_variables[var_index].flags & 2) && (balance_variables[var_index].flags & 0x10)) {
                if ((s32)value >= balance_variables[var_index].min && value <= balance_variables[var_index].max) {
                    *(u16 *)addr = value;
                    return 1;
                }
                sprintf(buf, "Value %d for %s - Outside Specified Bounds (%d,%d)\n", value, balance_variables[var_index].name,
                    balance_variables[var_index].min, balance_variables[var_index].max);
                return -1;
            }
            if ((balance_variables[var_index].flags & 2) && (balance_variables[var_index].flags & 8)) {
                s32 v = value;
                if ((s32)value >= balance_variables[var_index].min && (s32)value <= balance_variables[var_index].max) {
                    *(u16 *)addr = value;
                    return 1;
                }
                sprintf(buf, "Value %d for %s - Outside Specified Bounds (%d,%d)\n", value, balance_variables[var_index].name,
                    balance_variables[var_index].min, balance_variables[var_index].max);
                return -1;
            }
            if ((balance_variables[var_index].flags & 4) && (balance_variables[var_index].flags & 0x10)) {
                if ((s32)value >= balance_variables[var_index].min && value <= balance_variables[var_index].max) {
                    *(u32 *)addr = value;
                    return 1;
                }
                sprintf(buf, "Value %d for %s - Outside Specified Bounds (%d,%d)\n", value, balance_variables[var_index].name,
                    balance_variables[var_index].min, balance_variables[var_index].max);
                return -1;
            }
            if ((balance_variables[var_index].flags & 4) && (balance_variables[var_index].flags & 8)) {
                s32 v = value;
                if ((s32)value >= balance_variables[var_index].min && (s32)value <= balance_variables[var_index].max) {
                    *(u32 *)addr = value;
                    return 1;
                }
                sprintf(buf, "Value %d for %s - Outside Specified Bounds (%d,%d)\n", value, balance_variables[var_index].name,
                    balance_variables[var_index].min, balance_variables[var_index].max);
                return -1;
            }
            sprintf(buf, "TYPE NOT Recognised - %s\n", name);
            return -1;
        } else {
            sprintf(buf, "ERROR - Index too big. Must be between less than %d for %s\n", balance_variables[var_index].max_index, balance_variables[var_index].name);
            return -1;
        }
    } else {
        sprintf(buf, "ERROR - Table checking variable - %s not in the table\n", name);
        return -1;
    }
}

// FUNCTION: 0x00099494
s32 variable_address(char *name)
{
    u32 i;

    i = 0;
    while (i < 0xcb) {
        if (strcmp(balance_variables[i].name, name) == 0)
            return i;
        i++;
    }
    return -1;
}

// FUNCTION: 0x000994f0
void read_variable_file(char *name)
{
    u8 len;
    u8 members;
    u16 idx;
    s32 value;
    char *start;
    char *value_ptr;
    char *cursor;
    char var[0x80];
    char num[0x10];
    char full[0x80];

    len = 0;
    value = 0;
    file_len = LbFileLengthRnc(name);
    if (file_len == -1) {
        hosp_dos_printf("rvf failed to read %s\n", name);
        return;
    }
    file_buffer = (char *)LbMemoryAlloc(file_len + 1);
    if (file_buffer == 0) {
        hosp_dos_printf("rvf failed to alloc %d\n", file_len + 1);
        return;
    }
    if (LbFileLoadAt(name, file_buffer) == -1) {
        hosp_dos_printf("rvf failed to load %s\n", name);
        LbMemoryFree(file_buffer);
        return;
    }
    *(file_buffer + file_len) = 0xff;
    LbMemorySet(current_line, 0, 0x100);
    read_line_from_buf(0, 0, file_buffer);
    while (read_line_from_buf(current_line, 0x100, file_buffer)) {
        cursor = strstr(current_line, "#");
        if (cursor) {
            LbMemorySet(var, 0, 0x80);
            LbMemorySet(num, 0, 0x10);
            LbMemorySet(var_front, 0, 0x80);
            cursor++;
            start = cursor;
            len = 0;
            while (!isspace(*cursor) && *cursor) {
                len++;
                cursor++;
            }
            strncpy(var, start, len);
            idx = array_index(var);
            value_ptr = cursor;
            while (isspace(*value_ptr))
                value_ptr++;
            strncpy(var_front, var, fn_000cd716(var) + 1);
            if (strchr(var, '.')) {
                LbMemorySet(full, 0, 0x80);
                LbMemorySet(var_back, 0, 0x80);
                strtok(var_front, ".");
                cursor = strchr(var, '.');
                members = number_of_members(var);
                while (members-- > 0) {
                    start = cursor++;
                    len = 1;
                    while (!isspace(*cursor) && *cursor && *cursor != '.') {
                        len++;
                        cursor++;
                    }
                    strncpy(var_back, start, len);
                    strncpy(full, var_front, fn_000cd716(var_front) + 1);
                    fn_000c93bf(full, var_back);
                    start = value_ptr;
                    len = 0;
                    while (!isspace(*value_ptr) && *value_ptr) {
                        len++;
                        value_ptr++;
                    }
                    strncpy(num, start, len);
                    num[len] = 0;
                    value = atol(num);
                    if (assign_value(full, value, idx) == -1)
                        read_line_from_buf(0, 1, 0);
                    while (isspace(*value_ptr))
                        value_ptr++;
                    LbMemorySet(var_back, 0, 0x80);
                    LbMemorySet(full, 0, 0x80);
                    LbMemorySet(num, 0, 0x10);
                }
                LbMemorySet(var_front, 0, 0x80);
            } else {
                start = value_ptr;
                len = 0;
                while (!isspace(*value_ptr) && *value_ptr) {
                    len++;
                    value_ptr++;
                }
                strncpy(num, start, len);
                num[len] = 0;
                value = atol(num);
                strip_formatting(var);
                if (assign_value(var, value, idx) == -1)
                    read_line_from_buf(0, 1, 0);
            }
        }
        LbMemorySet(current_line, 0, 0x100);
        LbMemorySet(var, 0, 0x80);
        LbMemorySet(num, 0, 0x10);
        LbMemorySet(var_front, 0, 0x80);
        LbMemorySet(var_back, 0, 0x80);
        LbMemorySet(full, 0, 0x80);
    }
    LbMemoryFree(file_buffer);
    file_buffer = 0;
}

// FUNCTION: 0x00099a00
u16 array_index(char *name)
{
    u8 len;
    char *start;
    s32 idx;
    char *cursor;
    char buf[0x80];
    char buf2[0x80];
    char buf3[0x80];
    char num[8];

    len = 0;
    LbMemorySet(buf, 0, 0x80);
    LbMemorySet(buf2, 0, 0x80);
    LbMemorySet(buf3, 0, 0x80);
    LbMemorySet(num, 0, 8);
    strncpy(buf, name, fn_000cd716(name) + 1);
    cursor = strstr(name, "[");
    if (cursor) {
        strtok(buf, "[");
        strncpy(buf2, name, fn_000cd716(name) + 1);
        cursor = strstr(buf2, "[");
        ++cursor;
        start = cursor;
        while (*cursor != ']') {
            cursor++;
            len++;
        }
        strncpy(num, start, len);
        idx = atoi(num);
        len = 0;
        ++cursor;
        start = cursor;
        while (*cursor) {
            len++;
            cursor++;
        }
        strncpy(buf3, start, len);
        strncat(buf, buf3, fn_000cd716(buf3) + 1);
        fn_000bbff0(name, buf);
        return idx;
    }
    return 0;
}

// FUNCTION: 0x00099b8c
u8 number_of_members(char *name)
{
    u8 count;
    char *cursor;
    char *last;

    count = 1;
    cursor = strchr(name, '.');
    last = strrchr(name, '.');
    while (cursor != last) {
        if (*cursor == '.')
            count++;
        cursor++;
    }
    return count;
}

// FUNCTION: 0x00099bf4
void strip_formatting(char *name)
{
    u8 len;
    char *cursor;

    len = 0;
    cursor = strstr(current_line, "\t");
    if (cursor != 0 && (cursor = strstr(current_line, " ")) != 0) {
        for (cursor = name; *cursor != '\t' && *cursor != ' ';) {
            cursor++;
            len++;
        }
        *cursor = 0;
    }
}
