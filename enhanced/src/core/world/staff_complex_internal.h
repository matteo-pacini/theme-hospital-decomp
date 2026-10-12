#pragma once

// What staff_complex.cpp and staff_complex_actions.cpp share: the run's
// context and the steps' helpers (src/complex.cpp, src/interact.cpp).

#include "core/data/anims.h"
#include "core/world/complex.h"
#include "core/world/complex_tables.h"
#include "core/world/staff_complex.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

inline namespace staff_complex_detail
{

struct Run {
    World &world;
    const AnimData &anims;
    const PersonComplexes &person;
    const StaffComplexes &staff;
    uint32_t game_turn;
};

// What a step of a complex returns when it carried on; any other value is
// what process returns at once.
constexpr uint8_t kStepDone = 1;

uint16_t cell_of(const Thing &t);
size_t use_side(const Thing &staff);
void play_staff_anim(const AnimData &anims, Thing &s, size_t column);
void head_off(const Run &r, uint16_t id);
uint16_t can_i_go_near_point(const World &world, const Thing &person, uint16_t target);
void fail(const Run &r, uint16_t id);
uint8_t process(const Run &r, uint16_t id);
bool check_room_mutex_on_entry(World &world, uint16_t id);
void clear_my_mutex(World &world, uint16_t id);
void set_start(Thing &t, const AnimData &anims, size_t start);
void staff_success(World &world, Thing &s, uint16_t kind);
uint8_t process_either(const Run &r, uint16_t id);
void setup_next_interaction(const Run &r, uint16_t id);
void start_interaction(const Run &r, uint16_t id, uint8_t seq);
void interaction_frame(const Run &r, Thing &t);
uint8_t process_action(const Run &r, uint16_t id, const ComplexStep &entry, uint16_t obj_id, uint16_t tile);

} // namespace staff_complex_detail
} // namespace th
