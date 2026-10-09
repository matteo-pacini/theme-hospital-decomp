#include "common.h"

#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/render/iso.h"
#include "core/render/surface.h"
#include "core/render/view.h"
#include "core/ui/hire_staff.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/staff_complex.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace
{

// Carries out `e` as the app does: the interface's part, then the game's.
void hire_command(HireFixture &f, th::HireStaff &hire, th::Interface &ui, th::Event e)
{
    f.game->event = e;
    th::hire_staff_event(hire, ui, f.game->world, f.game->event);
    f.game->process_turn();
    th::hire_staff_after_turn(hire, f.game->world);
}

// Presses and releases button `button` (counted from 1) of the hire staff
// requester; returns what the interface did on the release.
th::InterfaceEvents click_hire_button(th::Interface &ui, int button)
{
    const th::Window &win = ui.window(int(th::Requester::HireStaff));
    const th::WindowButton &b = win.buttons[size_t(button - 1)];
    th::UiMouse m;
    m.x = win.x + b.x + b.width / 2;
    m.y = win.y + b.y + b.height / 2;
    m.left_pressed = m.left_held = true;
    ui.frame(m);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    return ui.frame(m);
}

} // namespace

namespace
{

// Whether applicant `t`'s wage is ability / divisor + the kind's minimum +
// the skills' additions, between the game's bounds for the kind.
bool wage_ok(const th::Thing &t, const th::Balance &settings)
{
    const th::Balance::Gbv &gbv = settings.gbv;
    int min_wage = settings.staff[t.subtype].min_salary;
    int max_wage = min_wage + 1000 / gbv.salary_ability_divisor;
    min_wage += 30 / gbv.salary_ability_divisor;
    for (size_t i = 3; i < 9; i++) {
        min_wage += std::min<int>(gbv.salary_add[i], 0);
        max_wage += std::max<int>(gbv.salary_add[i], 0);
    }
    return t.salary == t.wage_demand && t.salary >= min_wage && t.salary <= max_wage;
}

// Whether applicant `t` is one create_job_market makes for level 1.
bool applicant_ok(const th::Thing &t, const HireFixture &f)
{
    const th::Balance::Gbv &gbv = f.settings.gbv;
    bool valid = t.state == th::kStaffApplicant && t.owner == th::kNoOwner && t.cell_next == 0 && t.cell_prev == 0;
    valid &= th::applicant_expiry(t) >= 1 + 2500 && th::applicant_expiry(t) < 1 + 5000;
    valid &= t.initial >= 'A' && t.initial <= 'Z' && t.surname_a < 33 && t.surname_b < 26;
    valid &= t.ref_general < f.market.ref_counts[0] && t.ref_good < f.market.ref_counts[1] &&
             t.ref_bad < f.market.ref_counts[2];
    valid &= t.face_head < 18 && t.face_body < 18 && t.face_legs < 18;
    valid &= t.work_delay >= 1 && t.work_delay <= 10 && t.happiness <= 100;
    valid &= wage_ok(t, f.settings);
    if (t.subtype != th::kStaffDoctor)
        return valid && t.ability >= 30 && t.ability <= 1000;
    valid &= (t.skills & th::kSkillDoctor) != 0;
    bool junior = t.skills & th::kSkillJunior;
    valid &= junior ? t.ability >= 30 && t.ability <= gbv.doctor_threshold
                    : t.ability >= gbv.doctor_threshold && t.ability <= gbv.consultant_threshold;
    // FULL01.SAM: no surgeons, researchers or consultants yet.
    return valid && !(t.skills & (th::kSkillSurgeon | th::kSkillResearcher | th::kSkillConsultant));
}

} // namespace

void test_applicants_level1(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    const th::World &w = f.game->world;
    CHECK(f.market.ref_counts[0] > 0 && f.market.ref_counts[1] > 0 && f.market.ref_counts[2] > 0);
    // FULL01.SAM staff_levels[0]: 8 nurses, 8 doctors, 3 handymen, 8
    // receptionists.
    CHECK(th::count_applicants(w, th::kStaffNurse) == 8 && th::count_applicants(w, th::kStaffDoctor) == 8);
    CHECK(th::count_applicants(w, th::kStaffHandyman) == 3 && th::count_applicants(w, th::kStaffReceptionist) == 8);
    bool valid = true;
    long wages = 0;
    int psychiatrists = 0, juniors = 0;
    for (const th::Thing &t : w.things) {
        if (t.type != th::kThingStaff)
            continue;
        valid &= applicant_ok(t, f);
        if (t.subtype == th::kStaffDoctor) {
            psychiatrists += (t.skills & th::kSkillPsychiatrist) != 0;
            juniors += (t.skills & th::kSkillJunior) != 0;
        }
        wages += t.salary;
    }
    CHECK(valid);
    std::printf("applicants, level 1: wages %ld, %d psychiatrists, %d juniors, staff seed %#x\n", wages, psychiatrists,
                juniors, unsigned(f.market.seed));
    // Regression values, not checked against the game.
    CHECK(wages == 2936 && psychiatrists == 2 && juniors == 1 && f.market.seed == 0x47138585);
    // No draws from the simulation's generator.
    CHECK(w.seed == f.seed);
    // Again in month 0: everyone wanted is there, so no one is made.
    th::StaffMarket again = f.market;
    th::create_job_market(f.game->world, again, f.settings, 1, f.game->turn_count);
    CHECK(again.seed == f.market.seed && th::count_applicants(w, th::kStaffNurse) == 8);
}

namespace
{

// Opens the hire staff requester and takes the second receptionist through
// it to place. Returns them.
uint16_t take_receptionist(HireFixture &f, th::HireStaff &hire, th::Interface &ui)
{
    th::World &w = f.game->world;
    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    CHECK(ui.requester() == th::Requester::HireStaff && hire.mode == 1);
    // Hire is refused before a kind is chosen.
    CHECK(th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game).type == th::EV_NONE);

    th::Event e = th::hire_staff_click(hire, click_hire_button(ui, 0x29), *f.game);
    CHECK(e.type == th::EV_HIRE_SELECT_KIND && e.value == th::kStaffReceptionist);
    hire_command(f, hire, ui, e);
    uint16_t first = th::alter_applicant(w, true, 0, th::kStaffReceptionist);
    CHECK(hire.mode == 2 && hire.shown == first && first != 0);
    // Previous from the first is refused; next pages on.
    CHECK(th::hire_staff_click(hire, click_hire_button(ui, 0x2a), *f.game).type == th::EV_NONE);
    e = th::hire_staff_click(hire, click_hire_button(ui, 0x2d), *f.game);
    CHECK(e.type == th::EV_HIRE_NEXT_APPLICANT && e.value == 1);
    hire_command(f, hire, ui, e);
    uint16_t second = hire.shown;
    CHECK(second > first && w.things[second].subtype == th::kStaffReceptionist);

    e = th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game);
    CHECK(e.type == th::EV_MARK_EMPLOYING && e.arg == second && e.value == th::kStaffReceptionist);
    hire_command(f, hire, ui, e);
    CHECK(ui.window_count() == 0 && ui.requester() == th::Requester::None);
    CHECK(hire.placing() && hire.held == second && w.things[second].state == th::kStaffHeld);
    CHECK(th::count_applicants(w, th::kStaffReceptionist) == 8);
    return second;
}

// Puts held receptionist `id` down on the hospital's land and runs her
// first turn. Returns whether there was a cell for her.
bool place_receptionist(HireFixture &f, th::HireStaff &hire, th::Interface &ui, uint16_t id)
{
    th::World &w = f.game->world;
    // Off the hospital's land the click does nothing.
    int outside = f.level->entrances[0][0];
    CHECK(th::place_staff_click(hire, w, outside).type == th::EV_NONE && !hire.place_ok);
    const int cell = free_staff_cell(w, *f.level);
    CHECK(cell >= 0);
    if (cell < 0)
        return false;
    int balance = f.game->balance;
    int32_t salary = w.things[id].salary;
    th::Event e = th::place_staff_click(hire, w, cell);
    CHECK(e.type == th::EV_PLACE_STAFF && e.value == cell && e.arg == id && hire.place_ok);
    hire_command(f, hire, ui, e);
    const th::Thing &s = w.things[id];
    CHECK(!hire.placing());
    CHECK(s.type == th::kThingStaff && s.owner == 0 && s.state == th::kStaffReceptionistFindsReception);
    CHECK(s.x.tile + s.y.tile * th::kMapSize == cell && s.x.frac == 0x80 && s.y.frac == 0x80);
    CHECK(th::applicant_expiry(s) == 0 && s.hire_time == int16_t(f.game->turn_count) && s.speed != 0);
    CHECK(f.game->balance == balance - salary * 12 / 10);
    CHECK(th::count_applicants(w, th::kStaffReceptionist) == 7);
    bool listed = false;
    for (int t = w.map[size_t(cell)].first; t > 0; t = w.things[size_t(t)].cell_next)
        listed |= t == id;
    CHECK(listed && cell_lists_consistent(w));
    // With no reception desk, she goes to wander (and, not ported further,
    // stands).
    th::staff_turn(w, f.gd.anims(), f.gd.person_complexes(), f.gd.staff_complexes(), id, f.game->turn_count);
    CHECK(s.state == th::kStaffWanders && s.substate == 6);
    CHECK(s.sprite == f.gd.anims().starts[th::kStaffAnims[th::kStaffReceptionist].stand + s.facing].frame);
    return true;
}

} // namespace

void test_hire_receptionist(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    th::Interface ui;
    th::HireStaff hire;
    const uint16_t receptionist = take_receptionist(f, hire, ui);
    if (!place_receptionist(f, hire, ui, receptionist))
        return;

    // A nurse taken to place, then dropped: back on the list.
    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffNurse});
    uint16_t nurse = hire.shown;
    hire_command(f, hire, ui, th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game));
    CHECK(hire.held == nurse && w.things[nurse].state == th::kStaffHeld);
    hire_command(f, hire, ui, th::hire_staff_cancel(hire));
    CHECK(!hire.placing() && w.things[nurse].state == th::kStaffApplicant && ui.window_count() == 0);

    // An applicant withdraws once past their expiry.
    uint32_t expiry = th::applicant_expiry(w.things[nurse]);
    th::staff_turn(w, f.gd.anims(), f.gd.person_complexes(), f.gd.staff_complexes(), nurse, expiry);
    CHECK(w.things[nurse].type == th::kThingStaff);
    th::staff_turn(w, f.gd.anims(), f.gd.person_complexes(), f.gd.staff_complexes(), nurse, expiry + 1);
    CHECK(w.things[nurse].type == th::kThingFree && th::count_applicants(w, th::kStaffNurse) == 7);
}

namespace
{

void write_ppm(const char *path, const std::vector<uint32_t> &argb, int width, int height)
{
    if (FILE *out = std::fopen(path, "wb")) {
        std::fprintf(out, "P6\n%d %d\n255\n", width, height);
        for (uint32_t p : argb) {
            unsigned char rgb[3] = {uint8_t(p >> 16), uint8_t(p >> 8), uint8_t(p)};
            std::fwrite(rgb, 1, 3, out);
        }
        std::fclose(out);
    }
}

// The hire staff requester drawn over a clear interface, as an ARGB image.
struct HireImage {
    HireFixture &f;
    th::Interface &ui;
    th::HireStaff &hire;
    const th::RequesterArt &art;
    const th::HireStaffArt &hire_art;

    // Draws the interface; it covers most of the requester's window and
    // nothing outside it. Saved as `name` under TH_TEST_SCRATCH if set.
    std::vector<uint32_t> render(const char *name) const
    {
        const th::Window &win = ui.window(int(th::Requester::HireStaff));
        th::UiLayer layer;
        layer.resize(th::kUiWidth, th::kUiHeight);
        layer.clear();
        th::draw_interface(layer, ui, art);
        th::draw_hire_staff(layer, hire, ui, {art, hire_art, f.gd.text()}, f.game->world);
        std::vector<uint32_t> argb = layer.to_argb(f.gd.palette());
        int inside = 0, outside = 0;
        for (int y = 0; y < th::kUiHeight; y++)
            for (int x = 0; x < th::kUiWidth; x++)
                if (argb[size_t(y * th::kUiWidth + x)])
                    (x >= win.x && x < win.x + win.width && y >= win.y && y < win.y + win.height ? inside : outside)++;
        CHECK(inside > win.width * win.height * 9 / 10 && outside == 0);
        if (const char *scratch = std::getenv("TH_TEST_SCRATCH"))
            write_ppm((std::string(scratch) + "/" + name).c_str(), argb, th::kUiWidth, th::kUiHeight);
        return argb;
    }
};

// Whether every pixel of 320 x 240 `s` is the mask colour.
bool all_mask_colour(const th::Surface &s)
{
    return std::all_of(s.data(), s.data() + 320 * 240, [](uint8_t c) { return c == th::kMaskColour; });
}

// The held applicant under the pointer at the level's view start, coloured
// where they may be put down and greyed where not.
void check_held_staff_drawn(HireFixture &f, th::HireStaff &hire)
{
    th::Surface view, scratch, base;
    view.resize(320, 240);
    scratch.resize(320, 240);
    scratch.fill(th::kMaskColour);
    int start = f.level->camera_start[0];
    th::Point centre = th::cell_to_world(start % th::kMapSize, start / th::kMapSize);
    th::Point camera = {centre.x - 160, centre.y - 120};
    th::draw_world({view, {0, 0, 320, 240}, camera}, f.game->world, f.gd, nullptr, scratch);
    base = view;
    th::update_staff_cursor(hire, f.game->world, start);
    th::draw_held_staff({view, {0, 0, 320, 240}, camera}, hire, f.game->world, start, f.gd, scratch);
    int changed = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            changed += view.row(y)[x] != base.row(y)[x];
    CHECK(changed > 100);
    th::Surface coloured = view;
    view = base;
    hire.place_ok = false;
    th::draw_held_staff({view, {0, 0, 320, 240}, camera}, hire, f.game->world, start, f.gd, scratch);
    std::array<uint8_t, 256> grey = th::grey_remap(f.gd.palette());
    // All but the see-through pixels (shadows) are the coloured ones greyed.
    int greyed = 0, drawn = 0, differs = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            if (coloured.row(y)[x] != base.row(y)[x]) {
                greyed += view.row(y)[x] == grey[coloured.row(y)[x]];
                drawn++;
                differs += view.row(y)[x] != coloured.row(y)[x];
            }
    CHECK(drawn > 100 && greyed > drawn * 8 / 10 && differs > 0);
    CHECK(all_mask_colour(scratch));
}

} // namespace

void test_hire_requester_image(const char *dir)
{
    HireFixture f;
    th::RequesterArt art;
    th::HireStaffArt hire_art;
    std::string error;
    CHECK(f.start(dir) && art.load(dir, &error) && hire_art.load(dir, &error));
    if (!f.level)
        return;
    CHECK(hire_art.faces().size() == 65 * (29 + 24 + 22) * 18 && !hire_art.count_font().empty());
    CHECK(art.sheet(int(th::Requester::HireStaff)).size() > 0x128);
    th::Interface ui;
    th::HireStaff hire;
    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    const th::Window &win = ui.window(int(th::Requester::HireStaff));
    const HireImage image{f, ui, hire, art, hire_art};

    std::vector<uint32_t> blank = image.render("hire_blank.ppm");
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffDoctor});
    std::vector<uint32_t> doctor = image.render("hire_doctor.ppm");
    CHECK(doctor != blank);
    // The face is the faces file's pixels, inside its border.
    const th::Thing &d = f.game->world.things[hire.shown];
    size_t head = size_t(d.face_head) * 65 * 29;
    uint32_t want = f.gd.palette()[hire_art.faces()[head + 65 * 10 + 30]];
    CHECK(doctor[size_t((win.y + 128 - 75 - 3 + 10) * th::kUiWidth + win.x + 157 + 3 + 30)] == want);
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffHandyman});
    CHECK(image.render("hire_handyman.ppm") != doctor);

    hire_command(f, hire, ui, th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game));
    CHECK(hire.placing());
    check_held_staff_drawn(f, hire);
}

namespace
{

// Staff at work on level 1 from its start: the first applicant of `kind`
// hired and put down on the free cell of the player's land nearest the
// level's view start, or 0 if none could be.
uint16_t hire_and_place(HireFixture &f, uint8_t kind)
{
    th::World &w = f.game->world;
    uint16_t id = 0;
    for (uint16_t i = 1; i < w.things.size() && !id; i++)
        if (w.things[i].type == th::kThingStaff && w.things[i].subtype == kind &&
            w.things[i].state == th::kStaffApplicant)
            id = i;
    int start = f.level->camera_start[0];
    for (int r = 0; r < 20 && id; r++)
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++) {
                int c = start + dx + dy * th::kMapSize;
                if (c < 0 || c >= th::kMapCells || w.map[size_t(c)].first != 0 || !th::staff_place_ok(w, c, kind))
                    continue;
                bool placed = th::place_staff_on_map(w, {f.gd.anims(), f.settings, f.game->turn_count}, c, id, false);
                return placed ? id : 0;
            }
    return 0;
}

// Runs the staff for a turn, in thing order, as process_things does.
void staff_work_turn(HireFixture &f, const th::StaffComplexes &sc)
{
    th::World &w = f.game->world;
    f.game->turn_count++;
    for (uint16_t i = 1; i < w.things.size(); i++)
        if (w.things[i].type == th::kThingStaff)
            th::staff_turn(w, f.gd.anims(), f.gd.person_complexes(), sc, i, f.game->turn_count);
}

// A receptionist with no desk wanders off; given a desk facing 0 a few cells
// off she finds it, walks to it and mans it, then keeps it with no one
// queueing. Returns whether she could be hired and the desk placed;
// `consistent` is cleared if the cell lists ever were not.
bool check_receptionist_mans_desk(HireFixture &f, const th::StaffComplexes &sc, bool &consistent)
{
    th::World &w = f.game->world;
    uint16_t recep = hire_and_place(f, th::kStaffReceptionist);
    CHECK(recep != 0);
    if (!recep)
        return false;
    const th::Thing &r = w.things[recep];
    CHECK(r.state == th::kStaffReceptionistFindsReception);
    staff_work_turn(f, sc);
    CHECK(r.state == th::kStaffWanders && r.substate == 6);

    uint16_t desk = 0;
    int start = r.x.tile + r.y.tile * th::kMapSize;
    for (int d = 3; d < 10 && !desk; d++)
        desk = th::place_object(w, f.gd.anims(), {start + d, 11, 0}, 0, false);
    CHECK(desk != 0);
    if (!desk)
        return false;
    const th::Thing &dk = w.things[desk];
    int turns = 0;
    bool walked = false;
    while (turns < 2000 && !(dk.user == recep && r.state == th::kStaffReceptionistWorks)) {
        staff_work_turn(f, sc);
        turns++;
        walked |= r.state == th::kStaffGotoObject;
        consistent &= cell_lists_consistent(w);
    }
    CHECK(walked && consistent);
    CHECK(dk.in_use == 1 && dk.user == recep && dk.state == 4 && r.object == desk);
    CHECK(r.x.tile == dk.use_x.tile && r.y.tile == dk.use_y.tile && r.facing == dk.facing);
    CHECK(r.room != 0 && w.rooms[r.room].type == th::kRoomReception && w.rooms[r.room].owner == 0);
    std::printf("receptionist manned the desk in %d turns, seed %#x\n", turns, unsigned(w.seed));
    // Regression values, not checked against the game.
    CHECK(turns == 45 && w.seed == 0x7616f6f8);

    // At the desk with no one queueing she keeps it, drawing for her
    // idle tasks.
    uint32_t seed = w.seed;
    for (int i = 0; i < 500; i++) {
        staff_work_turn(f, sc);
        consistent &= cell_lists_consistent(w);
    }
    CHECK(consistent && w.seed != seed);
    CHECK(dk.user == recep && r.state == th::kStaffReceptionistWorks);
    return true;
}

// Runs the staff for 1000 turns. Returns the turns `id` was away from the
// cell it started on; `consistent` is cleared if the cell lists ever were
// not.
int turns_away(HireFixture &f, const th::StaffComplexes &sc, uint16_t id, bool &consistent)
{
    const th::World &w = f.game->world;
    const th::Thing &t = w.things[id];
    int from = t.x.tile + t.y.tile * th::kMapSize;
    int moved = 0;
    for (int i = 0; i < 1000; i++) {
        staff_work_turn(f, sc);
        moved += t.x.tile + t.y.tile * th::kMapSize != from;
        consistent &= cell_lists_consistent(w);
    }
    return moved;
}

// One run of test_staff_at_work_level1 from the level's start. Returns
// whether it got to the end.
bool staff_at_work_run(const char *dir, const th::StaffComplexes &sc)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return false;
    th::World &w = f.game->world;
    bool consistent = true;
    if (!check_receptionist_mans_desk(f, sc, consistent))
        return false;

    // A doctor wanders, drawing from the generator every turn.
    uint16_t doctor = hire_and_place(f, th::kStaffDoctor);
    CHECK(doctor != 0);
    if (!doctor)
        return false;
    const th::Thing &d = w.things[doctor];
    int moved = turns_away(f, sc, doctor, consistent);
    CHECK(consistent && moved > 0 && d.state == th::kStaffWanders);
    std::printf("doctor at %d,%d after 1000 turns, away %d turns, seed %#x\n", d.x.tile, d.y.tile, moved,
                unsigned(w.seed));
    CHECK(d.x.tile == 59 && d.y.tile == 76 && moved == 994 && w.seed == 0xa5fc3943);

    // A handyman, with nothing to sweep, water or repair, strolls.
    uint16_t handyman = hire_and_place(f, th::kStaffHandyman);
    CHECK(handyman != 0);
    if (!handyman)
        return false;
    const th::Thing &h = w.things[handyman];
    moved = turns_away(f, sc, handyman, consistent);
    CHECK(consistent && moved > 0);
    std::printf("handyman at %d,%d after 1000 turns, away %d turns, seed %#x\n", h.x.tile, h.y.tile, moved,
                unsigned(w.seed));
    CHECK(h.x.tile == 65 && h.y.tile == 62 && moved == 987 && w.seed == 0xefe8e4f0);
    return true;
}

} // namespace

// With the game data: S_COMP.LEX's complex 80 (walk to the desk, take it);
// on level 1, a receptionist put down near a reception desk walks to it,
// mans it (the desk in use, hers, she at its use position) and works there
// in a reception of her own, idling at the desk with no queue; a doctor
// wanders the corridor. Fixed turn counts and generator from the level's
// start, and the cell lists consistent every turn. A handyman strolls.
void test_staff_at_work_level1(const char *dir)
{
    th::StaffComplexes sc;
    std::string error;
    CHECK(th::load_staff_complexes(dir, sc, &error));
    CHECK(sc.group.size() == 2000 && sc.at(80) == 14 && sc.at(81) == 6 && sc.at(82) == 126 && sc.at(83) == 0);
    for (int run = 0; run < 2; run++)
        if (!staff_at_work_run(dir, sc))
            return;
}

// With the game data: a handyman on level 1 sweeps litter put beside him,
// and the cell is clean afterwards.
void test_handyman_sweeps(const char *dir)
{
    th::StaffComplexes sc;
    std::string error;
    CHECK(th::load_staff_complexes(dir, sc, &error));
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    uint16_t handyman = hire_and_place(f, th::kStaffHandyman);
    CHECK(handyman != 0);
    if (!handyman)
        return;
    const th::Thing &h = w.things[handyman];
    int cell = 0;
    for (int offset : {1, -1, th::kMapSize, -th::kMapSize}) {
        int c = h.x.tile + h.y.tile * th::kMapSize + offset;
        if (w.map[size_t(c)].owner_flags == 1) {
            cell = c;
            break;
        }
    }
    CHECK(cell != 0);
    th::place_litter(w, uint16_t(cell), 0, 2);
    CHECK((w.map[size_t(cell)].litter_shadow & 0xf) == 2);
    int turns = 0;
    while (turns < 2000 && (w.map[size_t(cell)].litter_shadow & 0xf) != 0) {
        staff_work_turn(f, sc);
        turns++;
    }
    std::printf("handyman swept in %d turns, seed %#x\n", turns, unsigned(w.seed));
    CHECK((w.map[size_t(cell)].litter_shadow & 0xf) == 0 && h.litter_count == 1);
    // Regression values, not checked against the game.
    CHECK(turns == 44 && w.seed == 0xe7957f81);
}

// With the game data: a handyman on level 1 waters a plant beside him that a
// handyman page asks for, and it is healthy and counted as watered
// afterwards. plant_ages, which makes the page, is checked by the plant
// replay scripts.
void test_handyman_waters(const char *dir)
{
    th::StaffComplexes sc;
    std::string error;
    CHECK(th::load_staff_complexes(dir, sc, &error));
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    uint16_t handyman = hire_and_place(f, th::kStaffHandyman);
    CHECK(handyman != 0);
    if (!handyman)
        return;
    const th::Thing &h = w.things[handyman];
    int cell = 0;
    for (int offset : {1, -1, th::kMapSize, -th::kMapSize}) {
        int c = h.x.tile + h.y.tile * th::kMapSize + offset;
        if (w.map[size_t(c)].owner_flags == 1 && w.map[size_t(c)].first == 0) {
            cell = c;
            break;
        }
    }
    CHECK(cell != 0);
    uint16_t plant = th::place_object(w, f.gd.anims(), {cell, 45, 0}, 0, true);
    CHECK(plant != 0);
    if (!plant)
        return;
    w.things[plant].plant_health = 300;
    th::thingpage_request(w.plant_pager, plant, 1);
    int turns = 0;
    while (turns < 4000 && w.things[plant].plant_health != 400) {
        staff_work_turn(f, sc);
        turns++;
    }
    std::printf("handyman watered in %d turns, seed %#x\n", turns, unsigned(w.seed));
    CHECK(w.things[plant].plant_health == 400 && h.plant_count == 1);
}

namespace
{

// The first `count` free cells of the hospital nearest the camera's start
// where a receptionist may be put down.
std::vector<int> free_staff_cells(const th::World &w, const th::Level &level, size_t count)
{
    std::vector<int> cells;
    int start = level.camera_start[0];
    for (int r = 0; r < 20 && cells.size() < count; r++)
        for (int dy = -r; dy <= r && cells.size() < count; dy++)
            for (int dx = -r; dx <= r && cells.size() < count; dx++) {
                int c = start + dx + dy * th::kMapSize;
                if (c >= 0 && c < th::kMapCells && w.map[size_t(c)].first == 0 &&
                    th::staff_place_ok(w, c, th::kStaffReceptionist) &&
                    std::find(cells.begin(), cells.end(), c) == cells.end())
                    cells.push_back(c);
            }
    return cells;
}

// Hired staff member `id`, picked up, is off the map and held, and placed
// again on `cell` costs nothing and keeps their hire turn `hired` and
// discontent. Returns the balance before they were placed again.
int check_picked_up_and_placed(HireFixture &f, th::HireStaff &hire, th::Interface &ui, uint16_t id, int cell,
                               int16_t hired)
{
    th::World &w = f.game->world;
    const th::Thing &s = w.things[id];
    // Picked up: off the map, held, and placed by the interface.
    int taken = s.x.tile + s.y.tile * th::kMapSize;
    th::Event e = th::pick_up_staff_click(w, id);
    CHECK(e.type == th::EV_PICK_UP_STAFF && e.value == id);
    hire_command(f, hire, ui, e);
    CHECK(s.state == th::kStaffHeld && w.held_staff == id && w.held_staff_tile == taken && s.room == 0);
    CHECK(hire.placing() && hire.held == id && hire.held_kind == th::kStaffNurse);
    bool listed = false;
    for (int t = w.map[size_t(taken)].first; t > 0; t = w.things[size_t(t)].cell_next)
        listed |= t == id;
    CHECK(!listed && cell_lists_consistent(w));

    // Placed again: no fee, the hire turn and discontent kept.
    w.things[id].discontent = 7;
    int balance = f.game->balance;
    e = th::place_staff_click(hire, w, cell);
    CHECK(e.type == th::EV_PLACE_STAFF && e.arg == id);
    hire_command(f, hire, ui, e);
    CHECK(!hire.placing() && w.held_staff == 0 && w.held_staff_tile == 0);
    CHECK(s.x.tile + s.y.tile * th::kMapSize == cell && s.state != th::kStaffHeld);
    CHECK(f.game->balance == balance && s.hire_time == hired && s.discontent == 7);
    return balance;
}

} // namespace

// Picking up hired staff (EV_PICK_UP_STAFF, src/io.cpp): hiring charges the
// fee once; a member of staff picked up and placed again costs nothing and
// keeps their hire turn and discontent; cancelling while held puts them back
// where they were taken from.
void test_pick_up_staff(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    th::Interface ui;
    th::HireStaff hire;

    std::vector<int> cells = free_staff_cells(w, *f.level, 2);
    CHECK(cells.size() == 2);
    if (cells.size() != 2)
        return;

    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffNurse});
    uint16_t id = hire.shown;
    CHECK(id != 0);
    CHECK(th::pick_up_staff_click(w, id).type == th::EV_NONE);
    hire_command(f, hire, ui, th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game));
    CHECK(hire.held == id);
    int balance = f.game->balance;
    int32_t fee = w.things[id].salary * 12 / 10;
    hire_command(f, hire, ui, th::place_staff_click(hire, w, cells[0]));
    const th::Thing &s = w.things[id];
    CHECK(!hire.placing() && s.owner == 0 && f.game->balance == balance - fee);
    int applicants = th::count_applicants(w, th::kStaffNurse);
    for (int i = 0; i < 100; i++)
        f.game->turn();
    int16_t hired = s.hire_time;
    CHECK(hired != int16_t(f.game->turn_count));
    balance = check_picked_up_and_placed(f, hire, ui, id, cells[1], hired);
    CHECK(th::count_applicants(w, th::kStaffNurse) == applicants && cell_lists_consistent(w));

    // Picked up and dropped: back on the cell they were taken from.
    for (int i = 0; i < 10; i++)
        f.game->turn();
    int taken = s.x.tile + s.y.tile * th::kMapSize;
    hire_command(f, hire, ui, th::pick_up_staff_click(w, id));
    CHECK(hire.held == id && s.state == th::kStaffHeld);
    hire_command(f, hire, ui, th::hire_staff_cancel(hire));
    CHECK(!hire.placing() && w.held_staff == 0 && s.state != th::kStaffHeld && s.state != th::kStaffApplicant);
    CHECK(s.x.tile + s.y.tile * th::kMapSize == taken && s.x.frac == 0x80 && s.y.frac == 0x80);
    CHECK(f.game->balance == balance && s.hire_time == hired && cell_lists_consistent(w));
}

namespace
{

// Whether the game's recorded events include `text`.
bool logged(const th::Game &game, const std::string &text)
{
    for (const th::TraceEvent &e : game.events.events)
        if (e.text == text)
            return true;
    return false;
}

// Whether thing `id` is in its cell's list.
bool on_map(const th::World &w, uint16_t id)
{
    const th::Thing &t = w.things[id];
    for (int i = w.map[size_t(t.x.tile + t.y.tile * th::kMapSize)].first; i > 0; i = w.things[size_t(i)].cell_next)
        if (i == id)
            return true;
    return false;
}

// Sacked doctor `doctor` walks out of the office, then to the entrance's door,
// where he goes off the map back to the applicants, withdrawn the turn
// after.
void check_sacked_doctor_leaves(QueueFixture &f, uint16_t doctor)
{
    const th::Game &g = *f.game;
    const th::World &w = g.world;
    const th::Thing &d = w.things[doctor];
    const uint16_t entrance = w.rooms[th::get_room_info(w, th::kRoomEntrance, 0x65, 0)].door_tile;
    bool left_office = false;
    int turns = 0;
    for (; turns < 3000 && d.state != th::kStaffApplicant; turns++) {
        f.turn();
        left_office |= d.room == 0;
    }
    std::printf("sacked doctor: back to the applicants after %d turns, seed %#x\n", turns, unsigned(w.seed));
    CHECK(left_office && d.state == th::kStaffApplicant && d.x.tile + d.y.tile * th::kMapSize == entrance);
    CHECK(!on_map(w, doctor) && cell_lists_consistent(w) && d.hire_time == int16_t(g.turn_count));
    CHECK(d.owner == th::kNoOwner && th::applicant_expiry(d) == 0);
    f.turn();
    CHECK(d.type == th::kThingFree);
}

// Whether a severance payment is among `fin`'s lines from `from` on.
bool paid_severance(const th::Finance &fin, size_t from)
{
    bool severance = false;
    for (size_t i = from; i < fin.entry_count; i++)
        severance |= fin.entries[i].type == th::kTxSeverancePay;
    return severance;
}

} // namespace

// Sacking the GP's doctor (EV_SACK_STAFF, sack_staff mode 0) while at the
// desk: a month's wage in severance pay, no longer the hospital's (nor his
// skill, so general practice is no longer available) and the office free;
// he walks out of the office and to the first entrance, where he goes off
// the map back to the applicants, withdrawn the turn after.
void test_sack_doctor(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    start_rooms(f);
    th::Game &g = *f.game;
    th::World &w = g.world;
    const int start = f.level->camera_start[0];
    CHECK(hire_near(f, th::kStaffReceptionist, start) && f.place_near(start + 3, 11));
    uint16_t gp = lay_out_gp_office(f, start - 10 * th::kMapSize);
    CHECK(gp != 0 && finish_gp_office(f, gp));
    uint16_t doctor = gp ? hire_near(f, th::kStaffDoctor, start + 2 * th::kMapSize) : 0;
    CHECK(doctor != 0);
    if (!doctor)
        return;
    const th::Thing &d = w.things[doctor];
    for (int i = 0; i < 3000 && !(d.room == gp && d.state == th::kStaffDoctorWorksAtDesk); i++)
        f.turn();
    CHECK(d.room == gp && d.state == th::kStaffDoctorWorksAtDesk && w.rooms[gp].staff[0] == doctor);
    CHECK(w.cure_available[th::kIllGeneralPractice] == 1 && th::staff_with_skill(w, 0, 4) == 1);

    const int applicants = th::count_applicants(w, th::kStaffDoctor);
    const int32_t balance = g.balance, salary = d.salary;
    const uint8_t work_delay = d.work_delay;
    g.event = {th::EV_SACK_STAFF, doctor, 0, 0};
    g.process_turn();
    CHECK(logged(g, "sack_staff " + std::to_string(doctor) + " mode=0"));
    CHECK(g.balance == balance - salary && g.finance.entry_count > 0 &&
          g.finance.entries[g.finance.entry_count - 1u].type == th::kTxSeverancePay);
    CHECK(d.owner == th::kNoOwner && d.happiness == 100 && d.orders == 0 && d.discontent == 0);
    CHECK(d.work_delay == work_delay + work_delay / 4 && d.next_state == th::kStaffBackToApplicants);
    CHECK(w.rooms[gp].staff[0] == 0 && w.cure_available[th::kIllGeneralPractice] == 0);
    CHECK(th::staff_with_skill(w, 0, 4) == 0 && th::count_applicants(w, th::kStaffDoctor) == applicants);
    CHECK(logged(g, "set_complex " + std::to_string(doctor) +
                        " action=310 arg=" + std::to_string(w.rooms[gp].door_inside_tile)));

    check_sacked_doctor_leaves(f, doctor);
}

// A discontented doctor hired long enough ago hands in his notice (1500
// turns, a player call); when it runs out he leaves (sack_staff mode 1,
// shall_i_resign) with no severance pay, the call removed, and goes back to
// the applicants at the entrance.
void test_staff_notice_runs_out(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    start_rooms(f);
    th::Game &g = *f.game;
    th::World &w = g.world;
    uint16_t doctor = hire_near(f, th::kStaffDoctor, f.level->camera_start[0]);
    CHECK(doctor != 0);
    if (!doctor)
        return;
    const th::Thing &d = w.things[doctor];
    // Up to nine months of turns, discontented: notice handed in.
    for (int i = 0; i < 14000 && d.orders == 0; i++) {
        w.things[doctor].discontent = 1000;
        f.turn();
    }
    CHECK(d.orders == 0x20 && d.resign_count == 1 && d.resign_time == g.turn_count + 1500 && d.playercall != 0);
    const uint16_t call = d.playercall;
    CHECK(th::check_playercall(w, 1, doctor, 0) == 1);
    const uint32_t notice_end = d.resign_time;

    while (g.turn_count < notice_end - 1)
        f.turn();
    CHECK(d.owner == 0 && d.orders == 0x20);
    const size_t entries = g.finance.entry_count;
    for (int i = 0; i < 1000 && d.owner == 0; i++)
        f.turn();
    CHECK(logged(g, "sack_staff " + std::to_string(doctor) + " mode=1"));
    CHECK(logged(g, "get_playercall " + std::to_string(call) + " type=1 owner=0"));
    CHECK(d.owner == th::kNoOwner && d.orders == 0 && d.resign_count == 0 && d.resign_time == 0 && d.playercall == 0);
    CHECK(th::check_playercall(w, 1, doctor, 0) == 0 && th::staff_with_skill(w, 0, 4) == 0);
    CHECK(!paid_severance(g.finance, entries));
    int turns = 0;
    for (; turns < 3000 && d.state != th::kStaffApplicant; turns++)
        f.turn();
    std::printf("notice ran out: left at turn %u, back to the applicants %d turns later, seed %#x\n",
                unsigned(notice_end), turns, unsigned(w.seed));
    CHECK(d.state == th::kStaffApplicant && !on_map(w, doctor) &&
          d.x.tile + d.y.tile * th::kMapSize == w.rooms[th::get_room_info(w, th::kRoomEntrance, 0x65, 0)].door_tile);
}

// adjust_handyman_values (src/req4.cpp): a priority up by 6, taken from the
// larger of the other two; from 94 it takes everything; any button starts
// the job counts again.
void test_adjust_handyman_values()
{
    auto w = std::make_unique<th::World>();
    th::Thing &h = w->things[3];
    h.type = th::kThingStaff;
    h.subtype = th::kStaffHandyman;
    h.prio_litter = 33;
    h.prio_plants = 33;
    h.prio_repair = 34;
    h.litter_count = 5;
    h.plant_count = 6;
    h.repair_count = 7;
    th::adjust_handyman_values(*w, 3, th::kHandymanLitterButton);
    CHECK(h.prio_litter == 39 && h.prio_plants == 31 && h.prio_repair == 30);
    CHECK(h.litter_count == 0 && h.plant_count == 0 && h.repair_count == 0);
    th::adjust_handyman_values(*w, 3, th::kHandymanPlantsButton);
    CHECK(h.prio_litter == 33 && h.prio_plants == 37 && h.prio_repair == 30);
    for (int i = 0; i < 20; i++)
        th::adjust_handyman_values(*w, 3, th::kHandymanRepairButton);
    CHECK(h.prio_litter == 0 && h.prio_plants == 0 && h.prio_repair == 100);
    th::adjust_handyman_values(*w, 3, th::kHandymanLitterButton);
    CHECK(h.prio_litter == 6 && h.prio_plants == 0 && h.prio_repair == 94);
    h.repair_count = 9;
    th::adjust_handyman_values(*w, 3, 15);
    CHECK(h.prio_litter == 6 && h.prio_plants == 0 && h.prio_repair == 94 && h.repair_count == 0);
}

// applicant_expiry/set_applicant_expiry (staff.cpp): the game's u32 at
// Thing+0x94, whose high half is work_target2 for a handyman and crazy with
// surgeon_training for a doctor (include/hosp.h). A sacked handyman or doctor
// keeps whatever those bytes held, so the expiry they are withdrawn at is the
// game's; the two kinds never use each other's fields.
void test_applicant_expiry()
{
    th::Thing handyman;
    handyman.type = th::kThingStaff;
    handyman.subtype = th::kStaffHandyman;
    handyman.work_target = 0x002a;
    handyman.work_target2 = 0x1234;
    CHECK(th::applicant_expiry(handyman) == 0x1234002au);

    th::Thing doctor;
    doctor.type = th::kThingStaff;
    doctor.subtype = th::kStaffDoctor;
    doctor.work_target = 0x002a;
    doctor.crazy = 0x56;
    doctor.surgeon_training = 0x78;
    CHECK(th::applicant_expiry(doctor) == 0x7856002au);

    th::set_applicant_expiry(handyman, 0x000ab0cd);
    CHECK(handyman.work_target == 0xb0cd && handyman.work_target2 == 0x000a);
    CHECK(th::applicant_expiry(handyman) == 0x000ab0cdu);
    th::set_applicant_expiry(doctor, 0x000ab0cd);
    CHECK(doctor.work_target == 0xb0cd && doctor.crazy == 0x0a && doctor.surgeon_training == 0x00);
    CHECK(th::applicant_expiry(doctor) == 0x000ab0cdu);
}
