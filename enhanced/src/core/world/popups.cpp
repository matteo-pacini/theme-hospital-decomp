#include "core/world/popups.h"

namespace th
{

namespace
{

// The panel's slots (the game's 9).
constexpr uint8_t kPopupSlots = 9;

// maintain_panel_popup's first step: the slots whose call is no longer the
// player's head call of its kind are emptied.
void maintain_panel_popup_gone(World &world)
{
    PanelPopups &p = world.popups;
    for (uint8_t i = 0; i < kPopupSlots; i++) {
        if (!p.pos[i]) {
            remove_panel_popup(world, i);
            continue;
        }
        bool gone = true;
        for (size_t j = 1; j < kPopupIcons.size(); j++)
            if (p.pos[i] == world.first_call[0][j])
                gone = false;
        if (gone)
            remove_panel_popup(world, i);
    }
}

// maintain_panel_popup's second step: a head call with an empty slot left of
// it moves into it, sliding in again.
void maintain_panel_popup_close_up(World &world)
{
    PanelPopups &p = world.popups;
    for (size_t j = 1; j < kPopupIcons.size(); j++) {
        uint16_t index = world.first_call[0][j];
        if (!index)
            continue;
        World::PlayerCall &call = world.calls[index];
        if (call.slot < kPopupSlots && call.slot > 0 && !p.pos[call.slot - 1]) {
            remove_panel_popup(world, uint8_t(call.slot - 1));
            call.slot--;
            p.pos[call.slot] = index;
            call.slide_step = 1;
        }
    }
}

// maintain_panel_popup's last step: of two head calls in the same slot,
// the other moves on a slot.
void maintain_panel_popup_separate(World &world)
{
    for (size_t j = 1; j < kPopupIcons.size(); j++) {
        uint16_t index = world.first_call[0][j];
        if (!index || world.calls[index].slot >= kPopupSlots)
            continue;
        uint8_t slot = world.calls[index].slot;
        for (size_t k = 1; k < kPopupIcons.size(); k++) {
            uint16_t other = world.first_call[0][k];
            if (other && other != index && world.calls[other].slot < kPopupSlots && world.calls[other].slot == slot)
                world.calls[other].slot++;
        }
    }
}

} // namespace

void add_panel_popup(World &world, uint16_t call)
{
    rebuild_panel_popup_pos(world);
    World::PlayerCall &c = world.calls[call];
    if (c.owner != 0)
        return;
    PanelPopups &p = world.popups;
    uint8_t i = 0;
    while (i < kPopupSlots && p.pos[i] != 0)
        i++;
    if (i < kPopupSlots) {
        c.slot = i;
        p.pos[i] = call;
        c.slide_step = 1;
        p.shutter_state = 2;
    } else {
        c.slot = kPopupSlots;
    }
    p.new_faxes++;
}

void delete_panel_popup(World &world, uint16_t call)
{
    if (world.calls[call].owner != 0)
        return;
    PanelPopups &p = world.popups;
    for (uint8_t i = 0; i < kPopupSlots; i++) {
        if (p.pos[i] == call)
            remove_panel_popup(world, i);
        if (p.pos[i] == 0)
            remove_panel_popup(world, i);
    }
}

void remove_panel_popup(World &world, uint8_t slot)
{
    PanelPopups &p = world.popups;
    for (uint8_t i = uint8_t(slot + 1); i < kPopupSlots; i++) {
        // Entry 0, for an empty slot, is moved down too, as in the game.
        World::PlayerCall &call = world.calls[p.pos[i]];
        if (call.slot > 0)
            call.slot--;
        p.pos[i - 1] = p.pos[i];
    }
    p.pos[kPopupSlots - 1] = 0;
}

void rebuild_panel_popup_pos(World &world)
{
    PanelPopups &p = world.popups;
    p.pos = {};
    for (size_t j = 1; j < kPopupIcons.size(); j++) {
        uint16_t index = world.first_call[0][j];
        // A slot of 9 (no room on the panel) lands past the game's table;
        // it is dropped here.
        if (index && world.calls[index].slot < kPopupSlots)
            p.pos[world.calls[index].slot] = index;
    }
}

void maintain_panel_popup(World &world)
{
    rebuild_panel_popup_pos(world);
    maintain_panel_popup_gone(world);
    maintain_panel_popup_close_up(world);
    maintain_panel_popup_separate(world);
}

void get_new_popup_head(World &world, uint16_t type, uint8_t owner)
{
    uint16_t lowest = 0xffff;
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        if (call.type != type || call.owner != owner)
            continue;
        if (lowest > call.id) {
            world.first_call[owner][type] = uint16_t(i);
            lowest = call.id;
        }
    }
}

} // namespace th
