#include "common.h"

#include "core/ui/staff_demand.h"
#include "core/ui/windows.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <cstdint>
#include <string>

// req12 and open_staff_resign_requester (core/ui/staff_demand.cpp): the
// demand requester opens for a member of staff, its kind is chosen from their
// state, and its two buttons give the demand's answer commands.
void test_staff_demand(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));

    uint16_t staff = 0;
    for (uint16_t i = 1; i < f.game->world.things.size(); i++)
        if (f.game->world.things[i].type == th::kThingStaff) {
            staff = i;
            break;
        }
    CHECK(staff != 0);

    th::StaffDemand demand;
    th::Interface ui;
    uint32_t safeseed = 0x12345678;
    th::open_staff_resign_requester(demand, ui, f.game->world, staff, safeseed);
    CHECK(ui.requester() == th::Requester::StaffDemand);
    CHECK(demand.staff == staff);
    CHECK(demand.kind >= 2 && demand.kind <= 8);

    // The granted button (19) gives the kind's command; the refused one (20)
    // refuses the demand.
    th::InterfaceEvents ev;
    ev.triggered_window = int(th::Requester::StaffDemand);
    ev.triggered_button = 19;
    bool accept = false;
    th::Event e = th::staff_demand_click(demand, ev, accept);
    CHECK(accept);
    CHECK(e.type != 0);
    ev.triggered_button = 20;
    th::Event refused = th::staff_demand_click(demand, ev, accept);
    CHECK(!accept);
    CHECK(refused.type == th::EV_REFUSE_STAFF_DEMAND);
    CHECK(refused.value == staff);
}
