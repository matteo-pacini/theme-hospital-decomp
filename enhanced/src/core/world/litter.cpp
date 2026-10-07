#include "core/world/litter.h"

#include "core/data/level.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/complex_tables.h"
#include "core/world/emergency.h"
#include "core/world/icons.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"

#include <array>
#include <cstdlib>

namespace th
{

namespace
{

constexpr uint8_t kObjRadiator = 44;
constexpr uint8_t kObjPlant = 45;

// kPatientAnimStarts columns of the reactions.
constexpr uint16_t kAnimVomit = 0x4b;
constexpr uint16_t kAnimWet = 0x66;
constexpr uint16_t kAnimQuake = 0x2d;

// The game's around (src/tables.cpp): the cells beside a cell, then the
// cell itself.
constexpr std::array<int, 5> kAround = {-128, 1, 128, -1, 0};

// The game's abilities (src/tables.cpp), per patient kind: column 1 kinds
// that get bored waiting, column 4 kinds that play the idle animations.
constexpr std::array<std::array<uint8_t, 6>, 15> kAbilities = {{
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {0},
    {0},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
}};

// The game's show_anims (ShowAnim, src/tables.cpp): the idle animations
// (kPatientAnimStarts columns), the facings they play in, the illness
// needed (0 any), the impatience needed (0 none; 1 over half
// gbv.leave_max, 2 and 3 over three quarters) and the kAbilities column
// needed (0 none).
struct ShowAnim {
    uint16_t anim;
    std::array<uint8_t, 4> facings;
    uint8_t any_facing;
    uint8_t illness;
    uint16_t condition;
    uint8_t ability;
};
constexpr std::array<ShowAnim, 5> kShowAnims = {{
    {98, {1, 1, 1, 1}, 0, 0, 2, 4},
    {99, {1, 1, 1, 1}, 0, 0, 3, 4},
    {100, {1, 1, 1, 1}, 0, 0, 1, 4},
    {100, {1, 1, 1, 1}, 0, 11, 0, 4},
    {100, {1, 1, 1, 1}, 0, 13, 0, 4},
}};

// The game's feely (src/tables.cpp), its one live entry: plants within 5
// cells on each axis change how people feel.
constexpr int kFeelRange = 5;

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// which_room (0x2384c in src/build.cpp): the room covering `tile`, 0 for
// none.
uint16_t which_room(const World &world, uint16_t tile)
{
    int x = tile % 128, y = tile / 128;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.type == kRoomFree)
            continue;
        int x0 = room.pos % 128, y0 = room.pos / 128;
        if (x0 > x || x0 + room.width - 1 < x || y0 > y || y0 + room.height - 1 < y)
            continue;
        return r;
    }
    return 0;
}

bool indoors(const World &world, const Thing &t)
{
    return !(world.map[cell_of(t)].owner_flags & kCellOutdoor);
}

// Only player 0's objects are listed (World::first_of_kind).
uint16_t first_of_kind(const World &world, uint8_t owner, uint8_t kind)
{
    return owner == 0 ? world.first_of_kind[kind] : 0;
}

// The game raises and lowers nausea as a signed value and clamps it at 0.
void add_nausea(Thing &p, int amount)
{
    p.nausea = uint16_t(p.nausea + amount);
    if (int16_t(p.nausea) < 0)
        p.nausea = 0;
}

} // namespace

void check_warmth(const World &world, Thing &p, int heating_level)
{
    uint16_t warmth = 0;
    auto heat = uint16_t(heating_level * 10);
    if (heat && !which_room(world, cell_of(p))) {
        for (uint16_t id = first_of_kind(world, p.owner, kObjRadiator); id; id = world.things[id].type_obj_next) {
            const Thing &radiator = world.things[id];
            if (radiator.room_id)
                continue;
            auto dx = uint16_t(std::abs(p.x.tile - radiator.x.tile));
            auto dy = uint16_t(std::abs(p.y.tile - radiator.y.tile));
            if (dx > 8 || dy > 8)
                continue;
            if (auto dist = uint16_t(dx + dy))
                warmth = uint16_t(warmth + heat / dist);
        }
    }
    if (warmth > int16_t(p.warmth))
        p.warmth++;
    else if (warmth < int16_t(p.warmth))
        p.warmth--;
    if (p.type == kThingPatient && int16_t(p.warmth) > 60)
        p.refreshment = int16_t(p.refreshment - 2);
    if (int16_t(p.warmth) < 0)
        p.warmth = 0;
    else if (int16_t(p.warmth) > 100)
        p.warmth = 100;
}

void staff_check_around(const World &world, Thing &s)
{
    for (uint16_t id = first_of_kind(world, s.owner, kObjPlant); id; id = world.things[id].type_obj_next) {
        const Thing &plant = world.things[id];
        if (plant.room_id || std::abs(s.x.tile - plant.x.tile) > kFeelRange ||
            std::abs(s.y.tile - plant.y.tile) > kFeelRange)
            continue;
        s.happiness = uint16_t(plant.plant_health < 0 ? s.happiness - 1 : s.happiness + 1);
    }
    uint16_t pos = cell_of(s);
    if (which_room(world, pos))
        return;
    for (int offset : kAround) {
        int cell = pos + offset;
        // The game reads past the map's ends; there is no litter there.
        if (cell < 0 || cell >= kMapCells)
            continue;
        uint8_t kind = world.map[size_t(cell)].litter_shadow & 0xf;
        if (kind == kLitterVomit)
            s.happiness--;
        else if (!(kind == 0 || kind == 7 || kind == 8))
            s.happiness = uint16_t(s.happiness - 2);
    }
}

namespace
{

// check_feelgood (0x71558).
void check_feelgood(const World &world, Thing &p)
{
    int8_t mood = 0;
    for (uint16_t id = first_of_kind(world, p.owner, kObjPlant); id; id = world.things[id].type_obj_next) {
        const Thing &plant = world.things[id];
        if (plant.room_id || std::abs(p.x.tile - plant.x.tile) > kFeelRange ||
            std::abs(p.y.tile - plant.y.tile) > kFeelRange || plant.subtype != kObjPlant)
            continue;
        mood = int8_t(plant.plant_health < 0 ? mood - 1 : mood + 1);
    }
    if (mood < 0) {
        add_nausea(p, 1);
        p.happiness--;
    } else if (mood > 0) {
        add_nausea(p, -1);
        p.happiness++;
    }
    if (p.state == kPsEntertainsThemself && kAbilities[p.subtype % 15][1] && p.complex_action != 0x46)
        p.happiness--;
}

// check_around (0x717f0).
void check_around(const World &world, Thing &p)
{
    uint16_t pos = cell_of(p);
    if (which_room(world, pos))
        return;
    for (int offset : kAround) {
        int cell = pos + offset;
        // The game reads past the map's ends; there is no litter there.
        if (cell < 0 || cell >= kMapCells)
            continue;
        uint8_t kind = world.map[size_t(cell)].litter_shadow & 0xf;
        if (kind == kLitterVomit || kind == kLitterPuddle) {
            add_nausea(p, 1);
            p.happiness = uint16_t(p.happiness - 2);
            p.litter_urge = uint16_t(p.litter_urge + 2);
        } else if (!(kind == 0 || kind == 7 || kind == 8)) {
            add_nausea(p, 2);
            p.happiness--;
            p.litter_urge++;
        }
    }
}

// spread_epidemic (0xa0040 in src/epidemic.cpp): passes the carrier's
// contagious illness to one undiagnosed, uncontagious patient on or next to
// its cell, splitting the carrier's contagion with them (and applying
// effect_illness to the carrier, as the game does). othervar.contagious_count
// is not kept: only the unported advisor epidemic advice reads it.
void spread_epidemic(World &world, const Balance &settings, uint16_t carrier_id)
{
    trace_event("epidemic %u", unsigned(carrier_id));

    Thing &carrier = world.things[carrier_id];
    // The game keeps this draw in othervar.rand_pct.
    uint32_t pct = old_newrand(world.seed) % 100;
    if (pct >= uint32_t(settings.gbv.contagious_spread_factor + carrier.warmth / 2))
        return;
    uint8_t illness = carrier.illness;
    uint16_t pos = cell_of(carrier);
    for (int offset : kAround) {
        int cell = pos + offset;
        if (cell < 0 || cell >= kMapCells)
            continue;
        for (int16_t id = world.map[size_t(cell)].first; id > 0; id = world.things[size_t(id)].cell_next) {
            Thing &victim = world.things[size_t(id)];
            if (victim.type != kThingPatient || uint16_t(id) == carrier_id)
                continue;
            if (victim.illness == victim.known_illness)
                continue;
            uint8_t other = victim.illness;
            // In this order: the draw is made only for the patients that
            // pass the first two tests.
            if (kNotContagious[victim.illness % kNotContagious.size()] || int8_t(victim.contagion) != 0 ||
                old_newrand(world.seed) & 1)
                continue;
            if (other != illness) {
                victim.illness = illness;
                effect_illness(world.seed, carrier, illness);
            }
            victim.contagion = uint8_t(int8_t(carrier.contagion) / 2);
            carrier.contagion = uint8_t(carrier.contagion - victim.contagion);
            if (carrier.person_flags & kPfContagious)
                victim.person_flags |= kPfContagious;
            return;
        }
    }
}

// setup_showanim (0x71dfc).
void setup_showanim(Thing &p, const AnimData &anims, uint16_t column)
{
    p.substate = p.state;
    p.saved_sprite = p.sprite;
    p.saved_flip = p.flip;
    p.state = kPsShowanim;
    uint16_t anim = kPatientAnimStarts[p.subtype % kPatientAnimStarts.size()][column];
    AnimStart start = anim < anims.starts.size() ? anims.starts[anim] : AnimStart{};
    p.sprite = uint16_t(start.frame);
    p.flip = uint8_t(start.flip);
}

} // namespace

void place_litter(World &world, uint16_t tile, uint8_t owner, uint8_t kind)
{
    trace_event("litter %u type=%u owner=%u", unsigned(tile), unsigned(kind), unsigned(owner));
    if (tile >= kMapCells)
        return;
    MapCell &m = world.map[tile];
    if (m.owner_flags != 1 << owner)
        return;
    uint8_t cur = m.litter_shadow & 0xf;
    if (cur == 0 && owner == 0)
        world.litter_count++;
    if (cur == 0 || ((kind == kLitterVomit || kind == 4) && !(cur == 7 || cur == 8)))
        m.litter_shadow = uint8_t((m.litter_shadow & 0xf0) | kind);
}

void remove_litter(World &world, uint16_t tile, uint8_t owner)
{
    trace_event("unlitter %u owner=%u", unsigned(tile), unsigned(owner));
    if (tile >= kMapCells)
        return;
    MapCell &m = world.map[tile];
    uint8_t cur = m.litter_shadow & 0xf;
    if (m.owner_flags == 1 << owner && !(cur == 0 || cur == 7 || cur == 8)) {
        m.litter_shadow &= 0xf0;
        if (owner == 0)
            world.litter_count--;
    }
}

void add_litter_urge(Thing &patient, uint16_t amount)
{
    patient.litter_urge = uint16_t(patient.litter_urge + amount);
}

// The contagious or vaccinated status icon of a patient whose contagion is
// set.
void person_contagion_icon(const AnimData &anims, Thing &p)
{
    if (!(p.person_flags & kPfContagious))
        return;
    if (int8_t(p.contagion) > 0)
        set_icon_value(anims, p, kIconContagious);
    else if (int8_t(p.contagion) < 0)
        set_icon_value(anims, p, kIconVaccinated);
}

// modify_person's warmth, happiness and needs: their effect on happiness,
// impatience and the too hot, too cold, unhappy, toilet and thirsty icons.
void person_needs(const AnimData &anims, Thing &p, const Balance::Gbv &gbv)
{
    if (int16_t(p.warmth) > 80) {
        add_nausea(p, 1);
        p.happiness--;
        set_icon_value(anims, p, kIconTooHot);
    } else if (int16_t(p.warmth) < 20) {
        p.happiness--;
        set_icon_value(anims, p, kIconTooCold);
    } else {
        p.happiness++;
        reset_icon_value(p, kIconTooHot);
        reset_icon_value(p, kIconTooCold);
    }
    if (int16_t(p.happiness) < gbv.unhappy) {
        set_icon_value(anims, p, kIconUnhappy);
        p.impatience++;
    } else {
        reset_icon_value(p, kIconUnhappy);
    }
    if (p.bladder > gbv.bowel_full) {
        set_icon_value(anims, p, kIconToilet);
        p.impatience++;
    }
    if (p.refreshment < 30) {
        set_icon_value(anims, p, kIconThirsty);
        p.impatience++;
    }
}

void modify_person(World &world, const AnimData &anims, const Balance &settings, int heating_level, uint16_t id,
                   uint32_t game_turn)
{
    Thing &p = world.things[id];
    const Balance::Gbv &gbv = settings.gbv;
    auto now = uint16_t(game_turn);
    if (modify_person_emergency(world, anims, id, game_turn))
        return;
    person_contagion_icon(anims, p);
    if (!(now > p.modify_time || p.modify_time - now > 0x7fff))
        return;
    p.modify_time = uint16_t(game_turn + gbv.modify_freq);
    if (!indoors(world, p))
        return;
    check_warmth(world, p, heating_level);
    check_feelgood(world, p);
    check_around(world, p);
    if (int8_t(p.contagion) > 1)
        spread_epidemic(world, settings, id);
    p.impatience++;
    p.waiting_time++;
    p.refreshment--;
    if (old_newrand(world.seed) % 3 == 0)
        p.bladder++;
    person_needs(anims, p, gbv);
    if (int16_t(p.happiness) < 0)
        p.happiness = 0;
    else if (int16_t(p.happiness) > 100)
        p.happiness = 100;
    if (p.refreshment < 0)
        p.refreshment = 0;
    change_health(world, anims, id, -1, game_turn);
}

namespace
{

// do_reaction: whether person `p` may play the idle animation `s`.
bool do_reaction_anim_suits(const Thing &p, const Balance::Gbv &gbv, const ShowAnim &s)
{
    if (s.illness != p.illness && s.illness != 0)
        return false;
    if (s.ability != 0 && !kAbilities[p.subtype % 15][s.ability])
        return false;
    // The game reads facings[4], which is any_facing, for a patient
    // stuck (facing 4).
    if (!s.any_facing && !(p.facing < 4 ? s.facings[p.facing] : s.any_facing))
        return false;
    switch (s.condition) {
    case 0:
        return true;
    case 1:
        return p.impatience > gbv.leave_max / 2;
    case 2:
    case 3:
        return p.impatience > gbv.leave_max * 3 / 4;
    default:
        return false;
    }
}

// do_reaction's idle animation: picks one of the animations that suit `p` and
// starts it. False when none does.
bool do_reaction_show_anim(World &world, const AnimData &anims, const Balance::Gbv &gbv, Thing &p,
                           std::source_location caller)
{
    std::array<uint16_t, kShowAnims.size()> list{};
    uint32_t count = 0;
    for (const ShowAnim &s : kShowAnims) {
        if (do_reaction_anim_suits(p, gbv, s))
            list[count++] = s.anim;
    }
    if (count == 0)
        return false;
    setup_showanim(p, anims, list[old_newrand(world.seed, caller) % count]);
    return true;
}

} // namespace

bool do_reaction(World &world, const AnimData &anims, const Balance &settings, uint16_t id, uint32_t game_turn)
{
    Thing &p = world.things[id];
    const Balance::Gbv &gbv = settings.gbv;
    uint16_t pos = cell_of(p);
    if (!indoors(world, p) || which_room(world, pos))
        return false;
    if (p.person_flags & kPfQuakeShaken) {
        p.person_flags &= ~kPfQuakeShaken;
        setup_showanim(p, anims, kAnimQuake);
        return true;
    }
    if (int16_t(p.nausea) >= gbv.vomit_limit && old_newrand(world.seed) % 5 == 0) {
        setup_showanim(p, anims, kAnimVomit);
        p.nausea = 5;
        p.person_flags |= kPfVomits;
        change_health(world, anims, id, -10, game_turn);
        return true;
    }
    if (p.bladder >= gbv.bowel_overflows && old_newrand(world.seed) % 5 == 0) {
        trace_event("wets %u bladder=%d", unsigned(id), int(p.bladder));
        reset_icon_value(p, kIconToilet);
        setup_showanim(p, anims, kAnimWet);
        p.bladder = p.illness == kIllTheSquits ? int16_t(gbv.bowel_full / 2) : int16_t(0);
        p.person_flags |= kPfWets;
        change_health(world, anims, id, -10, game_turn);
        return true;
    }
    if (old_newrand(world.seed) % 200 == 0 &&
        do_reaction_show_anim(world, anims, gbv, p, std::source_location::current()))
        return true;
    uint32_t chance = gbv.litter_random > 0 ? old_newrand(world.seed) % gbv.litter_random : 1;
    if ((p.litter_urge > gbv.litter_drop || chance == 0) && old_newrand(world.seed) % 10 == 0) {
        place_litter(world, pos, p.owner, uint8_t(old_newrand(world.seed) % 3 + 1));
        p.litter_urge = 0;
    }
    return false;
}

bool person_needs_turn(World &world, const AnimData &anims, const Balance &settings, int heating_level, uint16_t id,
                       uint32_t game_turn)
{
    modify_person(world, anims, settings, heating_level, id, game_turn);
    return do_reaction(world, anims, settings, id, game_turn);
}

void person_showanim_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &p = world.things[id];
    if (p.anim_speed == 0 || game_turn % p.anim_speed != 0)
        return;
    uint16_t next = anims.frames[p.sprite].next;
    if (!(anims.frames[next].flags & 1)) {
        p.sprite = next;
        return;
    }
    p.state = p.substate;
    p.sprite = p.saved_sprite;
    p.flip = p.saved_flip;
    p.saved_sprite = 0;
    p.saved_flip = 0;
    p.substate = 0;
    if (p.person_flags & kPfVomits) {
        place_litter(world, cell_of(p), p.owner, kLitterVomit);
        p.person_flags &= ~kPfVomits;
    } else if (p.person_flags & kPfWets) {
        place_litter(world, cell_of(p), p.owner, kLitterPuddle);
        p.person_flags &= ~kPfWets;
    }
}

} // namespace th
