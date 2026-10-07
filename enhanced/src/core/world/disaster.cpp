#include "core/world/disaster.h"

#include "core/data/blocks.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/complex_tables.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/nav.h"
#include "core/world/queue.h"
#include "core/world/reaper.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <array>

namespace th
{

namespace
{

// The first quarter turn of the game's lbSinTable (src/lib/bf1/gtrig.cpp):
// sin(i * 2pi / 2048) * 65536, truncated, for i 0-512.
constexpr std::array<int32_t, 513> kSinQuarter = {
    0,     201,   402,   603,   804,   1005,  1206,  1407,  1608,  1809,  2010,  2211,  2412,  2613,  2814,  3014,
    3215,  3416,  3617,  3818,  4018,  4219,  4420,  4620,  4821,  5021,  5222,  5422,  5622,  5823,  6023,  6223,
    6423,  6623,  6823,  7023,  7223,  7423,  7623,  7822,  8022,  8221,  8421,  8620,  8819,  9019,  9218,  9417,
    9616,  9814,  10013, 10212, 10410, 10609, 10807, 11006, 11204, 11402, 11600, 11797, 11995, 12193, 12390, 12588,
    12785, 12982, 13179, 13376, 13573, 13769, 13966, 14162, 14359, 14555, 14751, 14946, 15142, 15338, 15533, 15728,
    15923, 16118, 16313, 16508, 16702, 16897, 17091, 17285, 17479, 17672, 17866, 18059, 18253, 18446, 18638, 18831,
    19024, 19216, 19408, 19600, 19792, 19983, 20175, 20366, 20557, 20748, 20938, 21129, 21319, 21509, 21699, 21889,
    22078, 22267, 22456, 22645, 22833, 23022, 23210, 23398, 23586, 23773, 23960, 24147, 24334, 24521, 24707, 24893,
    25079, 25265, 25450, 25635, 25820, 26005, 26189, 26373, 26557, 26741, 26925, 27108, 27291, 27473, 27656, 27838,
    28020, 28201, 28383, 28564, 28745, 28925, 29105, 29285, 29465, 29645, 29824, 30003, 30181, 30360, 30538, 30715,
    30893, 31070, 31247, 31424, 31600, 31776, 31952, 32127, 32302, 32477, 32651, 32826, 32999, 33173, 33346, 33519,
    33692, 33864, 34036, 34208, 34379, 34550, 34721, 34891, 35061, 35231, 35400, 35569, 35738, 35906, 36074, 36242,
    36409, 36576, 36743, 36909, 37075, 37241, 37406, 37571, 37736, 37900, 38064, 38227, 38390, 38553, 38716, 38878,
    39039, 39201, 39362, 39522, 39682, 39842, 40002, 40161, 40319, 40478, 40636, 40793, 40950, 41107, 41263, 41419,
    41575, 41730, 41885, 42040, 42194, 42347, 42501, 42653, 42806, 42958, 43110, 43261, 43412, 43562, 43712, 43862,
    44011, 44160, 44308, 44456, 44603, 44750, 44897, 45043, 45189, 45335, 45480, 45624, 45768, 45912, 46055, 46198,
    46340, 46482, 46624, 46765, 46906, 47046, 47186, 47325, 47464, 47602, 47740, 47878, 48015, 48151, 48288, 48423,
    48558, 48693, 48828, 48961, 49095, 49228, 49360, 49492, 49624, 49755, 49886, 50016, 50146, 50275, 50403, 50532,
    50660, 50787, 50914, 51040, 51166, 51291, 51416, 51541, 51665, 51788, 51911, 52033, 52155, 52277, 52398, 52518,
    52639, 52758, 52877, 52996, 53114, 53231, 53348, 53465, 53581, 53696, 53811, 53926, 54040, 54153, 54266, 54379,
    54491, 54602, 54713, 54823, 54933, 55043, 55152, 55260, 55368, 55475, 55582, 55688, 55794, 55899, 56004, 56108,
    56212, 56315, 56417, 56519, 56621, 56722, 56822, 56922, 57022, 57120, 57219, 57316, 57414, 57510, 57606, 57702,
    57797, 57892, 57986, 58079, 58172, 58264, 58356, 58447, 58538, 58628, 58718, 58807, 58895, 58983, 59070, 59157,
    59243, 59329, 59414, 59499, 59583, 59666, 59749, 59831, 59913, 59994, 60075, 60155, 60235, 60313, 60392, 60470,
    60547, 60624, 60700, 60775, 60850, 60924, 60998, 61071, 61144, 61216, 61288, 61359, 61429, 61499, 61568, 61637,
    61705, 61772, 61839, 61905, 61971, 62036, 62100, 62164, 62228, 62291, 62353, 62414, 62475, 62536, 62596, 62655,
    62714, 62772, 62829, 62886, 62942, 62998, 63053, 63108, 63162, 63215, 63268, 63320, 63371, 63422, 63473, 63522,
    63571, 63620, 63668, 63715, 63762, 63808, 63854, 63899, 63943, 63987, 64030, 64073, 64115, 64156, 64197, 64237,
    64276, 64315, 64353, 64391, 64428, 64465, 64501, 64536, 64571, 64605, 64638, 64671, 64703, 64735, 64766, 64796,
    64826, 64855, 64884, 64912, 64939, 64966, 64992, 65018, 65043, 65067, 65091, 65114, 65136, 65158, 65179, 65200,
    65220, 65239, 65258, 65276, 65294, 65311, 65327, 65343, 65358, 65372, 65386, 65400, 65412, 65424, 65436, 65446,
    65457, 65466, 65475, 65483, 65491, 65498, 65505, 65511, 65516, 65520, 65524, 65528, 65531, 65533, 65534, 65535,
    65536,
};

// lbSinTable[i] for i 0-2559 (the table runs a quarter turn past a turn,
// so that cosines read at i + 0x200).
int32_t sin_table(int32_t i)
{
    i %= 2048;
    if (i <= 512)
        return kSinQuarter[size_t(i)];
    if (i <= 1024)
        return kSinQuarter[size_t(1024 - i)];
    if (i <= 1536)
        return -kSinQuarter[size_t(i - 1024)];
    return -kSinQuarter[size_t(2048 - i)];
}

// The game's objects[].explosion_offset, the same for every kind: from the
// machine's cell to where its explosion starts, one cell up and left.
constexpr uint16_t kExplosionOffset = 65407;

// The game's objects[].debris (its low byte, debris_lo): the blueprint
// marker (plus the facing) of the wreck an exploded machine leaves, by
// object kind; 0 for the kinds that are not machines.
constexpr std::array<uint8_t, kObjectKinds> kObjectDebris = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 0,  0, 0, 77, 81, 0,  0, 0, 0, 0, 0, 0, 97, 0, 93, 87, 99, 91, 0, 0, 95, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  85, 0, 0, 0,  73, 79, 0, 0, 0, 0, 0, 0, 89, 0, 0,  0,  0,  0,  0, 0, 0,
};

// RoomInfo.type and state of a room whose machine blew up (ROOM_DESTROYED,
// RS_CLOSED).
constexpr uint8_t kRoomDestroyed = 6;
constexpr int8_t kRoomClosed = 6;

// The blueprint markers blast_walls_and_floor leaves on a damaged wall:
// north and west, on a wall (block flag 1) or a window (block flag 2).
constexpr uint8_t kMarkerWallNorth = 0x65;
constexpr uint8_t kMarkerWallWest = 0x66;
constexpr uint8_t kMarkerWindowNorth = 0x67;
constexpr uint8_t kMarkerWindowWest = 0x68;

// SFX_* (include/enums.h).
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxSelectX = 0x43;
constexpr uint8_t kSfxMachWarn = 0xd0;
constexpr uint8_t kSfxBomb = 0xd5;

// The first announcement (play_ambient's Ambients index) of a machine
// worn out, by object kind from it; kinds from 0x3e have none.
constexpr uint8_t kAnnounceMachine = 0x4c;
constexpr uint8_t kAnnouncedKinds = 0x3e;

// The machines' sounds and announcements for the interface, up to
// kMachineSoundsKept of each (World::machine_sounds).
constexpr size_t kMachineSoundsKept = 8;

void machine_sound(std::vector<uint8_t> &sounds, uint8_t sound)
{
    if (sounds.size() < kMachineSoundsKept)
        sounds.push_back(sound);
}

// create_bang (0x752e8): a piece of debris with animation `anim` on `tile`,
// moving by (dx, dy, dz), in the first free thing from `id` on (the last
// 50 left free); its thing id, 0 if none was free.
uint16_t create_bang(World &world, const AnimData &anims, uint16_t id, uint16_t tile, std::array<int16_t, 3> v,
                     uint16_t anim)
{
    uint16_t room = room_at(world, tile);
    for (uint16_t b = id; b < kThingCount - 50; b++) {
        Thing &bang = world.things[b];
        if (bang.type != kThingFree)
            continue;
        bang.type = kThingDebris;
        const AnimStart start = anim < anims.starts.size() ? anims.starts[anim] : AnimStart{};
        bang.sprite = uint16_t(start.frame);
        bang.flip = uint8_t(start.flip);
        bang.anim_speed = uint8_t((old_newrand(world.seed, "create_bang") & 1) + 1);
        bang.x = {0x80, uint8_t(tile % kMapSize)};
        bang.y = {0x80, uint8_t(tile / kMapSize)};
        insert_node(world, b, bang.x, bang.y);
        bang.altitude = 0;
        bang.vel_x = v[0];
        bang.vel_y = v[1];
        bang.vel_z = v[2];
        if (room)
            bang.bang_room = room;
        return b;
    }
    return 0;
}

// blast_things (0x758b8): every object on cell `tile` taken off
// (remove_object), the wreck of a machine left in its place. remove_object
// (remove_object_from_map) interrupts the person using it, as in the game.
void blast_things(World &world, uint16_t tile)
{
    for (;;) {
        int16_t id = world.map[tile].first;
        while (id > 0 && world.things[size_t(id)].type != kThingObject)
            id = world.things[size_t(id)].cell_next;
        if (id <= 0)
            return;
        const Thing &obj = world.things[size_t(id)];
        uint8_t debris = kObjectDebris[obj.subtype % kObjectDebris.size()];
        uint8_t facing = obj.facing;
        if (!remove_object(world, uint16_t(id), obj.owner))
            return;
        if (debris)
            place_small_thing(world, tile, debris + facing, false);
    }
}

// blast_walls_and_floor's damage to one wall of `tile` (side 0 north, 1
// west): a marker on a wall or a window that may be damaged.
bool blast_wall(World &world, uint16_t tile, int side)
{
    const MapCell &m = world.map[tile];
    uint8_t flags = block_flags(side == 0 ? m.wall_n : m.wall_w);
    if (flags & 2) {
        if (!(flags & 0x10))
            place_small_thing(world, tile, side == 0 ? kMarkerWindowNorth : kMarkerWindowWest, true);
        return true;
    }
    if (flags & 1) {
        place_small_thing(world, tile, side == 0 ? kMarkerWallNorth : kMarkerWallWest, true);
        return true;
    }
    return false;
}

// blast_walls_and_floor (0x757a0): `tile`'s north wall damaged (a marker)
// with a 70% chance; failing that, or with no wall there, its west wall
// likewise. The floor is not touched.
void blast_walls_and_floor(World &world, uint16_t tile)
{
    if (old_newrand(world.seed, "blast_walls_and_floor") % 1000 > 300 && blast_wall(world, tile, 0))
        return;
    if (old_newrand(world.seed, "blast_walls_and_floor") % 1000 > 300)
        blast_wall(world, tile, 1);
}

// blast_effect (0x755c8): the cells of room `room_id` within 4-7 cells (a
// draw) of `tile` wrecked: their objects taken off (blast_things) and,
// the nearer the likelier (a draw each), their walls damaged.
void blast_effect(World &world, uint16_t tile, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    int radius = int(old_newrand(world.seed, "blast_effect") & 3) + 4;
    int cx = tile % kMapSize, cy = tile / kMapSize;
    trace_event("blast tile=%u radius=%d", unsigned(tile), radius);
    int rx = room.pos % kMapSize, ry = room.pos / kMapSize;
    int r2 = radius * radius;
    for (int x = cx - radius; x <= cx + radius; x++) {
        for (int y = cy - radius; y <= cy + radius; y++) {
            if (x < rx || x >= rx + room.width || y < ry || y >= ry + room.height)
                continue;
            auto cell = uint16_t(x + (y << 7));
            int d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            blast_things(world, cell);
            if (int(old_newrand(world.seed, "blast_effect") & 0x3ff) > (d2 << 10) / r2)
                blast_walls_and_floor(world, cell);
        }
    }
}

// explosion_kills_dudes_in_room (0x7599c): the staff in room `room_id`
// sacked (mode 2: gone at once), up to 50 tries, then its patients die
// (person_dies).
void explosion_kills_dudes_in_room(World &world, const AnimData &anims, uint16_t room_id, uint32_t game_turn)
{
    bool again = true;
    for (int tries = 50; again && --tries >= 0;) {
        again = false;
        for (uint16_t p = world.rooms[room_id].first_person; p; p = world.things[p].room_next) {
            if (world.things[p].type == kThingStaff) {
                sack_staff(world, anims, p, 2, game_turn);
                again = true;
                break;
            }
        }
    }
    for (uint16_t p = world.rooms[room_id].first_person; p; p = world.things[p].room_next) {
        if (world.things[p].type == kThingPatient)
            person_dies(world, anims, p, game_turn);
    }
}

// restart_entire_queue (0x75aac): every patient queueing for room
// `room_id` out of its queue and sent on again (start_person_going), to the
// GP's office if its hospital cannot give its treatment.
void restart_entire_queue(World &world, const AnimData &anims, uint16_t room_id, uint32_t game_turn)
{
    while (uint16_t id = world.rooms[room_id].queue_head) {
        Thing &patient = world.things[id];
        if (world.cure_available[patient.treatment % world.cure_available.size()] != 1)
            patient.treatment = kIllGeneralPractice;
        leave_queue(world, id);
        if (world.person_complexes) {
            interrupt_person(world, anims, *world.person_complexes, id, game_turn);
            start_person_going(world, anims, *world.person_complexes, id, game_turn);
        }
    }
}

// modify_machine's smoke, from 75% of its strength: the machine's
// announcement (the player's), then a puff shown at random (a draw from
// world.seed) or, from 85% with one shown, the warning sound and, from 95%,
// all three, else one more beside it (another draw); its smoke drawn from
// the start (overlay_frame).
void modify_machine_smoke(World &world, const AnimData &anims, uint16_t id)
{
    Thing &machine = world.things[id];
    if (machine.owner == 0 && machine.subtype < kAnnouncedKinds)
        machine_sound(world.machine_announcements, uint8_t(machine.subtype + kAnnounceMachine));
    int count = 0;
    for (uint8_t puff : machine.smoke)
        count += puff == 1;
    uint16_t overlay = kObjectUse[machine.subtype % kObjectUse.size()].overlay_anim;
    auto overlay_start = uint16_t(overlay < anims.starts.size() ? anims.starts[overlay].frame : 0);
    if (machine.strength * 85 / 100 <= machine.times_used && count != 0) {
        trace_event("machine_warning %u", unsigned(id));
        machine_sound(world.machine_sounds, kSfxMachWarn);
        if (machine.strength * 95 / 100 <= machine.times_used) {
            machine.smoke = {1, 1, 1};
            machine.overlay_frame = overlay_start;
        } else if (count == 1) {
            size_t j = 0;
            while (j < 3 && machine.smoke[j] != 1)
                j++;
            j = (j + 1 + (old_newrand(world.seed, "modify_machine") & 1)) % 3;
            machine.smoke[j] = 1;
            machine.overlay_frame = overlay_start;
        }
    } else {
        machine.smoke[old_newrand(world.seed, "modify_machine") % 3] = 1;
        machine.overlay_frame = overlay_start;
    }
}

} // namespace

bool modify_machine(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &machine = world.things[id];
    trace_event("modify_machine %u used=%d strength=%d smoke=%u,%u,%u", unsigned(id), int(machine.times_used),
                int(machine.strength), unsigned(machine.smoke[0]), unsigned(machine.smoke[1]),
                unsigned(machine.smoke[2]));
    if (machine.strength * 75 / 100 <= machine.times_used)
        modify_machine_smoke(world, anims, id);
    trace_event("machine_smoke %u smoke=%u,%u,%u", unsigned(id), unsigned(machine.smoke[0]), unsigned(machine.smoke[1]),
                unsigned(machine.smoke[2]));
    if (machine.times_used >= machine.strength) {
        int32_t i = machine.strength * 3 / 2 - machine.times_used + 1;
        if (i <= 1 || old_newrand(world.seed) % uint32_t(i) == 0) {
            machine_disaster(world, anims, id, game_turn);
            return true;
        }
    }
    return false;
}

void machine_disaster(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    const Thing &machine = world.things[id];
    trace_event("machine_disaster %u kind=%u room=%u", unsigned(id), unsigned(machine.subtype),
                unsigned(machine.room_id));
    uint16_t room_id = machine.room_id;
    auto tile = uint16_t(uint16_t(machine.x.tile + (machine.y.tile << 7)) + kExplosionOffset);
    create_explosion(world, anims, tile, kAnimMachineDebris);
    blast_effect(world, tile, room_id);
    explosion_kills_dudes_in_room(world, anims, room_id, game_turn);
    remove_expertise_room(world, room_id);
    room_deleted_update_listhead(world, room_id);
    RoomInfo &room = world.rooms[room_id];
    room.type = kRoomDestroyed;
    room.state = kRoomClosed;
    world.mishaps++;
    reputation_affected(world, 0, kRepMachineExploded);
    world.machine_exploded = true;
    trace_event("room_destroyed %u mishaps=%u", unsigned(room_id), unsigned(world.mishaps));
    restart_entire_queue(world, anims, room_id, game_turn);
}

void create_explosion(World &world, const AnimData &anims, uint16_t tile, uint16_t anim)
{
    trace_event("explosion tile=%u anim=%u", unsigned(tile), unsigned(anim));
    uint16_t id = 1;
    for (int32_t elevation = 0; elevation < 0x200; elevation += 0x55) {
        int32_t cos_elevation = sin_table(elevation + 0x200);
        for (int32_t heading = 0; heading < 0x800; heading += 0x800 / ((cos_elevation << 0xb >> 0x10) / 0x5f + 1)) {
            auto speed = int32_t(old_newrand(world.seed) % 50 + 100);
            int32_t horiz_speed = speed * cos_elevation >> 0x10;
            auto dx = int16_t(horiz_speed * sin_table(heading + 0x200) >> 0x10);
            auto dy = int16_t(horiz_speed * sin_table(heading) >> 0x10);
            auto dz = int16_t(speed * sin_table(elevation) >> 0xa);
            id = create_bang(world, anims, id, tile, {dx, dy, dz}, anim);
        }
    }
    machine_sound(world.machine_sounds, kSfxBomb);
}

void process_bang(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &bang = world.things[id];
    auto raw_x = uint16_t((bang.x.tile << 8 | bang.x.frac) + bang.vel_x);
    auto raw_y = uint16_t((bang.y.tile << 8 | bang.y.frac) + bang.vel_y);
    MapPos x{uint8_t(raw_x), uint8_t(raw_x >> 8)};
    MapPos y{uint8_t(raw_y), uint8_t(raw_y >> 8)};
    move_node(world, id, x, y);
    bang.altitude = int16_t(bang.altitude + bang.vel_z);
    bang.vel_z = int16_t(bang.vel_z - 0x200);
    bang.vel_x = int16_t(bang.vel_x - (bang.vel_x >> 2));
    bang.vel_y = int16_t(bang.vel_y - (bang.vel_y >> 2));
    bang.vel_z = int16_t(bang.vel_z - (bang.vel_z >> 2));
    bool gone = bang.altitude < 0;
    if (!gone && bang.bang_room) {
        const RoomInfo &room = world.rooms[bang.bang_room % kRoomCount];
        int x0 = room.pos % kMapSize, y0 = room.pos / kMapSize;
        gone = x.tile < x0 || x.tile == x0 + room.width - 1 || y.tile < y0 || y.tile == y0 + room.height - 1;
    }
    if (!gone && bang.anim_speed && game_turn % bang.anim_speed == 0 && bang.sprite < anims.frames.size()) {
        uint16_t next = anims.frames[bang.sprite].next;
        gone = next < anims.frames.size() && (anims.frames[next].flags & 1);
        if (!gone)
            bang.sprite = next;
    }
    if (!gone)
        return;
    // remove_thing (src/utils.cpp).
    trace_event("remove_thing %u type=%u", unsigned(id), unsigned(bang.type));
    remove_node(world, id);
    bang = Thing{};
}

void replace_machine(Game &game, uint16_t id)
{
    World &world = game.world;
    Thing &machine = world.things[id];
    trace_event("replace_machine %u used=%d strength=%d", unsigned(id), int(machine.times_used), int(machine.strength));
    int32_t cost = object_cost(world, machine.subtype);
    if (game.balance < cost || game.balance <= 0) {
        world.building.sounds.push_back(kSfxNo4);
        return;
    }
    change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxReplacementMachine, -cost});
    machine.strength = object_strength(world, machine.subtype);
    machine.times_used = 0;
    machine.overlay_frame = 0;
    machine.smoke = {};
    reset_icon_value(machine, kIconNeedsRepair);
    World::ThingPager &pager = world.repair_pager;
    for (int i = pager.count - 1; i >= 0; i--) {
        const World::ThingPage &page = pager.pages[size_t(i)];
        if (page.id == id && page.responder) {
            interrupt_staff(world, game.anims(), page.responder, game.turn_count);
            break;
        }
    }
    thingpage_cancel(pager, id);
    world.building.sounds.push_back(kSfxSelectX);
}

bool apply_machine_event(Game &game, const Event &event)
{
    World &world = game.world;
    bool machine = event.value && event.value < world.things.size();
    switch (event.type) {
    case EV_REPLACE_MACHINE:
        if (machine)
            replace_machine(game, event.value);
        return true;
    case EV_THINGPAGE_REQUEST:
        trace_event("repair_request %u", unsigned(event.value));
        if (machine) {
            thingpage_request(world.repair_pager, event.value, 0xff);
            set_icon_value(game.anims(), world.things[event.value], kIconNeedsRepair);
        }
        return true;
    default:
        return false;
    }
}

} // namespace th
