// The tutorial level's scripted advice (src/tutorial.cpp): the advisor's
// steps through building the first rooms and hiring the first staff.

#include "core/world/advisor_internal.h"

#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

namespace th
{

namespace
{

// The level briefing's page that ends the tutorial (show_help_page(1, 0xf)).
constexpr int kTutorialEndPage = 0xf;

// What tutorial's steps work on: the advisor, the turn's inputs and the
// events the steps test.
struct Tutorial {
    Advisor &a;
    const AdvisorInputs &in;
    const World &world;
    std::array<uint8_t, 0x26> &shown;
    const Event &last;
    const Event &pending;
    // The player cancelled or hired someone other than a receptionist.
    bool hiring_other;

    void message(int i)
    {
        a.text = in.text->at(TextSection::Advisor, i);
        a.rewrap = true;
        a.line_count = 0;
        a.extra_segments = 0;
    }

    // A hint given as help text: no message, him hidden.
    void hint(uint16_t highlight, int flag)
    {
        a.line_count = 0;
        a.extra_segments = 0;
        a.tutorial_highlight = highlight;
        shown[size_t(flag)] = 1;
        a.text.clear();
    }

    void go(uint8_t step)
    {
        a.tutorial_step = step;
        a.tutorial_highlight = 0;
    }

    // Messages to give again once the player is back at the step they belong to.
    void forget(std::initializer_list<int> flags)
    {
        for (int flag : flags)
            shown[size_t(flag)] = 0;
    }
};

// Each of tutorial's steps returns what tutorial returns.

// tutorial step 0x1d: open the corridor furnishing.
int tutorial_open_furnish_corridor(Tutorial &t)
{
    if (t.shown[0] == 2)
        return -1;
    if (t.last.type == EV_OPEN_FURNISH_CORRIDOR) {
        t.shown[0] = 2;
        t.go(0x28);
    } else if (t.shown[0] == 0) {
        t.message(29);
        t.a.tutorial_highlight = 0x13;
        t.shown[0] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x28: buy a reception desk.
int tutorial_buy_reception_desk(Tutorial &t)
{
    if (t.shown[11] == 2)
        return -1;
    if (t.pending.type == EV_CANCEL_BUY_OBJECTS || t.last.type == EV_PLACE_NEXT_OBJECT) {
        t.shown[0] = 0;
        t.shown[11] = 0;
        t.go(0x1d);
    } else if (t.last.type != EV_BUY_OBJECT || t.last.value != kObjectReceptionDesk) {
        if (t.shown[11] == 0) {
            t.hint(0xa, 11);
            return 0;
        }
    } else {
        t.shown[11] = 2;
        t.go(0x2a);
    }
    return -1;
}

// tutorial step 0x2a: confirm the purchase, to place the desk.
int tutorial_confirm_purchase(Tutorial &t)
{
    if (t.shown[13] == 2)
        return -1;
    if (t.pending.type == EV_CANCEL_BUY_OBJECTS) {
        t.shown[0] = 0;
        t.shown[11] = 0;
        t.shown[13] = 0;
        t.go(0x1d);
    } else if (t.last.type == EV_SELL_OBJECT && t.last.value == kObjectReceptionDesk &&
               t.world.placing.buy_count[kObjectReceptionDesk] == 0) {
        t.shown[13] = 0;
        t.shown[11] = 0;
        t.go(0x28);
    } else if (t.last.type != EV_PLACE_NEXT_OBJECT) {
        if (t.shown[13] == 0) {
            t.hint(9, 13);
            return 0;
        }
    } else {
        t.shown[13] = 2;
        t.go(0x29);
    }
    return -1;
}

// tutorial step 0x29: place the reception desk.
int tutorial_place_reception_desk(Tutorial &t)
{
    if (t.shown[12] == 2)
        return -1;
    if (t.pending.type == EV_CANCEL_BUILD) {
        t.shown[12] = 0;
        t.shown[13] = 0;
        t.shown[11] = 0;
        t.shown[0] = 0;
        t.go(0x1d);
    } else if (t.a.flags & kAdvisorBadPlacement) {
        t.message(46);
        t.shown[12] = 0;
        return 1;
    } else if (!has_reception(t.world)) {
        if (t.shown[12] == 0) {
            t.message(41);
            t.shown[12] = 1;
            return 1;
        }
    } else {
        t.shown[12] = 2;
        t.go(0x1e);
    }
    return -1;
}

// tutorial step 0x1e: open the staff hiring.
int tutorial_open_hire_staff(Tutorial &t)
{
    if (t.shown[1] == 2 || count_staff(t.world).receptionists != 0) {
        t.shown[1] = 2;
        t.go(0x20);
        return -1;
    }
    if (t.last.type == EV_OPEN_HIRE_STAFF) {
        t.shown[1] = 2;
        t.go(0x21);
    } else if (t.shown[1] == 0) {
        t.message(30);
        t.a.tutorial_highlight = 0x15;
        t.shown[1] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x21: choose receptionists.
int tutorial_select_receptionists(Tutorial &t)
{
    if (t.shown[4] == 2) {
        t.go(0x25);
        return -1;
    }
    if (t.hiring_other) {
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        // The message stays as it was.
        t.a.line_count = 0;
        t.a.extra_segments = 0;
        t.a.tutorial_highlight = 3;
        return 1;
    } else if (t.last.type != EV_HIRE_SELECT_KIND || t.last.value != kStaffReceptionist) {
        if (t.shown[4] == 0) {
            t.hint(3, 4);
            return 0;
        }
    } else {
        t.shown[4] = 2;
        t.go(0x25);
    }
    return -1;
}

// tutorial step 0x25: look at the next applicant.
int tutorial_next_applicant(Tutorial &t)
{
    if (t.shown[8] == 2) {
        t.go(0x26);
        return -1;
    }
    if (t.hiring_other) {
        t.shown[8] = 2;
        t.shown[9] = 2;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_MARK_EMPLOYING && t.last.value == kStaffReceptionist) {
        t.shown[8] = 2;
        t.shown[9] = 2;
        t.go(0x24);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        t.shown[8] = 2;
        t.shown[9] = 2;
        t.shown[4] = 0;
        t.go(0x21);
    } else if (t.last.type != EV_HIRE_NEXT_APPLICANT || t.last.value != 1) {
        if (t.shown[8] == 0) {
            t.hint(7, 8);
            return 0;
        }
    } else {
        t.shown[8] = 2;
        t.go(0x26);
    }
    return -1;
}

// tutorial step 0x26: look at the previous applicant.
int tutorial_previous_applicant(Tutorial &t)
{
    if (t.shown[9] == 2) {
        t.go(0x22);
        return -1;
    }
    if (t.hiring_other) {
        t.shown[9] = 2;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_MARK_EMPLOYING && t.last.value == kStaffReceptionist) {
        t.shown[9] = 2;
        t.go(0x24);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        t.shown[9] = 2;
        t.shown[4] = 0;
        t.go(0x21);
    } else if (t.last.type != EV_HIRE_NEXT_APPLICANT || t.last.value != 0) {
        if (t.shown[9] == 0) {
            t.hint(6, 9);
            return 0;
        }
    } else {
        t.shown[9] = 2;
        t.go(0x22);
    }
    return -1;
}

// tutorial step 0x22: hire the applicant.
int tutorial_hire_applicant(Tutorial &t)
{
    if (t.shown[5] == 2) {
        t.go(0x24);
        return -1;
    }
    if (t.last.type == EV_CANCEL_BUILD) {
        t.shown[5] = 0;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        t.shown[4] = 0;
        t.go(0x21);
    } else if (t.last.type != EV_MARK_EMPLOYING) {
        if (t.shown[5] == 0) {
            t.hint(5, 5);
            return 0;
        }
    } else {
        t.shown[5] = 2;
        t.go(0x24);
    }
    return -1;
}

// tutorial step 0x24: place the receptionist.
int tutorial_place_receptionist(Tutorial &t)
{
    if (t.shown[7] == 2) {
        t.go(0x20);
        return -1;
    }
    if (t.last.type == EV_CANCEL_BUILD) {
        t.shown[7] = 2;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (!t.in.staff_place_ok) {
        t.message(60);
        t.shown[7] = 0;
        return 1;
    } else if (count_staff(t.world).receptionists == 0) {
        if (t.shown[7] == 0) {
            t.message(36);
            t.shown[7] = 1;
            t.a.tutorial_highlight = 0;
            return 1;
        }
    } else {
        t.shown[7] = 2;
        t.go(0x20);
    }
    return -1;
}

// tutorial step 0x20: open the room building, once no GP's office exists.
int tutorial_open_build_room(Tutorial &t)
{
    if (t.shown[3] == 2 || t.world.room_of_type[kRoomGp]) {
        t.shown[3] = 2;
        t.go(0x1f);
    } else if (t.last.type == EV_OPEN_BUILD_ROOM) {
        t.shown[3] = 2;
        t.go(0x2b);
    } else if (t.shown[3] == 0) {
        t.message(32);
        t.a.tutorial_highlight = 0x12;
        t.shown[3] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x2b: choose the department of the GP's office.
int tutorial_choose_department(Tutorial &t)
{
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14});
        t.go(0x20);
    } else if (t.shown[14] != 2) {
        if (t.last.type != EV_REQ1_RESET_ROOMS || t.last.value != 3) {
            if (t.shown[14] == 0) {
                t.hint(8, 14);
                return 0;
            }
        } else {
            t.shown[14] = 2;
            t.go(0x2c);
        }
    }
    return -1;
}

// tutorial step 0x2c: choose the GP's office. The hint marks flag 14 shown,
// not 15, as the game does, so it is given every turn.
int tutorial_choose_gp_office(Tutorial &t)
{
    if (t.last.type == EV_CLOSE_REQUESTERS) {
        t.forget({3, 14, 15});
        t.go(0x20);
    } else if (t.shown[15] != 2) {
        if (t.last.type == EV_INIT_BLUEPRINT && t.last.value != kRoomGp) {
            t.forget({3, 14, 15});
            t.go(0x20);
        } else if (t.last.type != EV_INIT_BLUEPRINT) {
            if (t.shown[15] == 0) {
                t.hint(0xb, 14);
                return 0;
            }
        } else {
            t.shown[15] = 2;
            t.go(0x39);
        }
    }
    return -1;
}

// tutorial step 0x39: drag the blueprint out.
int tutorial_drag_blueprint(Tutorial &t)
{
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14, 15, 28});
        t.go(0x20);
    } else if (t.last.type != EV_MODIFY_BLUEPRINT) {
        t.message(47);
        t.shown[28] = 0;
        return 1;
    } else {
        t.shown[28] = 2;
        t.a.tutorial_step = 0x2f;
        t.a.tutorial_highlight = 0;
    }
    return -1;
}

// tutorial step 0x2f: make the blueprint valid, in size and placement.
int tutorial_valid_blueprint(Tutorial &t)
{
    uint8_t &flags = t.a.flags;
    bool bad = flags & kAdvisorBadPlacement, small = flags & kAdvisorBlueprintSmall;
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14, 15, 28, 18});
        t.go(0x20);
    } else if (small && bad) {
        t.message(61);
        t.shown[27] = 0;
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (!small && !bad) {
        t.message(52);
        t.a.forced_advice_id = 0;
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (t.a.forced_advice_id == 4 || bad) {
        t.message(51);
        t.shown[18] = 0;
        flags &= uint8_t(~kAdvisorBadPlacement);
        t.a.tutorial_highlight = 0;
        return 1;
    } else {
        t.shown[18] = 2;
        t.go(0x38);
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
    }
    return -1;
}

// tutorial step 0x38: accept the blueprint.
int tutorial_accept_blueprint(Tutorial &t)
{
    uint8_t &flags = t.a.flags;
    bool bad = flags & kAdvisorBadPlacement, small = flags & kAdvisorBlueprintSmall;
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20, 21, 29});
        t.go(0x20);
    } else if (small && bad) {
        t.message(61);
        t.shown[27] = 0;
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (small) {
        t.message(52);
        t.shown[27] = 0;
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (t.a.forced_advice_id == 4 || bad) {
        t.message(51);
        t.shown[27] = 0;
        flags &= uint8_t(~kAdvisorBadPlacement);
        t.a.forced_advice_id = 0;
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (t.last.type != EV_COMMIT_BLUEPRINT) {
        if (t.shown[27] == 0) {
            t.message(56);
            t.a.tutorial_highlight = kEditRoomConfirm;
            t.shown[27] = 1;
            flags &= uint8_t(~kAdvisorBadPlacement);
            return 1;
        }
    } else {
        t.shown[27] = 2;
        t.go(0x30);
    }
    return -1;
}

// tutorial step 0x30: place the door.
int tutorial_place_door(Tutorial &t)
{
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19});
        t.go(0x38);
    } else if (t.last.type == EV_PLACE_BP_DOOR && t.world.building.door_invalid) {
        t.message(53);
        t.shown[24] = 1;
        return 1;
    } else if (t.last.type != EV_PLACE_BP_DOOR) {
        if (t.shown[19] == 0) {
            t.message(48);
            t.shown[19] = 1;
            return 1;
        }
    } else {
        t.shown[19] = 2;
        t.go(0x31);
    }
    return -1;
}

// tutorial step 0x31: place the windows.
int tutorial_place_windows(Tutorial &t)
{
    bool placed = t.last.type == EV_PLACE_BP_DOOR;
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20});
        t.go(0x38);
    } else if (placed && t.world.building.door_invalid) {
        t.message(54);
        t.shown[25] = 1;
        return 1;
    } else if (placed) {
        t.message(49);
        t.shown[20] = 1;
        return 1;
    } else if (t.last.type != EV_COMMIT_DOORS) {
        if (t.shown[20] == 0) {
            t.message(49);
            t.a.tutorial_highlight = kEditRoomConfirm;
            t.shown[20] = 1;
            return 1;
        }
    } else {
        t.shown[19] = 2;
        t.go(0x32);
        t.a.flags &= uint8_t(~kAdvisorBadPlacement);
    }
    return -1;
}

// tutorial step 0x32: place the objects the room needs.
int tutorial_furnish_room(Tutorial &t)
{
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20, 21});
        t.go(0x38);
    } else if (t.a.flags & kAdvisorBadPlacement) {
        t.message(55);
        t.a.flags &= uint8_t(~kAdvisorBadPlacement);
        t.shown[21] = 0;
        return 1;
    } else if (!(t.a.flags & kAdvisorRoomFurnished)) {
        if (t.shown[21] == 0) {
            t.message(50);
            t.shown[21] = 1;
            return 1;
        }
    } else {
        t.shown[29] = 0;
        t.go(0x3a);
    }
    return -1;
}

// tutorial step 0x3a: open the room.
int tutorial_open_room(Tutorial &t)
{
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20, 21, 29});
        t.go(0x38);
    } else if (t.last.type != EV_OPEN_ROOM) {
        if (t.shown[29] == 0) {
            t.message(58);
            t.a.tutorial_highlight = kEditRoomConfirm;
            t.shown[29] = 1;
            return 1;
        }
    } else {
        t.shown[29] = 2;
        t.go(0x1f);
    }
    return -1;
}

// tutorial step 0x1f: open the staff hiring, for a doctor. The tutorial ends
// once the hospital has one.
int tutorial_open_hire_doctor(Tutorial &t)
{
    if (t.shown[2] == 2 || count_staff(t.world).doctors != 0) {
        t.shown[2] = 2;
        t.a.tutorial_highlight = 0;
        t.a.flags &= uint8_t(~kAdvisorTutorial);
    } else if (t.last.type == EV_OPEN_HIRE_STAFF) {
        t.shown[2] = 2;
        t.go(0x23);
    } else if (t.shown[2] == 0) {
        t.message(31);
        t.a.tutorial_highlight = 0x15;
        t.shown[2] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x23: choose doctors.
int tutorial_select_doctors(Tutorial &t)
{
    bool other = t.last.type == EV_CANCEL_BUILD || (t.last.type == EV_MARK_EMPLOYING && t.last.value != kStaffDoctor);
    if (t.shown[6] == 2) {
        t.go(0x2d);
    } else if (other) {
        t.forget({2, 6});
        t.go(0x1f);
    } else if (t.last.type != EV_HIRE_SELECT_KIND || t.last.value != kStaffDoctor) {
        if (t.shown[6] == 0) {
            t.hint(4, 6);
            return 0;
        }
    } else {
        t.shown[6] = 2;
        t.go(0x2d);
    }
    return -1;
}

// tutorial step 0x2d: hire the doctor.
int tutorial_hire_doctor(Tutorial &t)
{
    if (t.shown[16] == 2) {
        t.go(0x27);
    } else if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({2, 6, 16});
        t.go(0x1f);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffDoctor) {
        t.shown[6] = 0;
        t.go(0x23);
    } else if (t.last.type != EV_MARK_EMPLOYING) {
        if (t.shown[16] == 0) {
            t.hint(5, 16);
            return 0;
        }
    } else {
        t.shown[16] = 2;
        t.go(0x27);
    }
    return -1;
}

// tutorial step 0x27: place the doctor. That ends the tutorial, with the
// level's last help page.
int tutorial_place_doctor(Tutorial &t)
{
    if (t.shown[10] == 2) {
        t.a.tutorial_highlight = 0;
        t.a.flags &= uint8_t(~kAdvisorTutorial);
    } else if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({10, 2, 6, 16});
        t.go(0x1f);
    } else if (!t.in.staff_place_ok) {
        t.message(59);
        t.shown[10] = 0;
        return 1;
    } else if (count_staff(t.world).doctors == 0) {
        if (t.shown[10] == 0) {
            t.message(39);
            t.shown[10] = 1;
            return 1;
        }
    } else {
        t.shown[10] = 2;
        t.a.tutorial_highlight = 0;
        t.a.flags &= uint8_t(~kAdvisorTutorial);
        if (t.in.help_page)
            *t.in.help_page = kTutorialEndPage;
    }
    return -1;
}

// The tutorial's steps (advisor.tutorial_step) and what runs them.
struct TutorialStep {
    uint8_t step;
    int (*run)(Tutorial &);
};

constexpr std::array<TutorialStep, 24> kTutorialSteps = {{
    {0x1d, tutorial_open_furnish_corridor}, {0x28, tutorial_buy_reception_desk}, {0x2a, tutorial_confirm_purchase},
    {0x29, tutorial_place_reception_desk},  {0x1e, tutorial_open_hire_staff},    {0x21, tutorial_select_receptionists},
    {0x25, tutorial_next_applicant},        {0x26, tutorial_previous_applicant}, {0x22, tutorial_hire_applicant},
    {0x24, tutorial_place_receptionist},    {0x20, tutorial_open_build_room},    {0x2b, tutorial_choose_department},
    {0x2c, tutorial_choose_gp_office},      {0x39, tutorial_drag_blueprint},     {0x2f, tutorial_valid_blueprint},
    {0x38, tutorial_accept_blueprint},      {0x30, tutorial_place_door},         {0x31, tutorial_place_windows},
    {0x32, tutorial_furnish_room},          {0x3a, tutorial_open_room},          {0x1f, tutorial_open_hire_doctor},
    {0x23, tutorial_select_doctors},        {0x2d, tutorial_hire_doctor},        {0x27, tutorial_place_doctor},
}};

} // namespace

inline namespace advisor_detail
{

// tutorial (src/tutorial.cpp), in high resolution, where the hints about
// controls are help text (set_force_help, which the advisor does not call) and the advisor is
// hidden. Returns 1 when a message was just given, 0 when he is to be
// hidden, -1 otherwise.
int tutorial(Advisor &a, const AdvisorInputs &in)
{
    trace_event("tutorial %u", unsigned(a.tutorial_step));
    // process_turn leaves a deleted room or a cancelled room edit as EV_CANCEL_BUILD in
    // old_packet (src/io.cpp:1478-1489).
    Event last = in.last;
    if (last.type == EV_DELETE_ROOM || last.type == EV_CANCEL_EDIT_ROOM)
        last = {EV_CANCEL_BUILD, last.type == EV_DELETE_ROOM ? last.value : uint16_t(0), last.flags, last.arg};
    Tutorial t{a,
               in,
               *in.world,
               a.tutorial_shown,
               last,
               in.pending,
               last.type == EV_CANCEL_BUILD || (last.type == EV_MARK_EMPLOYING && last.value != kStaffReceptionist)};
    for (const TutorialStep &step : kTutorialSteps)
        if (step.step == a.tutorial_step)
            return step.run(t);
    return -1;
}
} // namespace advisor_detail

} // namespace th
