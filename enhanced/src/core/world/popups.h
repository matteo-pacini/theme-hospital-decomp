#pragma once

#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

// The panel's fax icons (src/panel.cpp) as the game keeps them with its
// calls: World::popups and each call's PlayerCall::slot and slide_step.
// Only hospital 0's calls (the player's) take a slot. A kind of call shows
// one icon, for its head call (World::first_call); slots are taken from the
// left as calls come and closed up as they go. core/ui/popups.h draws them.

// Per call kind (PlayerCall::type, 1-8), the panel sprite of its icon (the
// game's popup_icons, src/tables.cpp): a resignation, an emergency, an
// epidemic, a patient with no treatment, a report (an emergency's result),
// a VIP, a new illness, a patient with no diagnosis.
inline constexpr std::array<uint8_t, 9> kPopupIcons = {0, 47, 43, 45, 51, 55, 49, 53, 51};

// add_panel_popup (0x35760): call `call` (a World::calls index) of hospital
// 0 gets the first slot free (rebuild_panel_popup_pos first), starts its
// slide (slide_step 1) and closes the message bar's shutter (state 2); with
// all nine slots taken it gets slot 9 and no icon. Counts a new fax for
// the interface's sound (PanelPopups::new_faxes).
void add_panel_popup(World &world, uint16_t call);

// delete_panel_popup (0x35624): for a call of hospital 0, removes its icon
// from its slot and closes up any empty slot (remove_panel_popup).
void delete_panel_popup(World &world, uint16_t call);

// remove_panel_popup (0x356c4): empties slot `slot` (0-8), the calls after
// it moving down a slot.
void remove_panel_popup(World &world, uint8_t slot);

// rebuild_panel_popup_pos (0x35824): the slots from hospital 0's head calls'
// own slots.
void rebuild_panel_popup_pos(World &world);

// maintain_panel_popup (0x35914), every frame the panel draws its icons:
// empties the slots whose call is no longer a head, moves each head call
// into an empty slot left of it (sliding in again), and moves on a call
// sharing its slot with another.
void maintain_panel_popup(World &world);

// get_new_popup_head (0x18438): of `owner`'s calls of `type`, the one with
// the lowest id becomes its head (World::first_call), if any.
void get_new_popup_head(World &world, uint16_t type, uint8_t owner);

} // namespace th
