#include "common.h"

#include "core/data/anims.h"
#include "core/data/level.h"
#include "core/world/epidemic.h"
#include "core/world/queue.h"
#include "core/world/vaccine.h"
#include "core/world/world.h"

namespace
{

// A patient of hospital 0 on cell (10, 20 + id), inside the building.
th::Thing &a_patient(th::World &w, uint16_t id, uint8_t illness, uint8_t contagion)
{
    th::Thing &p = w.things[id];
    p.type = th::kThingPatient;
    p.owner = 0;
    p.illness = illness;
    p.contagion = contagion;
    p.x = {0x80, 10};
    p.y = {0x80, uint8_t(20 + id)};
    return p;
}

// count_and_flag_contagious_peeps: mode 1 flags a contagious patient of the
// epidemic's illness and counts it; a fed-up one is cured instead, an
// outdoor one loses its contagion, and another illness is left. Mode 2
// cleans the flagged and counts what was flagged.
void count_and_flag()
{
    using namespace th;
    World w;
    AnimData anims;
    Thing &p1 = a_patient(w, 1, 29, 5);
    Thing &p2 = a_patient(w, 2, 29, 3);
    p2.person_flags |= kPfFedUp;
    Thing &p3 = a_patient(w, 3, 29, 4);
    w.map[size_t(p3.x.tile + (p3.y.tile << 7))].owner_flags = kCellOutdoor;
    a_patient(w, 4, 3, 2);
    CHECK(count_and_flag_contagious_peeps(w, anims, 29, 1, 0) == 1);
    CHECK(p1.person_flags & kPfContagious);
    CHECK(!(p2.person_flags & kPfContagious) && p2.contagion == 0);
    CHECK(!(p3.person_flags & kPfContagious) && p3.contagion == 0);
    CHECK(!(w.things[4].person_flags & kPfContagious) && w.things[4].contagion == 2);
    CHECK(count_and_flag_contagious_peeps(w, anims, 29, 2, 0) == 1);
    CHECK(p1.contagion == 0 && !(p1.person_flags & kPfContagious));
    CHECK(count_and_flag_contagious_peeps(w, anims, 29, 2, 0) == 0);
}

// epidemic_status ends the cover-up when no contagious patient is left, and
// otherwise counts the timer down to 0.
void cover_up_status()
{
    using namespace th;
    World w;
    w.epidemic.active = 1;
    w.epidemic.timer = 5;
    w.epidemic.contagious_count = 0;
    CHECK(epidemic_status(w, 0) == 1);
    CHECK(!w.epidemic.active && w.epidemic.over);
    CHECK(w.epidemic_sounds.size() == 1);
    w.epidemic.active = 1;
    w.epidemic.over = 0;
    w.epidemic.timer = 2;
    w.epidemic.contagious_count = 3;
    CHECK(epidemic_status(w, 0) == 0 && w.epidemic.timer == 1);
    CHECK(epidemic_status(w, 0) == 1 && w.epidemic.timer == 0);
}

// The vaccination queue keeps its order and takes patients out;
// is_person_highlighted_for_vaccination picks the player's contagious
// patient of the epidemic's illness not already queued; clean_contagious_peep
// clears the contagion and the queue entry; send_peeps_home sends the
// contagious (or every) indoor patient home.
void vaccine_and_home()
{
    using namespace th;
    World w;
    AnimData anims;
    a_patient(w, 1, 29, 4);
    a_patient(w, 2, 29, 4);
    a_patient(w, 3, 29, 4);
    request_vaccination(w, anims, 1);
    request_vaccination(w, anims, 3);
    CHECK(w.epidemic.vaccine_queue == 1);
    CHECK(w.things[1].vaccine_next == 3);
    CHECK(is_person_in_vaccine_queue(w, 1) && is_person_in_vaccine_queue(w, 3));
    CHECK(!is_person_in_vaccine_queue(w, 2));
    CHECK(w.epidemic.vaccine_requests == 2);
    CHECK(remove_person_from_vaccination_queue(w, 1) == 1);
    CHECK(w.epidemic.vaccine_queue == 3);
    CHECK(remove_person_from_vaccination_queue(w, 2) == 0);

    w.epidemic.illness = 29;
    w.things[1].contagion = 4;
    w.things[1].person_flags = 0;
    uint16_t target = 7;
    CHECK(is_person_highlighted_for_vaccination(w, 1, target) && target == 1);
    request_vaccination(w, anims, 1);
    CHECK(!is_person_highlighted_for_vaccination(w, 1, target) && target == 0);
    w.things[2].illness = 3;
    w.things[2].contagion = 4;
    CHECK(!is_person_highlighted_for_vaccination(w, 2, target));

    CHECK(is_person_in_vaccine_queue(w, 1));
    clean_contagious_peep(w, 1);
    CHECK(w.things[1].contagion == 0 && !(w.things[1].person_flags & kPfContagious));
    CHECK(!is_person_in_vaccine_queue(w, 1));

    a_patient(w, 4, 29, 0);
    w.things[3].person_flags |= kPfContagious;
    send_peeps_home(w, 0, 0);
    CHECK(w.things[3].person_flags & kPfSentHomeByInspector);
    CHECK(!(w.things[3].person_flags & kPfContagious) && w.things[3].contagion == 0);
    CHECK(!(w.things[4].person_flags & kPfSentHomeByInspector));
    send_peeps_home(w, 1, 0);
    CHECK(w.things[4].person_flags & kPfSentHomeByInspector);
}

} // namespace

void test_epidemic()
{
    count_and_flag();
    cover_up_status();
    vaccine_and_home();
}
