#include "core/world/nav.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

namespace th
{

const std::array<PersonAnims, 15> kPatientAnims = {{
    {8, 12},
    {0, 4},
    {429, 433},
    {489, 493},
    {409, 413},
    {417, 421},
    {532, 536},
    {742, 746},
    {821, 920},
    {203, 207},
    {1438, 1442},
    {429, 433},
    {1506, 1510},
    {1352, 1356},
    {1799, 1803},
}};

const std::array<PersonAnims, 5> kStaffAnims = {{
    {603, 825},
    {16, 20},
    {929, 933},
    {1834, 1838},
    {1144, 1148},
}};

namespace
{

// A cell index as the game computes it, from coordinates that may be off
// the map; nullptr for one outside map[] (see wall_in_way).
const MapCell *cell_at(const World &w, int32_t x, int32_t y)
{
    int index = int(int16_t(x)) + int(int16_t(y)) * 128;
    return index >= 0 && index < kMapCells ? &w.map[size_t(index)] : nullptr;
}

bool impassable(const MapCell *c)
{
    return c && (c->flags & 1) && !(c->flags & 2);
}

// opposite_angle (src/tables.cpp), with the entry past its end the game
// reads for a stuck person (facing 4): next_angle[0], which follows it.
constexpr std::array<uint8_t, 5> kOppositeAngle = {2, 3, 0, 1, 1};

constexpr std::array<int32_t, 4> kReverse = {2, 3, 0, 1};

// The cell after (x, y) in `dir`, unchanged for 4 or more, as the switches
// of wall_in_way and person_in_way.
void step(int32_t &x, int32_t &y, uint32_t dir)
{
    if (dir < 4) {
        x += kAroundXy[dir][0];
        y += kAroundXy[dir][1];
    }
}

// The game's wallhug_dirn_steps applied to a byte waypoint: a step off the
// edge wraps to 0xff.
Waypoint step(Waypoint p, uint32_t dir)
{
    return {uint8_t(p.x + kAroundXy[dir][0]), uint8_t(p.y + kAroundXy[dir][1])};
}

bool same(Waypoint a, Waypoint b)
{
    return a.x == b.x && a.y == b.y;
}

// bresenham_start (0x48960) and bresenham (0x48a6c). The game keeps this
// state in globals shared by every line; each user here restarts it where
// the game does, so a local state is equivalent.
struct Bresenham {
    int32_t dx = 0;
    int32_t dy = 0;
    int32_t val = 0;
    uint32_t x_dirn = 0;
    uint32_t y_dirn = 0;

    Bresenham(Waypoint from, Waypoint to)
    {
        int32_t ddx = int32_t(to.x) - from.x;
        int32_t ddy = int32_t(to.y) - from.y;
        x_dirn = ddx > 0 ? 1 : 3;
        y_dirn = ddy > 0 ? 2 : 0;
        if (from.x == to.x)
            x_dirn = y_dirn;
        if (from.y == to.y)
            y_dirn = x_dirn;
        dx = std::abs(ddx);
        dy = std::abs(ddy);
        val = dx / 2 + dy / 2;
    }

    uint32_t next()
    {
        if (val >= dx) {
            val -= dx;
            return y_dirn;
        }
        val += dy;
        return x_dirn;
    }
};

// line_of_sight_cleanup (0x48b64).
bool line_of_sight_cleanup(const World &w, WallhugPath &path, uint32_t first)
{
    bool changed = false;
    bool progress;
    do {
        uint32_t i = first;
        uint32_t j = i;
        Waypoint cur = first == 0 ? path.start : path.route[first - 1];
        progress = false;
        while (path.count > j) {
            bool skipped = false;
            for (uint32_t k = 4; k > 0 && !skipped; k--) {
                if (j + k < path.count && line_of_sight(w, cur, path.route[j + k])) {
                    j += k;
                    progress = changed = skipped = true;
                }
            }
            if (skipped)
                continue;
            cur = path.route[i] = path.route[j];
            i++;
            j++;
        }
        path.count = uint8_t(i);
    } while (progress);
    return changed;
}

// The game's wallhug_hugger: one of the two wall followers.
struct Hugger {
    // Corners recorded in route.
    uint8_t count = 0;
    Waypoint end;
    std::array<Waypoint, 0xfc> route{};
    // Heading 0-3; kHugBack once back on the line to `end`, kHugFull when
    // route is full.
    uint32_t dir = 0;
    // The side the wall is kept on, -1 or 1, added to dir to turn.
    int32_t turn = 0;
    Waypoint pos;
    // Where the wall was hit.
    Waypoint hit;
};

constexpr uint32_t kHugBack = 0x32;
constexpr uint32_t kHugFull = 0x33;

// wallhug_hugstep (0x49400).
void hugstep(const World &w, Hugger &h)
{
    if (h.dir == kHugFull)
        return;
    if (wall_in_way(w, h.pos.x, h.pos.y, h.dir)) {
        h.dir = uint32_t(int32_t(h.dir) - h.turn) & 3;
        h.route[h.count++] = h.pos;
        if (h.count > 0xfb) {
            h.dir = kHugFull;
            return;
        }
    } else {
        h.pos = step(h.pos, h.dir);
        uint32_t dir = uint32_t(int32_t(h.dir) + h.turn) & 3;
        if (!wall_in_way(w, h.pos.x, h.pos.y, dir)) {
            h.dir = dir;
            h.route[h.count++] = h.pos;
            if (h.count > 0xfb) {
                h.dir = kHugFull;
                return;
            }
        }
    }
    // Back on course once `end` does not lie beyond the wall side...
    switch (uint32_t(int32_t(h.dir) + h.turn) & 3) {
    case 0:
        if (h.end.y < h.pos.y)
            return;
        break;
    case 1:
        if (h.end.x > h.pos.x)
            return;
        break;
    case 2:
        if (h.end.y > h.pos.y)
            return;
        break;
    case 3:
        if (h.end.x < h.pos.x)
            return;
        break;
    }
    // ... nor behind the heading...
    switch (h.dir) {
    case 0:
        if (h.end.y > h.pos.y)
            return;
        break;
    case 1:
        if (h.end.x < h.pos.x)
            return;
        break;
    case 2:
        if (h.end.y < h.pos.y)
            return;
        break;
    case 3:
        if (h.end.x > h.pos.x)
            return;
        break;
    }
    // ... and the follower is within the box from the hit to `end`, not at
    // the hit.
    if (h.hit.x <= h.end.x && (h.pos.x < h.hit.x || h.pos.x > h.end.x))
        return;
    if (h.hit.x >= h.end.x && (h.pos.x > h.hit.x || h.pos.x < h.end.x))
        return;
    if (h.hit.y <= h.end.y && (h.pos.y < h.hit.y || h.pos.y > h.end.y))
        return;
    if (h.hit.y >= h.end.y && (h.pos.y > h.hit.y || h.pos.y < h.end.y))
        return;
    if (same(h.hit, h.pos))
        return;
    h.dir = kHugBack;
}

// huggers_met_again (0x492f0): the first follower is about to step onto the
// second, facing it, so the obstacle encloses one of the ends.
bool huggers_met_again(const Hugger &a, const Hugger &b)
{
    if (a.dir == kHugFull || a.dir == kHugBack)
        return false;
    return same(b.pos, step(a.pos, a.dir)) && ((a.dir + 2) & 3) == b.dir;
}

// wallhug_add_huggers_path (0x49230).
bool add_huggers_path(WallhugPath &path, const Hugger &h)
{
    if (h.count + path.count + 2 > 0xfc)
        return false;
    for (int i = 0; i < h.count; i++)
        path.route[path.count++] = h.route[size_t(i)];
    path.route[path.count++] = h.pos;
    return true;
}

// wallhug_continue_trivial (0x48ea4); `steps` is the game's
// wallhug_current_count. Its bookkeeping for wallhug_looking_for_last is
// left out: nothing in the game sets that flag.
uint32_t continue_trivial(const World &w, WallhugPath &path, Waypoint cur, uint32_t limit, uint32_t &steps)
{
    std::array<Hugger, 2> huggers;
    Hugger &left = huggers[0];
    Hugger &right = huggers[1];
    Bresenham line(cur, path.end);
    for (; steps < limit && !same(cur, path.end); steps++) {
        uint32_t dir = line.next();
        if (!wall_in_way(w, cur.x, cur.y, dir)) {
            cur = step(cur, dir);
            continue;
        }
        path.route[path.count++] = cur;
        left.pos = right.pos = left.hit = right.hit = cur;
        left.turn = -1;
        right.turn = 1;
        left.dir = (dir + 1) & 3;
        right.dir = (dir - 1) & 3;
        left.end = right.end = path.end;
        left.count = right.count = 0;
        while (steps < limit) {
            hugstep(w, left);
            if (huggers_met_again(left, right))
                goto fail;
            hugstep(w, right);
            if (huggers_met_again(left, right))
                goto fail;
            if (left.dir == kHugBack) {
                if (!add_huggers_path(path, left))
                    goto fail;
                break;
            }
            if (right.dir == kHugBack) {
                if (!add_huggers_path(path, right))
                    goto fail;
                break;
            }
            if (left.dir == kHugFull && right.dir == kHugFull)
                goto fail;
            steps++;
        }
        if (steps == limit)
            goto fail;
        line_of_sight_cleanup(w, path, 0);
        cur = path.route[size_t(path.count - 1)];
        line = Bresenham(cur, path.end);
    }
    if (path.count >= 0xfa)
        return kWallhugFailed;
    if (same(cur, path.end)) {
        path.route[path.count++] = cur;
        return steps;
    }
    return kWallhugFailed;
fail:
    path.route[path.count++] = cur;
    return kWallhugFailed;
}

// wallhug_cleanup (0x48c78).
uint32_t wallhug_cleanup(const World &w, WallhugPath &path, uint32_t result)
{
    WallhugPath sub;
    int tries = 10;
    uint32_t from = 0;
restart:
    line_of_sight_cleanup(w, path, from);
    if (--tries == 0)
        return result;
    for (uint32_t i = 0; i + 1 < path.count; i++) {
        sub.start = i == 0 ? path.start : path.route[i - 1];
        sub.end = path.route[i + 1];
        if (wallhug_trivial(w, sub) == kWallhugFailed)
            continue;
        bool through = false;
        for (uint32_t k = 0; k + 1 < sub.count; k++)
            through |= same(sub.route[k], path.route[i]);
        if (through)
            continue;
        if (sub.count + path.count - 2 > 0xfc)
            continue;
        // route[i] and route[i + 1] become sub's route, which ends at route[i + 1].
        std::memmove(&path.route[i + sub.count], &path.route[i + 2], (path.count - i - 2) * sizeof(Waypoint));
        std::memcpy(&path.route[i], sub.route.data(), sub.count * sizeof(Waypoint));
        path.count = uint8_t(sub.count + path.count - 2);
        from = i < 4 ? 0 : i - 4;
        goto restart;
    }
    return result;
}

// delete_first_path_entry (0x553e0). With count 0 (orthonav_wn after a
// failed orthonav_init) the game's count wraps to 255 and its shifting loop
// runs 255 entries on, over the rest of the thing and the things after it;
// here the shift stays within steps.
void delete_first_path_entry(OrthonavPath &path)
{
    path.x0 = path.steps[0].x;
    path.y0 = path.steps[0].y;
    path.count--;
    for (size_t i = 0; i < path.count && i + 1 < path.steps.size(); i++)
        path.steps[i] = path.steps[i + 1];
}

int32_t rand_ffff(World &w)
{
    return int32_t(old_newrand(w.seed) % 0xffff);
}

// orthonav_wn (0x5562c): the direction to step from (x, y) towards the
// path's next waypoint, keeping near the line from (x0, y0) to it, with some
// randomness; kNavArrived on the last waypoint. `other` (the game's
// other_wherefacing) gets the second-best direction.
int orthonav_wn(World &w, OrthonavPath &path, int32_t x, int32_t y, int32_t dir, int32_t &other)
{
    int32_t ldx;
    int32_t ldy;
    int32_t dx;
    int32_t dy;
    int32_t xdir;
    int32_t ydir;
    int32_t nx = 0;
    int32_t ny = 0;
    int32_t fx;
    int32_t fy;
    bool deleted = false;

    auto next_cell = [&] {
        nx = x + kAroundXy[size_t(dir)][0];
        ny = y + kAroundXy[size_t(dir)][1];
    };
    auto swap_dirs = [&] { std::swap(dir, other); };

again:
    if (path.steps[0].x == x && path.steps[0].y == y) {
        if (path.count == 1)
            return kNavArrived;
        delete_first_path_entry(path);
        deleted = true;
    }
    ldx = (path.steps[0].x - path.x0) << 8;
    ldy = (path.steps[0].y - path.y0) << 8;
    dx = (path.steps[0].x - x) << 8;
    dy = (path.steps[0].y - y) << 8;
    if (dx > 0)
        xdir = 1;
    else if (dx < 0)
        xdir = 3;
    else if (dir == 3 || dir == 1)
        xdir = dir;
    else
        xdir = 1;
    if (dy > 0)
        ydir = 2;
    else if (dy < 0)
        ydir = 0;
    else if (dir == 2 || dir == 0)
        ydir = dir;
    else
        ydir = 2;
    if (dir != xdir && dir != ydir) {
        if (dir == 1 || dir == 3) {
            dir = ydir;
            other = xdir;
        } else {
            dir = xdir;
            other = ydir;
        }
    } else if (dir == xdir) {
        other = ydir;
    } else {
        other = xdir;
    }
    if (path.count > 1) {
        if (!deleted && !wall_in_way(w, x, y, uint32_t(dir)) &&
            ((path.steps[0].x == x && dir == xdir) || (path.steps[0].y == y && dir == ydir))) {
            // In line with the waypoint and stepping on: drop it unless the
            // one after lies back the way it is going.
            int32_t ex = path.steps[1].x - x;
            int32_t ey = path.steps[1].y - y;
            bool keep = (dir == 0 && ey > 0) || (dir == 2 && ey < 0) || (dir == 3 && ex > 0) || (dir == 1 && ex < 0);
            if (!keep) {
                delete_first_path_entry(path);
                deleted = true;
                goto again;
            }
        }
        if ((path.steps[0].x == x && dir == ydir) || (path.steps[0].y == y && dir == xdir))
            return dir;
    } else {
        if (path.steps[0].x == x)
            return path.steps[0].y < y ? 0 : 2;
        if (path.steps[0].y == y)
            return path.steps[0].x < x ? 3 : 1;
    }
    if (ldx == 0 && dx == 0)
        return ydir;
    if (ldy == 0 && dy == 0)
        return xdir;
    if (wall_in_way(w, x, y, uint32_t(dir)))
        return other;
retry:
    next_cell();
    if (rand_ffff(w) < 0xa000) {
        if (wall_in_way(w, nx, ny, uint32_t(dir))) {
            swap_dirs();
            next_cell();
        } else if (rand_ffff(w) < 0x8000) {
            int32_t nx2 = nx + kAroundXy[size_t(dir)][0];
            int32_t ny2 = ny + kAroundXy[size_t(dir)][1];
            if (wall_in_way(w, nx2, ny2, uint32_t(dir))) {
                swap_dirs();
                next_cell();
            }
        }
    }
    // Stepping past both the start and the waypoint.
    switch (dir) {
    case 0:
        if (path.y0 > ny && path.steps[0].y > ny)
            return other;
        break;
    case 2:
        if (path.y0 < ny && path.steps[0].y < ny)
            return other;
        break;
    case 3:
        if (path.x0 > nx && path.steps[0].x > nx)
            return other;
        break;
    case 1:
        if (path.x0 < nx && path.steps[0].x < nx)
            return other;
        break;
    }
    // A clear row or column from the next cell back to the line?
    if (dir == ydir) {
        if ((path.x0 > nx && ldx >= 0) || (path.x0 < nx && ldx <= 0))
            goto swap;
        if (ldx == 0)
            return dir;
        fy = (((nx - path.x0) << 8) * ldy + 0x80) / ldx + (path.y0 << 8) + 0x80;
        if ((ny << 8 < fy && dir == 0) || (ny << 8 > fy && dir == 2))
            goto swap;
        fx = nx << 8;
    } else {
        if ((path.y0 > ny && ldy >= 0) || (path.y0 < ny && ldy <= 0))
            goto swap;
        if (ldy == 0)
            return dir;
        fx = (((ny - path.y0) << 8) * ldx + 0x80) / ldy + (path.x0 << 8) + 0x80;
        if ((nx << 8 < fx && dir == 3) || (nx << 8 > fx && dir == 1))
            goto swap;
        fy = ny << 8;
    }
    if (free_orthog_line(w, nx, ny, fx >> 8, fy >> 8))
        return dir;
swap:
    if (rand_ffff(w) > 0xe000) {
        swap_dirs();
        goto retry;
    }
    // nx, ny are still the cell `dir` leads to, not swapped with it.
    if (dir == xdir) {
        if ((path.x0 > nx && ldx > 0) || (path.x0 < nx && ldx < 0))
            return other;
        if (ldx == 0)
            fy = ny << 8;
        else
            fy = (path.y0 << 8) + (((nx - path.x0) << 8) * ldy + 0x80) / ldx + 0x80;
        fx = nx << 8;
    } else {
        if ((path.y0 > ny && ldy > 0) || (path.y0 < ny && ldy < 0))
            return other;
        if (ldy == 0)
            fx = nx << 8;
        else
            fx = (path.x0 << 8) + (((ny - path.y0) << 8) * ldx + 0x80) / ldy + 0x80;
        fy = ny << 8;
    }
    if (free_orthog_line(w, nx, ny, fx >> 8, fy >> 8))
        return dir;
    return other;
}

// person_in_way (0x442a0): a patient or member of staff on the cell next to
// (x, y) in `dir` faces `walker` head-on. Only the cell's first four things
// are looked at.
int32_t person_in_way(const World &w, uint16_t walker, int32_t x, int32_t y, uint32_t dir)
{
    step(x, y, dir);
    const MapCell *cell = cell_at(w, x, y);
    int id = cell ? cell->first : 0;
    uint8_t facing = w.things[walker].facing;
    for (int i = 0; i < 4 && id > 0; i++) {
        const Thing &other = w.things[size_t(id)];
        if ((other.type == kThingPatient || other.type == kThingStaff) && id != walker &&
            other.facing == kOppositeAngle[facing])
            return 0x8000;
        id = other.cell_next;
    }
    return 0;
}

// person_overlap (0x443ec): a patient or member of staff of a higher thing
// index shares `walker`'s cell and facing. Only the cell's first four things
// are looked at.
int32_t person_overlap(const World &w, uint16_t walker)
{
    const Thing &self = w.things[walker];
    int id = w.map[size_t(self.y.tile * kMapSize + self.x.tile)].first;
    for (int i = 0; i < 4 && id > 0; i++) {
        const Thing &other = w.things[size_t(id)];
        if ((other.type == kThingPatient || other.type == kThingStaff) && id > walker && other.facing == self.facing)
            return 0x8000;
        id = other.cell_next;
    }
    return 0;
}

} // namespace

bool wall_in_way(const World &world, int32_t x, int32_t y, uint32_t dir)
{
    if ((dir == 0 && y == 0) || (dir == 1 && uint32_t(x) >= 0x7f) || (dir == 2 && uint32_t(y) >= 0x7f) ||
        (dir == 3 && x == 0))
        return true;
    const MapCell *cell = cell_at(world, x, y);
    if (impassable(cell))
        return true;
    const MapCell *edge = nullptr;
    uint8_t MapCell::*side = &MapCell::wall_n;
    switch (dir) {
    case 0:
        edge = cell;
        break;
    case 1:
        edge = cell_at(world, x + 1, y);
        side = &MapCell::wall_w;
        break;
    case 2:
        edge = cell_at(world, x, y + 1);
        break;
    case 3:
        edge = cell;
        side = &MapCell::wall_w;
        break;
    }
    if (edge && edge->*side)
        return true;
    step(x, y, dir);
    return impassable(cell_at(world, x, y));
}

bool line_of_sight(const World &world, Waypoint from, Waypoint to)
{
    Waypoint cur = from;
    Bresenham line(from, to);
    while (!same(cur, to)) {
        uint32_t dir = line.next();
        if (wall_in_way(world, cur.x, cur.y, dir))
            return false;
        cur = step(cur, dir);
    }
    return true;
}

uint32_t wallhug_trivial(const World &world, WallhugPath &path)
{
    path.count = 0;
    uint32_t steps = 0;
    return continue_trivial(world, path, path.start, 60000, steps);
}

uint32_t wallhug_tricky(const World &world, WallhugPath &path)
{
    uint32_t result = wallhug_trivial(world, path);
    return wallhug_cleanup(world, path, result);
}

uint32_t astar_route(const World &world, WallhugPath &path)
{
    path.count = 0;
    const int start = path.start.x + path.start.y * kMapSize;
    const int goal = path.end.x + path.end.y * kMapSize;
    if (path.start.x >= kMapSize || path.start.y >= kMapSize || path.end.x >= kMapSize || path.end.y >= kMapSize)
        return kWallhugFailed;
    if (start == goal) {
        path.route[0] = path.end;
        path.count = 1;
        return 0;
    }
    auto h = [&](int cell) { return std::abs(cell % kMapSize - path.end.x) + std::abs(cell / kMapSize - path.end.y); };
    constexpr int kUnseen = std::numeric_limits<int>::max();
    std::vector<int> g(kMapCells, kUnseen);
    std::vector<int> from(kMapCells, -1);
    std::vector<bool> closed(kMapCells, false);
    // Open cells by (f, h, insertion order): ties go to the cell nearer the
    // goal, then to the earlier one, so routes are the same every time.
    using Entry = std::tuple<int, int, uint32_t, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
    uint32_t order = 0;
    g[size_t(start)] = 0;
    open.emplace(h(start), h(start), order++, start);
    while (!open.empty()) {
        int cell = std::get<3>(open.top());
        open.pop();
        if (closed[size_t(cell)])
            continue;
        closed[size_t(cell)] = true;
        if (cell == goal)
            break;
        int x = cell % kMapSize, y = cell / kMapSize;
        for (uint32_t dir = 0; dir < 4; dir++) {
            if (wall_in_way(world, x, y, dir))
                continue;
            int next = (x + kAroundXy[dir][0]) + (y + kAroundXy[dir][1]) * kMapSize;
            int cost = g[size_t(cell)] + 1;
            if (closed[size_t(next)] || cost >= g[size_t(next)])
                continue;
            g[size_t(next)] = cost;
            from[size_t(next)] = cell;
            open.emplace(cost + h(next), h(next), order++, next);
        }
    }
    if (g[size_t(goal)] == kUnseen)
        return kWallhugFailed;

    std::vector<Waypoint> cells;
    for (int cell = goal; cell != -1; cell = from[size_t(cell)])
        cells.push_back({uint8_t(cell % kMapSize), uint8_t(cell / kMapSize)});
    std::reverse(cells.begin(), cells.end());
    // Pull the path taut: from each waypoint, the furthest cell ahead still in
    // line of sight (looking up to 64 cells on).
    size_t cur = 0;
    while (cur + 1 < cells.size() && path.count < path.route.size()) {
        size_t best = cur + 1;
        for (size_t j = cur + 2; j < cells.size() && j <= cur + 64; j++)
            if (line_of_sight(world, cells[cur], cells[j]))
                best = j;
        path.route[path.count++] = cells[best];
        cur = best;
    }
    if (cur + 1 < cells.size())
        return kWallhugFailed; // more waypoints than a route holds
    return uint32_t(g[size_t(goal)]);
}

void orthonav_init(const World &world, OrthonavPath &path, int x0, int y0, int x1, int y1)
{
    WallhugPath hug;
    hug.start = {uint8_t(x0), uint8_t(y0)};
    hug.end = {uint8_t(x1), uint8_t(y1)};
    uint32_t found = world.path_finder == PathFinder::AStar ? astar_route(world, hug) : wallhug_tricky(world, hug);
    if (found == kWallhugFailed) {
        path.count = 0;
        return;
    }
    path.count = hug.count > 4 ? 4 : hug.count;
    path.x0 = uint8_t(x0);
    path.y0 = uint8_t(y0);
    path.x1 = uint8_t(x1);
    path.y1 = uint8_t(y1);
    for (size_t i = 0; i < path.count; i++)
        path.steps[i] = hug.route[i];
}

bool free_orthog_line(const World &world, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (x1 == x2) {
        if (y2 < y1)
            std::swap(y1, y2);
        for (; y1 < y2; y1++)
            if (wall_in_way(world, x1, y1, 2))
                return false;
    } else {
        if (x2 < x1)
            std::swap(x1, x2);
        for (; x1 < x2; x1++)
            if (wall_in_way(world, x1, y1, 1))
                return false;
    }
    return true;
}

int orthonav_whatnext(World &world, uint16_t walker, OrthonavPath &path, int x, int y, int dir)
{
    auto blocked_at = [&](int32_t d) { return wall_in_way(world, x, y, uint32_t(d)); };
    if (path.count == 0) {
        if (blocked_at(0) && blocked_at(2) && blocked_at(3) && blocked_at(1))
            return kNavWalledIn;
        return kNavNoPath;
    }
    int32_t other = 0;
    int next_dir = orthonav_wn(world, path, x, y, dir, other);
    if (next_dir == kNavArrived) {
        if (path.x1 == x && path.y1 == y)
            return kNavArrived;
        // At the last waypoint kept, short of the destination: plan the
        // rest. The game passes x1, y1 as signed bytes.
        orthonav_init(world, path, x, y, int8_t(path.x1), int8_t(path.y1));
        next_dir = orthonav_wn(world, path, x, y, dir, other);
    }
    if (blocked_at(next_dir)) {
        orthonav_init(world, path, x, y, int8_t(path.x1), int8_t(path.y1));
        next_dir = orthonav_wn(world, path, x, y, dir, other);
        if (blocked_at(next_dir)) {
            // Walled off from the plan: an open direction, from a random
            // roll that each direction not leading away from the next
            // waypoint uses up.
            next_dir = kNavWalledIn;
            int32_t roll = int32_t(old_newrand(world.seed) % 0xffff) + 0xffff;
            do {
                if (!blocked_at(0))
                    next_dir = 0;
                if (path.steps[0].y <= y)
                    roll -= 0x4000;
                if (roll < 0)
                    break;
                if (!blocked_at(2))
                    next_dir = 2;
                if (path.steps[0].y >= y)
                    roll -= 0x4000;
                if (roll < 0)
                    break;
                if (!blocked_at(3))
                    next_dir = 3;
                if (path.steps[0].x <= x)
                    roll -= 0x4000;
                if (roll < 0)
                    break;
                if (!blocked_at(1))
                    next_dir = 1;
                if (path.steps[0].x >= x)
                    roll -= 0x4000;
            } while (roll >= 0);
            path.x1 = 0xff;
            return next_dir;
        }
    }
    // next_dir can be kNavArrived here, when a new plan's only waypoint is
    // where it stands: the game then looks for people on its own cell and,
    // with none, returns it as an arrival short of the destination.
    int32_t blocked = person_in_way(world, walker, x, y, uint32_t(next_dir)) | person_overlap(world, walker);
    if (blocked == 0)
        return next_dir;
    auto open = [&](int32_t d) { return !blocked_at(d) && !person_in_way(world, walker, x, y, uint32_t(d)); };
    if (open(other) && rand_ffff(world) < blocked + 0x3000) {
        next_dir = other;
    } else {
        int32_t alt_dir = kReverse[size_t(other)];
        if (open(alt_dir) && rand_ffff(world) < blocked + 0x3000) {
            next_dir = alt_dir;
        } else if (rand_ffff(world) < blocked - 0x2000) {
            // Pushes on into them, keeping the path.
            return next_dir;
        } else {
            // For kNavArrived the game's switch leaves alt_dir as it was.
            if (next_dir < 4)
                alt_dir = kReverse[size_t(next_dir)];
            if (!open(alt_dir)) {
                path.x1 = 0xff;
                return kNavBlockedByPeople;
            }
            next_dir = alt_dir;
        }
    }
    path.x1 = 0xff;
    return next_dir;
}

void nav_init(const World &world, Thing &person)
{
    orthonav_init(world, person.path, person.x.tile, person.y.tile, person.dest_x, person.dest_y);
}

int person_head_for(World &world, uint16_t person)
{
    Thing &t = world.things[person];
    if (t.path.count == 0)
        nav_init(world, t);
    if (t.dest_x != t.path.x1 || t.dest_y != t.path.y1)
        nav_init(world, t);
    // The game passes the cell as signed bytes.
    return orthonav_whatnext(world, person, t.path, int8_t(t.x.tile), int8_t(t.y.tile), t.facing);
}

void set_destination_coords(World &world, uint16_t person, int tile)
{
    Thing &t = world.things[person];
    t.dest_x = uint8_t(tile % kMapSize);
    t.dest_y = uint8_t(tile / kMapSize);
    t.countdown = 0;
    nav_init(world, t);
}

void insert_node(World &world, uint16_t id, MapPos x, MapPos y)
{
    Thing &node = world.things[id];
    MapCell &cell = world.map[size_t((y.tile << 7) + x.tile)];
    if (cell.first) {
        node.cell_next = cell.first;
        if (node.cell_next > 0)
            world.things[uint16_t(cell.first)].cell_prev = int16_t(id);
    }
    cell.first = int16_t(id);
}

void remove_node(World &world, uint16_t id)
{
    Thing &node = world.things[id];
    if (node.cell_prev == 0) {
        MapCell &cell = world.map[size_t((node.y.tile << 7) + node.x.tile)];
        cell.first = node.cell_next;
        if (cell.first > 0)
            world.things[size_t(node.cell_next)].cell_prev = 0;
    } else if ((world.things[size_t(node.cell_prev)].cell_next = node.cell_next) > 0) {
        world.things[size_t(node.cell_next)].cell_prev = node.cell_prev;
    }
    node.cell_next = 0;
    node.cell_prev = 0;
}

bool move_node(World &world, uint16_t id, MapPos x, MapPos y)
{
    if (x.tile >= 0x80 || y.tile >= 0x80)
        return false;
    Thing &node = world.things[id];
    if (x.tile != node.x.tile || y.tile != node.y.tile) {
        remove_node(world, id);
        insert_node(world, id, x, y);
        node.x = x;
        node.y = y;
        return true;
    }
    node.x = x;
    node.y = y;
    return false;
}

void play_frame(Thing &person, const AnimData &anims, uint16_t anim)
{
    size_t start = size_t(anim == 9999 ? 0 : anim) + person.facing;
    if (start >= anims.starts.size())
        return;
    person.sprite = uint16_t(anims.starts[start].frame);
    person.flip = anims.starts[start].flip ? 1 : 0;
}

WalkResult walk_turn(World &world, const AnimData &anims, uint16_t person, uint32_t game_turn)
{
    Thing &t = world.things[person];
    const PersonAnims &kind = t.type == kThingStaff ? kStaffAnims[t.subtype] : kPatientAnims[t.subtype];
    if (game_turn % t.anim_speed == 0)
        t.sprite = anims.frames[t.sprite].next;
    t.countdown--;
    if (t.countdown <= 0) {
        t.countdown = int16_t(0x100 / t.speed);
        int dir = person_head_for(world, person);
        switch (dir) {
        case kNavArrived:
            t.countdown = 0;
            return WalkResult::Arrived;
        case kNavBlockedByPeople:
            play_frame(t, anims, kind.stand);
            t.countdown = 0;
            return WalkResult::BlockedByPeople;
        case kNavWalledIn:
            return WalkResult::WalledIn;
        case kNavNoPath:
            return WalkResult::NoPath;
        default: {
            // Unlike play_frame, without check_frame.
            size_t stand = size_t(kind.stand) + t.facing;
            if (t.facing != dir || (stand < anims.starts.size() && anims.starts[stand].frame == t.sprite)) {
                t.facing = uint8_t(dir);
                play_frame(t, anims, kind.walk);
            }
            break;
        }
        }
    }
    // The game indexes around_xy with a stuck person's facing 4 too, off the
    // table's end, if a walk resumes with countdown left; its callers resume
    // walks with none. Here such a person does not move.
    if (t.facing >= 4)
        return WalkResult::Walking;
    uint16_t raw_x = uint16_t((t.x.tile << 8 | t.x.frac) + t.speed * kAroundXy[t.facing][0]);
    uint16_t raw_y = uint16_t((t.y.tile << 8 | t.y.frac) + kAroundXy[t.facing][1] * t.speed);
    move_node(world, person, {uint8_t(raw_x), uint8_t(raw_x >> 8)}, {uint8_t(raw_y), uint8_t(raw_y >> 8)});
    return WalkResult::Walking;
}

} // namespace th
