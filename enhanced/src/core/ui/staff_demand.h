#pragma once

#include "core/ui/hire_staff.h"
#include "core/ui/windows.h"
#include "core/world/events.h"

#include <cstdint>

namespace th
{

class GameData;
struct Thing;
struct World;

// The staff demand requester's state (the game's crequester while it is
// REQ_STAFF_DEMAND): the request kind (crequester.kind), the member of staff
// shown (crequester.arg) and the request's amount (crequester.arg2).
struct StaffDemand {
    uint8_t kind = 0;
    uint16_t staff = 0;
    int32_t amount = 0;
};

// open_staff_resign_requester (0x6bda8): opens the staff demand requester for
// `staff`, choosing the request kind and amount from their wage demand,
// tiredness and happiness (the draws from the safe seed `safeseed`). A rival's
// poaching offer (kind 9) is not ported.
void open_staff_resign_requester(StaffDemand &demand, Interface &ui, const World &world, uint16_t staff,
                                 uint32_t &safeseed);

// req12 (0x6b1f4): the command the demand requester's clicked button gives
// (the interface's triggered button in window Requester::StaffDemand: 19 the
// demand granted, 20 refused), with `accept` set for the first. EV_NONE for
// other clicks.
Event staff_demand_click(const StaffDemand &demand, const InterfaceEvents &events, bool &accept);

// req12_overdraw (0x6b464): the member of staff's name, wage, face,
// qualifications and ability, the request's text and a view of them. The map
// view window is drawn as their current frame centred in its box.
void draw_staff_demand(UiLayer &layer, const StaffDemand &demand, const Interface &ui, const WindowArt &art,
                       const GameData &data, const World &world);

} // namespace th
