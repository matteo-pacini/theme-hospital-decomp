#pragma once

#include <cstdint>

namespace th
{

struct World;
struct Thing;

// RoomInfo.type of a training room, and the objects it uses: the projector a
// consultant starts, the lecture chair a student sits on, and the skeleton.
constexpr uint8_t kRoomTraining = 22;
constexpr uint8_t kObjLectureChair = 36;
constexpr uint8_t kObjProjector = 37;
constexpr uint8_t kObjSkeleton = 60;

// find_training_room_no_consultant (0x43168): player `owner`'s training room
// other than `exclude` with no consultant among the people in it, or 0.
uint16_t find_training_room_no_consultant(const World &world, uint8_t owner, uint16_t exclude);

// find_emptiest_training_room (0x43294): player `owner`'s training room other
// than `exclude` with the most lecture chairs left free by students (chairs
// minus doctors without the consultant skill in it), or 0 if none. Draws
// nothing.
uint16_t find_emptiest_training_room(const World &world, uint8_t owner, uint16_t exclude);

// training_done (0x43434): one lesson of `kind` (0 lecture, 1 skeleton,
// 2 bookcase) by `consultant` in their room. Each seated student (a doctor
// without the consultant skill on a lecture chair) whose ability is below a
// random part of the consultant's raises it and gains training points
// towards the surgeon, psychiatrist and researcher skills the consultant
// has and they lack, getting each skill once past its threshold (announced
// for the consultant's player, set_expertise_staff). The ability gain is
// divided by a quarter of the seated students plus one, and by a count that
// grows by one for each student met with a skill to learn. Draws once.
void training_done(World &world, const Thing &consultant, uint8_t kind);

} // namespace th
