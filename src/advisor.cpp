#include "hosp.h"

struct Advice {
    u16 id;
    u16 duration;
    u16 repeat_weight;
    u8 times_given;
    u8 f7;
    u32 time;
    u32 fc;
};

Advice advice[] = {
    {0, 0, 0},
    {0x6a, 0xfa, 0},
    {0x1c, 0xfa, 0x1e},
    {0x87, 0xfa, 0x1e},
    {0x8d, 0xfa, 0x1e},
    {0x8a, 0xfa, 0x1e},
    {0x8b, 0xfa, 0x1e},
    {0x8c, 0xfa, 0x1e},
    {0x15, 0xfa, 0},
    {0x4d, 0xfa, 0},
    {0x48, 0xfa, 0x14},
    {0x5e, 0xfa, 0x14},
    {0x78, 0xfa, 0x14},
    {0x3, 0xfa, 0x14},
    {0x6f, 0xfa, 0x1e},
    {0x72, 0xfa, 0x1e},
    {0x75, 0xfa, 0x14},
    {0x4, 0x82, 0},
    {0x5, 0x82, 0},
    {0xc, 0xfa, 0},
    {0x58, 0xfa, 0x14},
    {0x7b, 0xfa, 0},
    {0x6, 0xfa, 0x14},
    {0x63, 0xfa, 0x14},
    {0x7, 0xfa, 0x14},
    {0x8, 0xfa, 0x14},
    {0x9, 0xfa, 0x14},
    {0xa, 0xfa, 0x14},
    {0x14, 0xfa, 0x14},
    {0x76, 0xfa, 0x14},
    {0x77, 0xfa, 0x14},
    {0xb, 0xfa, 0x14},
    {0xc, 0xfa, 0x14},
    {0xd, 0xfa, 0x14},
    {0xe, 0xfa, 0x14},
    {0xf, 0xfa, 0x14},
    {0x10, 0xfa, 0x14},
    {0x11, 0xfa, 0x14},
    {0x12, 0xfa, 0x14},
    {0x13, 0xfa, 0x14},
    {0x86, 0xfa, 0},
    {0x7a, 0xfa, 0},
    {0x79, 0x15e, 0},
    {0x44, 0xfa, 0},
    {0x89, 0x15e, 0},
    {0x16, 0x15e, 0},
    {0x17, 0xfa, 0},
    {0x18, 0xfa, 0},
    {0x19, 0xfa, 0},
    {0x1a, 0xfa, 0},
    {0x1b, 0xfa, 0},
    {0x45, 0xfa, 0x14},
    {0x46, 0xfa, 0x14},
    {0x88, 0xfa, 0x14},
    {0x41, 0xfa, 0x14},
    {0x42, 0xfa, 0x14},
    {0x43, 0xfa, 0x14},
    {0x64, 0xfa, 0x14},
    {0x65, 0xfa, 0x14},
    {0x4b, 0xfa, 0x14},
    {0x5c, 0xfa, 0x14},
    {0x5a, 0xfa, 0xf},
    {0x5b, 0xfa, 0x14},
    {0x4a, 0xfa, 0x14},
    {0x49, 0xfa, 0x14},
    {0x66, 0x96, 0x14},
    {0x54, 0x15e, 0x14},
    {0x67, 0x96, 0x14},
    {0x55, 0x96, 0x14},
    {0x56, 0x96, 0x14},
    {0x50, 0x64, 0x14},
    {0x53, 0x15e, 0x14},
    {0x4f, 0x96, 0x14},
    {0x57, 0x64, 0x14},
    {0x4e, 0x15e, 0x14},
    {0x5d, 0x64, 0x14},
    {0x47, 0x4b, 0x14},
    {0x52, 0x4b, 0x14},
    {0x4c, 0x64, 0x14},
    {0x51, 0x64, 0x14},
    {0x69, 0x64, 0x1e},
    {0x68, 0x64, 0x1e},
    {0x6e, 0x4b, 0x14},
    {0x6b, 0xfa, 0x1e},
    {0, 0, 0},
};
extern WinLose win_criteria[];
extern WinLose lose_criteria[];
extern char **advisor_text;
extern const u8 staff_unhappy[];
extern const u16 advisor_anims[];

s8 tutorial(void);
void advisor_character(void);
void advisor_text_box(void);
u8 find_advice(const u16 id);
void reset_all_count(void);
u8 is_relevant_criterion(const u8 criterion, const WinLose *const table);
void mouse_advice(void);
void forced_advice(void);
void done_announcement(const u16 type);
u8 advisor_toilets(void);
u8 advisor_plants(void);
u8 advisor_old_hardware(void);
u8 advisor_thirsty_peeps(void);
u8 advisor_peeps_unhappy(void);
u8 advisor_peeps_warmth(void);
u8 advisor_litter_problem(void);
u8 advisor_benches(void);
u8 advisor_dead_plants(void);
u8 advisor_staff_tired(void);
u8 advisor_staff_warmth(void);

extern "C" {
extern char **object_names;
extern char **illness_text;
extern char **research_names;
extern char **real_room_names;
extern u8 D_00156910;
extern char **forced_advice_text;
extern char **mouse_advice_text;
}

// Runs the advisor for a frame, if enabled (myplayer.advisor_flags[20]) and not
// suppressed: in the tutorial shows the tutorial text; otherwise lets the
// current advice expire after its time (advice[].duration), then picks the first
// advice entry not given too often or too recently whose condition holds
// (find_advice), animates the advisor and draws the speech bubble, and sends
// the advisor away once the advice is done.
// FUNCTION: 0x0005dbb0
void display_advisor(void)
{
    u8 stale;
    u16 i;
    u16 elapsed;

    if (!gamevar.rat_fest_level) {
    if (!debug.suppress_help) {
    if (!debug.advisor_off) {
    if (myplayer.advisor_flags[20]) {
        if (advisor.flags & 1) {
            if (tutorial() && advisor.text[0]) {
                advisor.advice_id = 1;
                advisor_character();
                if (advisor_up_states[advisor.anim_state]) {
                    advisor_text_box();
                    D_00156910 = 1;
                }
            } else {
                return;
            }
        } else {
            stale = 0;
            if (advisor.advice_id) {
                if (resvars.game_mode != SCREEN_MODE_1 || (crequester.id == REQ_NONE || crequester.id == REQ_EDIT_ROOM)) {
                    elapsed = actual_game_turn - advice[advisor.advice_id].time;
                    if (elapsed > 0x64 || othervar.dismiss_advisor == 1)
                        stale = 1;
                    if (elapsed >= advice[advisor.advice_id].duration || othervar.dismiss_advisor == 1)
                        advisor.advice_id = 0;
                } else {
                    advice[advisor.advice_id].time = actual_game_turn - advice[advisor.advice_id].duration / 4;
                }
            }
            if (advisor.advice_id == 0 || stale == 1) {
                for (i = 1; advice[i].id; i++) {
                    if (actual_game_turn > advice[i].time) {
                        if ((advice[i].times_given * advice[i].repeat_weight << 10) / (actual_game_turn - advice[i].time) < 10) {
                            if (find_advice(i) == 1) {
                                advisor.rewrap = 1;
                                break;
                            }
                        }
                    }
                }
            }
            advisor_character();
            if (advisor_up_states[advisor.anim_state]) {
                if (advisor.advice_id && !othervar.dismiss_advisor) {
                    advisor_text_box();
                    D_00156910 = 1;
                } else if (--advisor.time_left <= 0 || othervar.dismiss_advisor) {
                    othervar.dismiss_advisor = 0;
                    advisor.time_left = -1;
                    advisor.anim_state = next_advisor_state[advisor.anim_state];
                    advisor.anim_frame = start[advisor_anims[advisor.anim_state]].frame;
                    advisor.state_advanced = 0;
                } else if (!advisor.state_advanced) {
                    advisor.anim_state = next_advisor_state[advisor.anim_state];
                    advisor.anim_frame = start[advisor_anims[advisor.anim_state]].frame;
                    advisor.state_advanced = 1;
                    othervar.dismiss_advisor = 0;
                }
            }
        }
    }
    }
    }
    }
}

// Checks advice entry id (advice[] index): if its situation applies (from the
// statistics pass, announcements and the level's criteria), writes the advice
// text into advisor.text, makes it the current advice (advisor.advice_id) and records
// when it was given. Returns 1 if advice was written, else 0.
// FUNCTION: 0x0005defc
u8 find_advice(const u16 id)
{
    u8 f7;
    u16 found;
    u32 extra;
    s32 crit;
    s32 win_crit;
    s32 value;

    found = 0;
    extra = 0;
    f7 = 0;
    switch (advice[id].id) {
    case 0x6a:
        if (gamevar.quake_phase == 2 && !myplayer.advisor_flags[13]) {
            sprintf(advisor.text, advisor_text[0x6a]);
            myplayer.advisor_flags[13] = 1;
            found = 1;
        }
        break;
    case 0x4d:
        if (NRAdcounts.last_quake_size >= 8) {
            sprintf(advisor.text, advisor_text[0x4d], NRAdcounts.last_quake_size / 2);
            NRAdcounts.last_quake_size = 0;
            found = 1;
        }
        break;
    case 0x63:
        if (NRAdcounts.quake_damage_report) {
            sprintf(advisor.text, advisor_text[0x63], NRAdcounts.last_quake_size / 2, NRAdcounts.last_quake_size / 2);
            NRAdcounts.quake_damage_report = 0;
            found = 1;
        }
        break;
    case 0x5e:
        if (adcounts.crackups && advisor.has_patients) {
            if (adcounts.crackups == 1) {
                sprintf(advisor.text, advisor_text[0x5e]);
                adcounts.crackups = 0;
                found = 1;
            }
        }
        break;
    case 0x48:
        crit = is_relevant_criterion(2, lose_criteria);
        if (crit && (statvar.num_players > 1 ? 1 : myhosp->hospital_open)) {
            win_crit = is_relevant_criterion(2, win_criteria);
            if (win_crit && myhosp->balance - myhosp->loan < win_criteria[win_crit].value) {
                u8 i;

                for (i = 0; i < 4; i++) {
                    if (myhosp->balance < lose_criteria[crit].value + (i + 1) * fn_000cb2db(lose_criteria[crit].value) / 10
                        && myhosp->balance > lose_criteria[crit].value) {
                        sprintf(advisor.text, extra_advice[0x1c + i], myhosp->balance - lose_criteria[crit].value);
                        found = 1;
                        break;
                    }
                }
            }
        } else if (towns[myhosp->level].start_cash / 10 >= myhosp->balance && myhosp->loan == 0) {
            sprintf(advisor.text, advisor_text[0x48]);
            found = 1;
        } else if (towns[myhosp->level].start_cash / 10 >= myhosp->balance && myhosp->loan != 0 && myhosp->loan < myhosp->max_loan) {
            sprintf(advisor.text, extra_advice[0xf]);
            found = 1;
        }
        break;
    case 0x41:
        crit = is_relevant_criterion(5, lose_criteria);
        if (crit && (statvar.num_players > 1 ? 1 : myhosp->hospital_open) && myhosp->visitors && (advisor.event_flags_u32 & 0x80)) {
            if (lose_criteria[crit].value - myhosp->deaths * 100 / myhosp->visitors < 10) {
                sprintf(advisor.text, extra_advice[0x3a]);
                found = 1;
            } else if (lose_criteria[crit].value - myhosp->deaths * 100 / myhosp->visitors < 0x19) {
                sprintf(advisor.text, advisor_text[0x41], myhosp->visitors * lose_criteria[crit].value / 100 - myhosp->deaths);
                found = 1;
            }
            advisor.event_flags_u32 &= ~0x80;
        }
        break;
    case 0x42:
        crit = is_relevant_criterion(1, win_criteria);
        if (crit && (statvar.num_players > 1 ? 1 : myhosp->hospital_open) && mycontrol->month > 5) {
            if (myhosp->reputation > win_criteria[crit].value) {
                sprintf(advisor.text, extra_advice[0x3b], win_criteria[crit].value);
            } else if ((win_criteria[crit].value - myhosp->reputation) * 100 / win_criteria[crit].value < 10) {
                sprintf(advisor.text, extra_advice[0x3c], win_criteria[crit].value);
            } else {
                sprintf(advisor.text, advisor_text[0x42], myhosp->reputation - win_criteria[crit].value);
            }
            found = 1;
        }
        break;
    case 0x43:
        crit = is_relevant_criterion(6, win_criteria);
        if (crit && (statvar.num_players > 1 ? 1 : myhosp->hospital_open) && mycontrol->month > 6) {
            if (myhosp->hospital_value > win_criteria[crit].value) {
                sprintf(advisor.text, extra_advice[0x3d], win_criteria[crit].value);
                found = 1;
            } else if ((win_criteria[crit].value - myhosp->hospital_value) * 100 / win_criteria[crit].value < 10) {
                sprintf(advisor.text, extra_advice[0x3e], win_criteria[crit].value - myhosp->hospital_value);
                found = 1;
            } else if (myhosp->hospital_value < win_criteria[crit].value) {
                sprintf(advisor.text, advisor_text[0x43], win_criteria[crit].value);
                found = 1;
            }
        }
        break;
    case 0x14:
        if (myhosp->loan && myhosp->balance > myhosp->loan * 3) {
            sprintf(advisor.text, extra_advice[0x38]);
            found = 1;
        } else {
            crit = is_relevant_criterion(2, win_criteria);
            if (crit && (statvar.num_players > 1 ? 1 : myhosp->hospital_open)) {
                if (myhosp->balance - myhosp->loan > win_criteria[crit].value && (advisor.event_flags_u32 & 0x20)) {
                    sprintf(advisor.text, extra_advice[0x37], win_criteria[crit].value);
                    found = 1;
                } else if (myhosp->balance - myhosp->loan > win_criteria[crit].value - fn_000cb2db(win_criteria[crit].value) / 10
                           && myhosp->balance - myhosp->loan < win_criteria[crit].value && (advisor.event_flags_u32 & 0x20)) {
                    sprintf(advisor.text, advisor_text[0x14], win_criteria[crit].value - (myhosp->balance - myhosp->loan));
                    found = 1;
                }
            }
        }
        break;
    case 0x88:
        crit = is_relevant_criterion(4, win_criteria);
        if (crit && (statvar.num_players > 1 ? 1 : myhosp->hospital_open) && (advisor.event_flags_u32 & 0x40)
            && myhosp->cures > myhosp->level * 3) {
            if (myhosp->cures > win_criteria[crit].value && adcounts.lose_progress > 0x19 && advisor.has_patients) {
                sprintf(advisor.text, extra_advice[0x39], win_criteria[crit].value);
                found = 1;
            } else if (myhosp->cures < win_criteria[crit].value) {
                sprintf(advisor.text, advisor_text[0x88], win_criteria[crit].value - myhosp->cures);
                found = 1;
            }
            advisor.event_flags_u32 &= ~0x40;
        }
        break;
    case 0x78:
        if (adcounts.worst_machine && advisor.has_patients) {
            sprintf(advisor.text, advisor_text[0x78], object_names[adcounts.worst_machine]);
            found = 1;
        }
        break;
    case 3:
        if (adcounts.machines > 0 && advisor.has_patients) {
            if (advisor_old_hardware())
                found = 1;
        }
        break;
    case 4:
        if (advisor.forced_advice_id_s) {
            forced_advice();
            found = 1;
        }
        break;
    case 5:
        if (advisor.mouse_advice_id) {
            mouse_advice();
            found = 1;
        }
        break;
    case 6:
        break;
    case 7:
        if (adcounts.patients > 3 && advisor.has_patients) {
            if (adcounts.doctor_wait_count > 0 && adcounts.idle_doctors == 0) {
                if ((adcounts.doctor_wait_sum << 8) / adcounts.doctor_wait_count > (gbv.leave_max << 8) / 2) {
                    sprintf(advisor.text, advisor_text[7]);
                    found = 1;
                }
            }
        }
        break;
    case 8:
        if (myhosp->staff_by_kind[1] >= 6 && adcounts.idle_doctors > myhosp->staff_by_kind[1] / 3 && advisor.has_patients) {
            sprintf(advisor.text, advisor_text[8]);
            found = 1;
        }
        break;
    case 0x64:
        if (adcounts.patients > 3 && advisor.has_patients) {
            if (adcounts.nurse_wait_count > 0 && adcounts.idle_nurses == 0) {
                if ((adcounts.nurse_wait_sum << 8) / adcounts.nurse_wait_count > (gbv.leave_max << 8) / 2) {
                    sprintf(advisor.text, advisor_text[0x64]);
                    found = 1;
                }
            }
        }
        break;
    case 0x65:
        if (myhosp->staff_by_kind[0] >= 4 && adcounts.idle_nurses > myhosp->staff_by_kind[0] / 4 && advisor.has_patients) {
            sprintf(advisor.text, advisor_text[0x65]);
            found = 1;
        }
        break;
    case 9:
        if (mycontrol->month > 4 && (statvar.num_players > 1 ? 1 : myhosp->hospital_open) && mycontrol->history_visitors_now == 0
            && advice[id].times_given < 1) {
            sprintf(advisor.text, advisor_text[9]);
            found = 1;
        }
        break;
    case 0xa:
        if (mycontrol->month > 4 && (statvar.num_players > 1 ? 1 : myhosp->hospital_open) && advice[id].times_given < 1
            && mycontrol->history_cures_now == 0 && mycontrol->history_visitors_now > 0 && myhosp->visitors > othervar.patients_left_count) {
            sprintf(advisor.text, advisor_text[10]);
            found = 1;
        }
        break;
    case 0xb:
        if (adcounts.patients > 5 && advisor.has_patients) {
            if ((adcounts.waiting_sum << 8) / adcounts.patients > (gbv.leave_max << 9) / 4) {
                value = ((adcounts.impatience_sum - adcounts.waiting_sum) << 8) / gbv.leave_max;
                if (value * 10 < 0x100)
                    sprintf(advisor.text, advisor_text[11]);
                else
                    sprintf(advisor.text, extra_advice[0x32]);
                found = 1;
            }
        }
        break;
    case 0x8d:
        if (!myplayer.advisor_flags[14] && (advisor.event_flags_u32 & 8)) {
            sprintf(advisor.text, advisor_situations[11]);
            myplayer.advisor_flags[14] = 1;
            advisor.event_flags_u32 &= ~8;
            found = 1;
        }
        break;
    case 0xd:
        if (advisor.event_flags_u32 & 8) {
            if (othervar.contagious_count >= 4) {
                sprintf(advisor.text, extra_advice[1]);
                found = 1;
            } else if (othervar.contagious_count >= 8) {
                sprintf(advisor.text, extra_advice[2]);
                found = 1;
            }
        }
        break;
    case 0xe:
        if (adcounts.patients > 5 && advisor.has_patients) {
            if (advisor_peeps_unhappy())
                found = 1;
        }
        break;
    case 0xf:
        if (adcounts.patients > 5 && advisor.has_patients) {
            if (advisor_thirsty_peeps())
                found = 1;
        }
        break;
    case 0x58:
        break;
    case 0x10:
        if (myhosp->staff_count > 3 && advisor.has_patients) {
            if (advisor_staff_tired()) {
            }
        }
        break;
    case 0x11:
        if (adcounts.patients > 10 && advisor.has_patients) {
            if (advisor_peeps_warmth())
                found = 1;
        }
        break;
    case 0x12:
        if (myhosp->staff_count > 10 && advisor.has_patients) {
            if (advisor_staff_warmth())
                found = 1;
        }
        break;
    case 0x13:
        if (adcounts.patients > 10 && advisor.has_patients) {
            if (advisor_toilets())
                found = 1;
        }
        break;
    case 0xc:
        value = gamevar.announce[ANN_STUCK][0];
        if (value) {
            sprintf(advisor.text, advisor_text[12]);
            done_announcement(ANN_STUCK);
            found = 1;
        }
        break;
    case 0x59:
        value = gamevar.announce[ANN_STAFF_POACHED][0];
        if (value) {
            if (!myplayer.advisor_flags[9]) {
                sprintf(advisor.text, advisor_situations[9]);
                myplayer.advisor_flags[9] = 1;
            } else {
                sprintf(advisor.text, advisor_text[0x59]);
            }
            done_announcement(ANN_STAFF_POACHED);
            found = 1;
        }
        break;
    case 0x15:
        if (advisor.heli_landed) {
            if (!myplayer.advisor_flags[19]) {
                sprintf(advisor.text, advisor_situations[16]);
                myplayer.advisor_flags[19] = 1;
            } else {
                sprintf(advisor.text, advisor_text[0x15]);
            }
            advisor.heli_landed = 0;
            found = 1;
        }
        break;
    case 0x7a:
        value = gamevar.announce[ANN_AUTOPSY_DISCOVERED][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x7a]);
            done_announcement(ANN_AUTOPSY_DISCOVERED);
            found = 1;
        }
        break;
    case 0x44:
        value = gamevar.announce[ANN_PROMOTED][0];
        if (value) {
            if (value == 5)
                sprintf(advisor.text, advisor_text[0x44], help_text[0x3f]);
            else if (value == 6)
                sprintf(advisor.text, advisor_text[0x44], help_text[0x40]);
            else if (value == 8)
                sprintf(advisor.text, advisor_text[0x44], help_text[0x41]);
            done_announcement(ANN_PROMOTED);
            found = 1;
        }
        break;
    case 0x89:
        value = gamevar.announce[ANN_DOCTOR_TO_CONSULTANT][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x89]);
            done_announcement(ANN_DOCTOR_TO_CONSULTANT);
            found = 1;
        }
        break;
    case 0x16:
        value = gamevar.announce[ANN_JUNIOR_TO_DOCTOR][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x16]);
            done_announcement(ANN_JUNIOR_TO_DOCTOR);
            found = 1;
        }
        break;
    case 0x17:
        value = gamevar.announce[ANN_OBJECT_RESEARCHED][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x17], object_names[value]);
            done_announcement(ANN_OBJECT_RESEARCHED);
            found = 1;
        }
        break;
    case 0x18:
        value = gamevar.announce[ANN_NEW_DRUG][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x18], illness_text[value]);
            done_announcement(ANN_NEW_DRUG);
            found = 1;
        }
        break;
    case 0x19:
        value = gamevar.announce[ANN_DRUG_IMPROVED][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x19], illness_text[value]);
            done_announcement(ANN_DRUG_IMPROVED);
            found = 1;
        }
        break;
    case 0x1a:
        value = gamevar.announce[ANN_OBJECT_IMPROVED][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x1a], object_names[value]);
            done_announcement(ANN_OBJECT_IMPROVED);
            found = 1;
        }
        break;
    case 0x1b:
        value = gamevar.announce[ANN_OBJECT_AVAILABLE][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x1b], object_names[value]);
            done_announcement(ANN_OBJECT_AVAILABLE);
            found = 1;
        }
        break;
    case 0x79:
        value = gamevar.announce[ANN_CATEGORY_DONE][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x79], research_names[value]);
            done_announcement(ANN_CATEGORY_DONE);
            found = 1;
        }
        break;
    case 0x45:
        if (advice[id].times_given < 1 && myhosp->litter_count > 0 && myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, advisor_text[0x45]);
            found = 1;
        }
        break;
    case 0x46:
        if (myhosp->area > 0 && advisor.has_patients) {
            if (advisor_litter_problem())
                found = 1;
        }
        break;
    case 0x47:
        if (advice[id].times_given < 1 && myhosp->deaths == 1) {
            sprintf(advisor.text, advisor_text[0x47]);
            found = 1;
        }
        break;
    case 0x52:
        if (advice[id].times_given < 1 && myhosp->cures == 1) {
            sprintf(advisor.text, advisor_text[0x52]);
            found = 1;
        }
        break;
    case 0x5c:
        if (NRAdcounts.plants > 0 && myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, advisor_text[0x5c]);
            found = 1;
        }
        break;
    case 0x5a:
        if (NRAdcounts.plants > 2 && advisor.has_patients) {
            if (advisor_dead_plants())
                found = 1;
        }
        break;
    case 0x5b:
        if (myhosp->area && advisor.has_patients) {
            value = (NRAdcounts.plants << 14) / myhosp->area;
            if (value > 0x800) {
                sprintf(advisor.text, advisor_text[0x5b]);
                found = 1;
            }
        }
        break;
    case 0x4a:
        if (adcounts.patients > 8 && myhosp->area > 0 && advisor.has_patients) {
            if (advisor_plants())
                found = 1;
        }
        break;
    case 0x4b:
        if (adcounts.patients > 5 && advisor.has_patients) {
            if (advisor_benches())
                found = 1;
        }
        break;
    case 0x49:
        if (myhosp->staff_count > myhosp->level + 5 && !listhead[player].rooms[ROOM_STAFF_ROOM]) {
            sprintf(advisor.text, advisor_text[0x49]);
            found = 1;
        }
        break;
    case 0x50:
        if (listhead[player].rooms[ROOM_TRAINING] != 0 && adcounts.consultants == 0 && othervar.last_built_room_s == 0x16 && advisor.has_patients) {
            sprintf(advisor.text, advisor_text[0x50]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x53:
        if (listhead[player].rooms[ROOM_RESEARCH] != 0 && adcounts.researchers == 0 && othervar.last_built_room_s == 0x1c && advisor.has_patients) {
            if (!myplayer.advisor_flags[1]) {
                sprintf(advisor.text, advisor_situations[2]);
                myplayer.advisor_flags[1] = 1;
            } else {
                sprintf(advisor.text, advisor_text[0x53]);
            }
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x66:
        what_is_required(0x13, player);
        if ((required.raw[0] & 0x20) && required.missing_room != 10 && required.surgeons_needed == 2 && othervar.last_built_room_s == 10
            && required.needs_ward) {
            if (!myplayer.advisor_flags[2]) {
                sprintf(advisor.text, advisor_situations[3]);
                myplayer.advisor_flags[2] = 1;
            } else {
                sprintf(advisor.text, advisor_text[0x66]);
            }
            found = 1;
            othervar.last_built_room_s = 1;
        }
        break;
    case 0x54:
        what_is_required(0x13, player);
        if ((required.raw[0] & 0x20) && required.missing_room != 10 && required.surgeons_needed == 2 && othervar.last_built_room_s == 10) {
            if (!myplayer.advisor_flags[2]) {
                sprintf(advisor.text, advisor_situations[3]);
                myplayer.advisor_flags[2] = 1;
            } else {
                sprintf(advisor.text, advisor_text[0x54]);
            }
            found = 1;
            othervar.last_built_room_s = 1;
        }
        break;
    case 0x67:
        what_is_required(0x13, player);
        if ((required.raw[0] & 0x20) && required.missing_room != 10 && required.surgeons_needed == 1 && othervar.last_built_room_s == 10
            && required.needs_ward) {
            sprintf(advisor.text, advisor_text[0x67]);
            found = 1;
        }
        break;
    case 0x55:
        what_is_required(0x13, player);
        if ((required.raw[0] & 0x20) && required.missing_room != 10 && required.surgeons_needed == 1 && othervar.last_built_room_s == 10) {
            sprintf(advisor.text, advisor_text[0x55]);
            found = 1;
        }
        break;
    case 0x56:
        what_is_required(0x13, player);
        if (!required.flags && required.missing_room != 10 && required.needs_ward && othervar.last_built_room_s == 10) {
            sprintf(advisor.text, advisor_text[0x56]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x57:
        what_is_required(0x13, player);
        if (!required.needs_ward && required.needs_nurse && othervar.last_built_room_s == 9) {
            sprintf(advisor.text, advisor_text[0x57]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x4f:
        what_is_required(5, player);
        if ((required.raw[0] & 4) && required.missing_room != 0xb && othervar.last_built_room_s == 0xb) {
            sprintf(advisor.text, advisor_text[0x4f]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x4e:
        what_is_required(4, player);
        if ((required.raw[0] & 0x40) && required.missing_room != 8 && othervar.last_built_room_s == 8) {
            if (!myplayer.advisor_flags[3]) {
                sprintf(advisor.text, advisor_situations[4]);
                myplayer.advisor_flags[3] = 1;
            } else {
                sprintf(advisor.text, advisor_text[0x4e]);
            }
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x5d:
        if (adcounts.machines > 0 && myhosp->staff_by_kind[2] == 0 && othervar.last_built_room_s > 0 && advisor.has_patients) {
            sprintf(advisor.text, advisor_text[0x5d]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x4c:
        if (myhosp->deaths > 4 && (advisor.event_flags_u32 & 0x80) && saferand() % 4 == 0) {
            if (saferand() % 2 == 0)
                sprintf(advisor.text, advisor_text[0x4c], myhosp->deaths);
            else
                sprintf(advisor.text, extra_advice[0x25], myhosp->deaths);
            advisor.event_flags_u32 &= ~0x80;
            found = 1;
        }
        break;
    case 0x51:
        if (myhosp->cures > 4 && (advisor.event_flags_u32 & 0x40) && saferand() % 4 == 0) {
            if (saferand() % 2 == 0)
                sprintf(advisor.text, advisor_text[0x51], myhosp->cures);
            else
                sprintf(advisor.text, extra_advice[0x24], myhosp->cures);
            found = 1;
            advisor.event_flags_u32 &= ~0x40;
        }
        break;
    case 0x68:
        if (listhead[player].rooms[ROOM_GP] != 0 && myhosp->staff_by_kind[1] == 0 && othervar.last_built_room_s == 7 && myhosp->level == 1) {
            sprintf(advisor.text, advisor_text[0x68]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x69:
        if (listhead[player].rooms[ROOM_RECEPTION] != 0 && myhosp->staff_by_kind[3] == 0 && myhosp->level == 1) {
            sprintf(advisor.text, advisor_text[0x69]);
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    case 0x6b:
        if ((myhosp->level >= 2 || myhosp->level <= 5) && NRAdcounts.open_rooms > 2 && NRAdcounts.open_rooms < 7) {
            sprintf(advisor.text, advisor_text[0x6b + saferand() % 3]);
            found = 1;
        }
        break;
    case 0x6e:
        if (!myplayer.advisor_flags[11] && gamevar.abductee) {
            sprintf(advisor.text, advisor_text[0x6e]);
            myplayer.advisor_flags[11] = 1;
            found = 1;
        }
        break;
    case 0x1c:
        if (!myplayer.advisor_flags[10] && (advisor.event_flags_u32 & 1)) {
            sprintf(advisor.text, advisor_text[0x1c]);
            myplayer.advisor_flags[10] = 1;
            advisor.event_flags_u32 &= ~1;
            found = 1;
        }
        break;
    case 0x72:
        if (myhosp->level > 1 && mycontrol->month > 6 && advisor.has_patients) {
            win_crit = 100 - adcounts.lose_progress;
            if (win_crit < 0x32 && win_crit > 0x19) {
                sprintf(advisor.text, advisor_text[0x74]);
                found = 1;
            } else if (win_crit < 0x19 && win_crit > 5) {
                sprintf(advisor.text, advisor_text[0x73]);
                found = 1;
            } else if (win_crit < 5 && win_crit > 0) {
                sprintf(advisor.text, advisor_text[0x72]);
                found = 1;
            }
        }
        break;
    case 0x6f:
        if (myhosp->level > 1 && mycontrol->month > myhosp->level + 3 && advisor.has_patients) {
            win_crit = 100 - adcounts.win_progress;
            if (win_crit < 0x32 && win_crit > 0x19) {
                sprintf(advisor.text, advisor_text[0x71]);
                found = 1;
            } else if (win_crit < 0x19 && win_crit > 5) {
                sprintf(advisor.text, advisor_text[0x70]);
                found = 1;
            } else if (win_crit < 5 && win_crit > 0) {
                sprintf(advisor.text, advisor_text[0x6f]);
                found = 1;
            }
        }
        break;
    case 0x75:
        if ((advisor.event_flags_u32 & 2) && advisor.unpaid_treatment != 0) {
            sprintf(advisor.text, advisor_text[0x75], illness_text[advisor.unpaid_treatment]);
            found = 1;
            advisor.event_flags_u32 &= ~2;
        }
        break;
    case 0x76:
        if (myhosp->price_level > 0x96) {
            sprintf(advisor.text, advisor_text[0x76]);
            found = 1;
        }
        break;
    case 0x77:
        if (myhosp->price_level && myhosp->price_level < 0x3c) {
            sprintf(advisor.text, advisor_text[0x77]);
            found = 1;
        }
        break;
    case 0x7b:
        if (advisor.new_staff_pending) {
            if (advisor.new_staff_kind == 1) {
                if (advisor.new_staff_room == 8) {
                    sprintf(advisor.text, advisor_text[0x7d]);
                    found = 1;
                } else if (advisor.new_staff_room == 0x1c) {
                    sprintf(advisor.text, advisor_text[0x7b]);
                    found = 1;
                } else if (advisor.new_staff_room == 10) {
                    sprintf(advisor.text, advisor_text[0x7c]);
                    found = 1;
                } else if (advisor.new_staff_room != 0x1d) {
                    sprintf(advisor.text, advisor_text[0x7e + saferand() % 2], real_room_names[rooms[advisor.new_staff_room].id]);
                    found = 1;
                }
            } else if (advisor.new_staff_kind == 0) {
                if (advisor.new_staff_room != 0x1d) {
                    sprintf(advisor.text, advisor_text[0x80 + saferand() % 2], real_room_names[rooms[advisor.new_staff_room].id]);
                    found = 1;
                }
            } else if (advisor.new_staff_kind == 3) {
                sprintf(advisor.text, advisor_text[0x82]);
                found = 1;
            }
            advisor.new_staff_pending = 0;
            advisor.new_staff_kind = 0;
            advisor.new_staff_room = 0;
        }
        break;
    case 0x86:
        value = gamevar.announce[ANN_RIVAL_OPENED][0];
        if (value) {
            sprintf(advisor.text, advisor_text[0x86], allhosps[value].name);
            done_announcement(ANN_RIVAL_OPENED);
            found = 1;
        }
        break;
    case 0x87:
        if (!myplayer.advisor_flags[8] && (advisor.event_flags_u32 & 4)) {
            sprintf(advisor.text, advisor_text[0x87]);
            myplayer.advisor_flags[8] = 1;
            advisor.event_flags_u32 &= ~4;
            found = 1;
        }
        crit = myplayer.advisor_flags[8];
        win_crit = 8;
        break;
    case 0x8a:
        if (!myplayer.advisor_flags[6] && (advisor.event_flags_u32 & 0x100)) {
            sprintf(advisor.text, advisor_situations[7]);
            myplayer.advisor_flags[6] = 1;
            advisor.event_flags_u32 &= ~0x100;
            found = 1;
        }
    case 0x8b:
        if (!myplayer.advisor_flags[4] && myhosp->rathole_count > 0) {
            sprintf(advisor.text, advisor_situations[5]);
            myplayer.advisor_flags[4] = 1;
            found = 1;
        }
        break;
    case 0x8c:
        if (!myplayer.advisor_flags[0] && othervar.last_built_room_s == 0x1c) {
            sprintf(advisor.text, advisor_situations[1]);
            myplayer.advisor_flags[0] = 1;
            found = 1;
            othervar.last_built_room_s = 0;
        }
        break;
    default:
        break;
    }
    if (found == 1 && advice[id].id && (s8)advisor.text[0] != 0) {
        advisor.mouse_advice_id = 0;
        advisor.line_count = 0;
        advisor.extra_segments = 0;
        advice[id].times_given++;
        advice[id].time = actual_game_turn;
        advice[id].f7 = f7;
        advice[id].fc = extra;
        advisor.advice_id = id;
        advisor.time_left = advice[id].duration / 2;
    }
    return found;
}

// Writes the text for the mouse advice advisor.mouse_advice_id into advisor.text.
// FUNCTION: 0x000608bc
void mouse_advice(void)
{
    s32 unused;

    switch (advisor.mouse_advice_id) {
    default:
        fn_000bbff0(advisor.text, mouse_advice_text[advisor.mouse_advice_id]);
        break;
    }
}

// Writes the text for the forced advice advisor.forced_advice_id (some with names filled in)
// into advisor.text, and clears the request.
// FUNCTION: 0x00060904
void forced_advice(void)
{
    u8 i;

    switch (advisor.forced_advice_id_s) {
    case 2:
        fn_000bbff0(advisor.text, forced_advice_text[advisor.forced_advice_id_s]);
        for (i = 0; i < 4; i++) {
            if (othervar.net_winners[i]) {
                sprintf(advisor.text, "%s %s", advisor.text, allhosps[i].name);
                othervar.net_winners[i] = 0;
            }
        }
        break;
    case 8:
        if (myplayer.advisor_flags[7] == 0) {
            sprintf(advisor.text, advisor_situations[8]);
            myplayer.advisor_flags[7] = 1;
        } else {
            sprintf(advisor.text, forced_advice_text[advisor.forced_advice_id_s], mayor_names[allhosps[player].mayor_name]);
        }
        break;
    default:
        fn_000bbff0(advisor.text, forced_advice_text[advisor.forced_advice_id_s]);
        break;
    }
    advisor.advice_id = 0;
    advisor.forced_advice_id_s = 0;
}

// Clears all advice history and the current advice, and starts a new
// statistics pass.
// FUNCTION: 0x00060a80
void reset_advisor(void)
{
    u16 i;

    for (i = 1; advice[i].id; i++) {
        advice[i].times_given = 0;
        advice[i].f7 = 0;
        advice[i].time = 0;
        advice[i].fc = 0;
    }
    advisor.advice_id = 0;
    advisor.time_left = 0;
    advisor.unpaid_treatment = 0;
    advisor.forced_advice_id_s = 0;
    advisor.mouse_advice_id = 0;
    advisor.text[0] = 0;
    othervar.every_32_turns = 1;
    advisor.has_patients = 0;
    advisor.event_flags_u32 ^= advisor.event_flags_u32;
    reset_all_count();
}

// Queues announcement value of type type for the advisor (up to 4 per type,
// duplicates ignored).
// FUNCTION: 0x00060b54
void announce_this(const u8 type, const u32 value)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if (gamevar.announce[type][i] == 0) {
            gamevar.announce[type][i] = value;
            break;
        }
        if (gamevar.announce[type][i] == value)
            break;
    }
}

// Removes the oldest announcement of type type and asks for the bubble to be
// re-wrapped.
// FUNCTION: 0x00060bf4
void done_announcement(const u16 type)
{
    u32 i;

    for (i = 0; i < 3; i++)
        gamevar.announce[type][i] = gamevar.announce[type][i + 1];
    gamevar.announce[type][3] = 0;
    advisor.rewrap = 1;
}

// Toilet advice from the patients' average bladder (advisor counts). Returns
// 1 if advice was written.
// FUNCTION: 0x00060c78
u8 advisor_toilets(void)
{
    s32 ratio;

    ratio = adcounts.bladder_sum / adcounts.patients;
    if (ratio * 4 > gbv.bowel_full * 3 && listhead[player].objects[OBJ_TOILET] == 0) {
        sprintf(advisor.text, advisor_text[0x13]);
        return 1;
    }
    if (gbv.bowel_full < ratio && listhead[player].objects[OBJ_TOILET] == 0) {
        sprintf(advisor.text, extra_advice[10]);
        return 1;
    }
    if (gbv.bowel_full < ratio && listhead[player].objects[OBJ_TOILET] != 0) {
        sprintf(advisor.text, extra_advice[11]);
        return 1;
    }
    if (gbv.bowel_overflows < ratio && listhead[player].objects[OBJ_TOILET] == 0) {
        sprintf(advisor.text, extra_advice[0x26]);
        return 1;
    }
    if (gbv.bowel_overflows < ratio) {
        sprintf(advisor.text, extra_advice[12]);
        return 1;
    }
    return 0;
}

// Plant advice from the plants per land area and the patients' impatience and
// happiness. Returns 1 if advice was written.
// FUNCTION: 0x00060e18
u8 advisor_plants(void)
{
    s32 per_area;
    s32 impatience;
    s32 happiness;

    per_area = (NRAdcounts.plants << 14) / myhosp->area;
    impatience = (adcounts.impatience_sum << 6) / (adcounts.patients * gbv.leave_max);
    happiness = (adcounts.happiness_sum << 10) / adcounts.patients;
    if (impatience > 0x30) {
        if (per_area < 0x100) {
            sprintf(advisor.text, extra_advice[0x14]);
            return 1;
        } else if (per_area < 0x200) {
            sprintf(advisor.text, extra_advice[0x15]);
            return 1;
        }
    } else if ((gbv.very_unhappy << 10) > happiness) {
        if (per_area < 0x100) {
            sprintf(advisor.text, extra_advice[0x16]);
            return 1;
        } else if (per_area < 0x200) {
            sprintf(advisor.text, extra_advice[0x17]);
            return 1;
        }
    } else if (per_area >= 0x400 && per_area <= 0x800) {
        sprintf(advisor.text, advisor_text[0x4a]);
        return 1;
    }
    return 0;
}

// Machine advice from their average wear and whether there are handymen.
// Returns 1 if advice was written.
// FUNCTION: 0x00060f90
u8 advisor_old_hardware(void)
{
    if (adcounts.machine_wear_sum * 20 * adcounts.machines > adcounts.machines * 0x1300) {
        if (myhosp->staff_by_kind[2] > 0) {
            sprintf(advisor.text, advisor_text[3]);
            return 1;
        } else if (myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, extra_advice[0x21]);
            return 1;
        }
    } else if (adcounts.machine_wear_sum * 5 * adcounts.machines > adcounts.machines * 0x400) {
        sprintf(advisor.text, extra_advice[0x1f]);
        return 1;
    } else if (adcounts.machine_wear_sum * 5 * adcounts.machines > adcounts.machines * 0x300) {
        if (myhosp->staff_by_kind[2] > 0) {
            sprintf(advisor.text, extra_advice[0x20]);
            return 1;
        } else if (myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, extra_advice[0x22]);
            return 1;
        }
    } else if (adcounts.machine_wear_sum * 2 * adcounts.machines > adcounts.machines * 0x100) {
        if (!myplayer.advisor_flags[12]) {
            sprintf(advisor.text, advisor_situations[10]);
            myplayer.advisor_flags[12] = 1;
            return 1;
        } else if (myhosp->staff_by_kind[2] > 0) {
            sprintf(advisor.text, extra_advice[0x23]);
            return 1;
        }
    }
    return 0;
}

// Drinks advice from the patients' average refreshment. Returns 1 if advice
// was written. The "very thirsty" branch (below 10) is never reached.
// FUNCTION: 0x00061188
u8 advisor_thirsty_peeps(void)
{
    s32 thirst;

    thirst = adcounts.refreshment_sum / adcounts.patients;
    if (thirst < 0x1e) {
        if (!myplayer.advisor_flags[18]) {
            sprintf(advisor.text, advisor_situations[15]);
            myplayer.advisor_flags[18] = 1;
            return 1;
        } else if (listhead[player].objects[OBJ_DRINKS_MACHINE] == 0) {
            sprintf(advisor.text, advisor_text[15]);
            return 1;
        } else {
            sprintf(advisor.text, extra_advice[4]);
            return 1;
        }
    } else if (thirst < 10) {
        if (listhead[player].objects[OBJ_DRINKS_MACHINE] == 0) {
            sprintf(advisor.text, advisor_text[5]);
            return 1;
        } else {
            sprintf(advisor.text, extra_advice[0x33]);
            return 1;
        }
    }
    return 0;
}

// Advice about unhappy patients: those who left fed up (othervar.patients_left_count, then
// cleared) or a low average happiness. Returns 1 if advice was written.
// FUNCTION: 0x000612b8
u8 advisor_peeps_unhappy(void)
{
    s32 happiness;

    happiness = (adcounts.happiness_sum << 10) / adcounts.patients;
    if (othervar.patients_left_count) {
        if (othervar.patients_left_count > 1)
            sprintf(advisor.text, extra_advice[7]);
        else
            sprintf(advisor.text, extra_advice[0x31]);
        othervar.patients_left_count = 0;
        return 1;
    } else if ((gbv.very_unhappy << 10) > happiness) {
        sprintf(advisor.text, extra_advice[6]);
        return 1;
    } else if ((gbv.unhappy << 10) > happiness) {
        sprintf(advisor.text, advisor_text[14]);
        return 1;
    }
    return 0;
}

// Heating advice from the patients' average warmth, with a too hot or too
// cold sound. Returns 1 if advice was written.
// FUNCTION: 0x000613a0
u8 advisor_peeps_warmth(void)
{
    s32 warmth;

    warmth = adcounts.warmth_sum / adcounts.patients;
    if (warmth * 2 > 0xf0) {
        sprintf(advisor.text, extra_advice[0x34]);
        play_ambient(0xd9);
        return 1;
    } else if (warmth > 0x50) {
        if (!myplayer.advisor_flags[17]) {
            sprintf(advisor.text, advisor_situations[14]);
            myplayer.advisor_flags[17] = 1;
        } else {
            sprintf(advisor.text, advisor_text[0x11]);
        }
        play_ambient(0xd9);
        return 1;
    } else if (warmth < 10) {
        sprintf(advisor.text, extra_advice[0x35]);
        play_ambient(0xd8);
        return 1;
    } else if (warmth < 0x14) {
        if (!myplayer.advisor_flags[15] && myhosp->radiator_count == 0) {
            sprintf(advisor.text, advisor_situations[12]);
            myplayer.advisor_flags[15] = 1;
        } else if (myplayer.advisor_flags[15] && !myplayer.advisor_flags[16] && myhosp->radiator_count == 0) {
            sprintf(advisor.text, advisor_situations[13]);
            myplayer.advisor_flags[16] = 1;
        } else {
            sprintf(advisor.text, extra_advice[13]);
        }
        play_ambient(0xd8);
        return 1;
    }
    return 0;
}

// Litter advice from the litter per land area and the number of handymen.
// Returns 1 if advice was written.
// FUNCTION: 0x00061558
u8 advisor_litter_problem(void)
{
    s32 litter;

    litter = (myhosp->litter_count << 14) / myhosp->area;
    if (litter > 0x600) {
        play_ambient(0xd7);
        if (myhosp->staff_by_kind[2] > 0) {
            sprintf(advisor.text, extra_advice[0x12]);
            return 1;
        } else if (myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, extra_advice[0x11]);
            return 1;
        }
    } else if (litter > 0x300) {
        sprintf(advisor.text, extra_advice[0x10]);
        return 1;
    } else if (litter > 0x100) {
        if (myhosp->staff_by_kind[2] > 1) {
            sprintf(advisor.text, advisor_text[0x46]);
            return 1;
        } else if (myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, extra_advice[0x13]);
            return 1;
        }
    }
    return 0;
}

// Bench advice from the bench shortage score per patient (NRAdcounts.bench_shortage:
// up when a patient finds no free bench, down when one does) and the number of
// benches (unknown4). Returns 1 if advice was written.
// FUNCTION: 0x000616b4
u8 advisor_benches(void)
{
    s32 standing;

    standing = (NRAdcounts.bench_shortage << 10) / adcounts.patients;
    if (standing > 0x1000) {
        sprintf(advisor.text, extra_advice[0x1b]);
        return 1;
    } else if (standing > 0x400) {
        sprintf(advisor.text, extra_advice[0x18]);
        return 1;
    } else if (NRAdcounts.benches > myhosp->level + 8 && standing < -0x1400) {
        sprintf(advisor.text, extra_advice[0x1a]);
        return 1;
    } else if (NRAdcounts.benches > 8 && standing < -0x400) {
        sprintf(advisor.text, extra_advice[0x19]);
        return 1;
    } else if (NRAdcounts.benches > 4) {
        sprintf(advisor.text, advisor_text[0x4b]);
        return 1;
    }
    return 0;
}

// Advice from the plants' average health. Returns 1 if advice was written.
// FUNCTION: 0x000617e4
u8 advisor_dead_plants(void)
{
    if (NRAdcounts.plant_health_sum * 0x190000 / NRAdcounts.plants < 0xce4000) {
        if (myhosp->staff_by_kind[2] == 0) {
            sprintf(advisor.text, extra_advice[0x27]);
            return 1;
        } else if (myhosp->staff_by_kind[2] > 1) {
            sprintf(advisor.text, extra_advice[0x28]);
            return 1;
        }
    } else if (NRAdcounts.plant_health_sum * 0x190000 / NRAdcounts.plants < 0x2260000) {
        sprintf(advisor.text, extra_advice[0x29]);
        return 1;
    } else if (NRAdcounts.plant_health_sum * 0x190000 / NRAdcounts.plants < 0x3c28000) {
        sprintf(advisor.text, advisor_text[0x5a]);
        return 1;
    } else if (NRAdcounts.plant_health_sum * 0x190000 / NRAdcounts.plants > 0x51a4000) {
        sprintf(advisor.text, extra_advice[0x2b]);
        return 1;
    } else if ((NRAdcounts.plant_health_sum << 14) / NRAdcounts.plants > 0x490c000) {
        sprintf(advisor.text, extra_advice[0x2a]);
        return 1;
    }
    return 0;
}

// Lets the unhappy patients and thirst advice (entries 14 and 15) be given
// again this month.
// FUNCTION: 0x00061998
void monthly_update_of_adivsor_counts(void)
{
    advice[14].times_given = 0;
    advice[15].times_given = 0;
}

// Returns a staff_unhappy entry for the first kind of staff more tired than
// gbv.very_tired on average (graded by the overall tiredness), or 0. Writes no text;
// find_advice ignores the result.
// FUNCTION: 0x000619bc
u8 advisor_staff_tired(void)
{
    s32 average;
    s32 tired;
    u8 i;
    u8 severity;

    average = (adcounts.staff_tiredness_sum << 8) / myhosp->staff_count;
    i = 0;
    severity = 0;
    if ((gbv.very_tired << 8) < average)
        severity = 0;
    else if ((gbv.tired << 8) < average)
        severity = 0x10;
    else if ((gbv.not_tired << 8) > average)
        severity = 0x20;
    else
        severity = 0x30;
    for (; i < 4; i++) {
        if (myhosp->staff_by_kind[i]) {
            tired = (adcounts.staff_tiredness_by_kind[i] << 8) / myhosp->staff_by_kind[i];
            if ((gbv.very_tired << 8) < tired) {
                if (((average - tired) << 8) / (myhosp->staff_count - myhosp->staff_by_kind[i])
                    < (gbv.very_tired << 8))
                    ++severity;
                if (staff_unhappy[severity])
                    return staff_unhappy[severity];
            }
        }
    }
    return 0;
}

// Heating advice from the staff's average warmth, with a sound. Returns 1 if
// advice was written.
// FUNCTION: 0x00061b34
u8 advisor_staff_warmth(void)
{
    s32 warmth;

    warmth = adcounts.staff_warmth_sum / myhosp->staff_count;
    if (warmth > 0x50) {
        sprintf(advisor.text, advisor_text[0x12]);
        play_ambient(0xd9);
        return 1;
    } else if (warmth < 0x14) {
        sprintf(advisor.text, extra_advice[0x36]);
        play_ambient(0xd8);
        return 1;
    }
    return 0;
}
