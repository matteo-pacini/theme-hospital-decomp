#include "common.h"

#include "core/data/gamedata.h"
#include "core/render/frame.h"
#include "core/render/surface.h"
#include "core/world/icons.h"
#include "core/world/world.h"

#include <cstdint>
#include <memory>
#include <string>

// The icon is drawn above the thing's frame: the returned box reaches higher
// with an icon than without.
void status_icon_drawn(const th::GameData &gd, uint32_t icon)
{
    th::Surface plain;
    plain.resize(300, 300);
    plain.fill(0);
    th::Rect base = th::draw_frame(plain, {150, 150}, icon, false, gd, {});
    CHECK(base.w > 0);
    th::Surface lifted;
    lifted.resize(300, 300);
    lifted.fill(0);
    th::FrameOptions opts;
    opts.icon = icon;
    opts.icon_over_frame = true;
    th::Rect with = th::draw_frame(lifted, {150, 150}, icon, false, gd, opts);
    CHECK(with.y < base.y);
}

// The status-icon system (core/world/icons.cpp, src/icons.cpp) and the icon
// the frame drawing hangs over a thing (frame.cpp's draw_special_id_frame).
void test_status_icons(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    const th::AnimData &anims = gd.anims();

    // A machine's repair icon: permanent (mode > 0), its start[] frame drawn.
    th::Thing machine;
    machine.type = th::kThingObject;
    th::set_icon_value(anims, machine, th::kIconNeedsRepair);
    CHECK(machine.icon_id == th::kIconNeedsRepair);
    CHECK(machine.icon_timer == -1);
    CHECK(machine.icon_frame == anims.starts[th::kIconTable[th::kIconNeedsRepair].anim].frame);

    // A more important icon (lower priority) replaces it; a lesser one does
    // not.
    th::set_icon_value(anims, machine, th::kIconTooHot);
    CHECK(machine.icon_id != th::kIconTooHot);
    th::reset_icon_value(machine, th::kIconNeedsRepair);
    CHECK(machine.icon_id == 0 && machine.icon_frame == 0 && machine.icon_timer == 0);

    // A pending icon (mode < 0) is stored negative and shown by peep_icon,
    // which also queues its sound.
    th::Thing patient;
    patient.type = th::kThingPatient;
    patient.health = 1000;
    th::set_icon_value(anims, patient, th::kIconTooHot);
    CHECK(patient.icon_id == -th::kIconTooHot);
    auto world = std::make_unique<th::World>();
    th::peep_icon(*world, patient);
    CHECK(patient.icon_id == th::kIconTooHot);
    CHECK(patient.icon_timer == th::kIconTable[th::kIconTooHot].duration);
    CHECK(world->building.sounds.size() == 1);

    // update_icon_counter counts the time down and advances the icon frame.
    uint16_t frame0 = patient.icon_frame;
    th::update_icon_counter(anims, patient);
    CHECK(patient.icon_timer == th::kIconTable[th::kIconTooHot].duration - 1);
    CHECK(patient.icon_frame == anims.frames[frame0].next);

    // A dead patient shows no icon.
    patient.health = 0;
    th::set_icon_value(anims, patient, th::kIconNeedsRepair);
    CHECK(patient.icon_id == th::kIconTooHot || patient.icon_id == 0);

    status_icon_drawn(gd, anims.starts[th::kIconTable[th::kIconNeedsRepair].anim].frame);
}
