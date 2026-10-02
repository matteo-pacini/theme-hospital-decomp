#include "hosp.h"

s32 wall_in_way(u32 x, u32 y, u32 dir);
void bresenham_start(wallhug_waypoint from, wallhug_waypoint to);
u32 bresenham(void);
u32 line_of_sight(wallhug_waypoint from, wallhug_waypoint to);
u32 wallhug_cleanup(wallhug_path *path, u32 result);
u32 huggers_met_again(wallhug_info *info);
u32 line_of_sight_cleanup(wallhug_path *path, u32 first);
u32 wallhug_continue_trivial(wallhug_path *path, wallhug_waypoint cur, u32 limit);
u32 wallhug_trivial(wallhug_path *path);

wallhug_waypoint wallhug_dirn_steps[] = {{0, 0xff}, {1, 0}, {0, 1}, {0xff, 0}};
extern s32 dx;
extern s32 dy;
extern s32 bresval;
extern u32 y_dirn;
extern u32 x_dirn;
extern u32 x_longer;
extern u32 wallhug_current_count;
extern u8 wallhug_last_hugstart;
extern u8 wallhug_last_handed;
extern u8 wallhug_last_dirn;
extern u32 wallhug_last_hug_count;
extern u32 wallhug_looking_for_last;

extern "C" {
u32 wallhug_add_huggers_path(wallhug_path *path, wallhug_hugger *hugger);
void wallhug_hugstep(wallhug_hugger *hugger);
}

// Starts stepping a line from `from` to `to` in orthogonal unit steps
// (Bresenham, in the globals dx, dy, bresval, x_dirn, y_dirn, x_longer).
// FUNCTION: 0x00048960
void bresenham_start(wallhug_waypoint from, wallhug_waypoint to)
{
    s32 ddx;
    s32 ddy;

    ddx = to.x;
    ddx -= from.x;
    ddy = to.y;
    ddy -= from.y;
    if (abs(ddx) > abs(ddy))
        x_longer = 1;
    else
        x_longer = 0;
    if (ddx > 0)
        x_dirn = 1;
    else
        x_dirn = 3;
    if (ddy > 0)
        y_dirn = 2;
    else
        y_dirn = 0;
    if (from.x == to.x)
        x_dirn = y_dirn;
    if (from.y == to.y)
        y_dirn = x_dirn;
    dx = abs(ddx);
    dy = abs(ddy);
    bresval = dx / 2 + dy / 2;
}

// Returns the direction (0-3, see wall_in_way) of the line's next step.
// FUNCTION: 0x00048a6c
u32 bresenham(void)
{
    if (bresval >= dx) {
        bresval -= dx;
        return y_dirn;
    } else {
        bresval += dy;
        return x_dirn;
    }
}

// Returns 1 if the stepped line from `from` to `to` crosses no wall, else 0.
// FUNCTION: 0x00048abc
u32 line_of_sight(wallhug_waypoint from, wallhug_waypoint to)
{
    wallhug_waypoint cur;
    u32 dir;

    cur = from;
    bresenham_start(from, to);
    while (cur.x != to.x || cur.y != to.y) {
        dir = bresenham();
        if (wall_in_way(cur.x, cur.y, dir))
            return 0;
        if (dir == 0 || dir == 1 || dir == 2 || dir == 3) {
        }
        cur.x += wallhug_dirn_steps[dir].x;
        cur.y += wallhug_dirn_steps[dir].y;
    }
    return 1;
}

// Shortens path from waypoint first on: drops waypoints that can be skipped
// (up to four ahead) by a clear line of sight, until none can. Returns 1 if
// anything was dropped.
// FUNCTION: 0x00048b64
u32 line_of_sight_cleanup(wallhug_path *path, u32 first)
{
    u32 progress;
    u32 changed;
    u32 j;
    u32 i;
    wallhug_waypoint cur;

    changed = 0;
    do {
        i = first;
        j = i;
        if (first == 0)
            cur = path->start;
        else
            cur = path->route[first - 1];
        progress = 0;
        while (path->count > j) {
            u32 k;

            for (k = 4; k > 0; k--) {
                if (j + k < path->count && line_of_sight(cur, path->route[j + k])) {
                    j += k;
                    progress = 1;
                    changed = 1;
                    goto next;
                }
            }
            cur = path->route[i] = path->route[j];
            i++;
            j++;
        next:;
        }
        path->count = i;
    } while (progress);
    return changed;
}

// Smooths path: removes skippable waypoints, then replaces a waypoint with
// the route wallhug_trivial finds between its neighbours when that route does
// not pass through it, and starts again (up to ten rounds). Returns result
// unchanged.
// FUNCTION: 0x00048c78
u32 wallhug_cleanup(wallhug_path *path, u32 result)
{
    u32 k;
    wallhug_path sub;
    u32 tries;
    u32 unused18;
    u32 unused14;
    u32 from;
    u32 unusedc;

    tries = 10;
    from = 0;
restart:
    line_of_sight_cleanup(path, from);
    if (--tries == 0)
        return result;
    {
        u32 i;

        for (i = 0; i + 1 < path->count; i++) {
            if (i == 0)
                sub.start = path->start;
            else
                sub.start = path->route[i - 1];
            sub.end = path->route[i + 1];
            if (wallhug_trivial(&sub) == 0x70000000)
                continue;
            for (k = 0; k + 1 < sub.count; k++) {
                if (sub.route[k].x == path->route[i].x && sub.route[k].y == path->route[i].y)
                    goto next;
            }
            if (sub.count + path->count - 2 > 0xfc)
                continue;
            memmove(path->route + i + sub.count, path->route + i + 2, (path->count - i) * 2 - 4);
            lib_memcpy(path->route + i, sub.route, sub.count * 2);
            path->count = sub.count + path->count - 2;
            if (i < 4)
                from = 0;
            else
                from = i - 4;
            goto restart;
        next:;
        }
    }
    return result;
}

// Moves wall-following hugger one step along the wall on its side (turn),
// recording its corners. Sets dir 0x32 once it is back on the line to its
// end past where it hit the wall, 0x33 if its route is full.
// FUNCTION: 0x00049400 COMDAT
inline void wallhug_hugstep(wallhug_hugger *hugger)
{
    u32 dir;

    if (hugger->dir == 0x33)
        return;
    if (wall_in_way(hugger->pos.x, hugger->pos.y, hugger->dir)) {
        hugger->dir = (-hugger->turn + hugger->dir) & 3;
        hugger->route[hugger->count] = hugger->pos;
        hugger->count++;
        if (hugger->count > 0xfb) {
            hugger->dir = 0x33;
            return;
        }
    } else {
        if (hugger->dir == 0 || hugger->dir == 1 || hugger->dir == 2 || hugger->dir == 3) {
        }
        hugger->pos.x += wallhug_dirn_steps[hugger->dir].x;
        hugger->pos.y += wallhug_dirn_steps[hugger->dir].y;
        dir = (hugger->dir + hugger->turn) & 3;
        if (!wall_in_way(hugger->pos.x, hugger->pos.y, dir)) {
            hugger->dir = dir;
            hugger->route[hugger->count] = hugger->pos;
            hugger->count++;
            if (hugger->count > 0xfb) {
                hugger->dir = 0x33;
                return;
            }
        }
    }
    switch ((hugger->dir + hugger->turn) & 3) {
    case 0:
        if (hugger->end.y < hugger->pos.y)
            return;
        break;
    case 1:
        if (hugger->end.x > hugger->pos.x)
            return;
        break;
    case 2:
        if (hugger->end.y > hugger->pos.y)
            return;
        break;
    case 3:
        if (hugger->end.x < hugger->pos.x)
            return;
        break;
    }
    switch (hugger->dir) {
    case 0:
        if (hugger->end.y > hugger->pos.y)
            return;
        break;
    case 1:
        if (hugger->end.x < hugger->pos.x)
            return;
        break;
    case 2:
        if (hugger->end.y < hugger->pos.y)
            return;
        break;
    case 3:
        if (hugger->end.x > hugger->pos.x)
            return;
        break;
    }
    if (hugger->hit.x <= hugger->end.x) {
        if (hugger->pos.x < hugger->hit.x)
            return;
        if (hugger->pos.x > hugger->end.x)
            return;
    }
    if (hugger->hit.x >= hugger->end.x) {
        if (hugger->pos.x > hugger->hit.x)
            return;
        if (hugger->pos.x < hugger->end.x)
            return;
    }
    if (hugger->hit.y <= hugger->end.y) {
        if (hugger->pos.y < hugger->hit.y)
            return;
        if (hugger->pos.y > hugger->end.y)
            return;
    }
    if (hugger->hit.y >= hugger->end.y) {
        if (hugger->pos.y > hugger->hit.y)
            return;
        if (hugger->pos.y < hugger->end.y)
            return;
    }
    if (hugger->hit.x == hugger->pos.x && hugger->hit.y == hugger->pos.y)
        return;
    hugger->dir = 0x32;
}

// Finds a path from path->start to path->end by walking straight and
// following walls around obstacles, then smooths it. Returns the steps
// taken, or 0x70000000 if no path was found.
// FUNCTION: 0x00048e6c
u32 wallhug_tricky(wallhug_path *path)
{
    u32 result;

    result = wallhug_trivial(path);
    return wallhug_cleanup(path, result);
}

// Returns 1 if the two wall huggers have met face to face (the obstacle is
// enclosed), else 0.
// FUNCTION: 0x000492f0 COMDAT
inline u32 huggers_met_again(wallhug_info *info)
{
    wallhug_waypoint next;

    if (info->hugger[0].dir == 0x33 || info->hugger[0].dir == 0x32)
        return 0;
    next = info->hugger[0].pos;
    if (info->hugger[0].dir == 0 || info->hugger[0].dir == 1 || info->hugger[0].dir == 2 || info->hugger[0].dir == 3) {
    }
    next.x += wallhug_dirn_steps[info->hugger[0].dir].x;
    next.y += wallhug_dirn_steps[info->hugger[0].dir].y;
    if (!((info->hugger[1].pos.x - next.x) & 0xff) && !((info->hugger[1].pos.y - next.y) & 0xff) &&
        ((info->hugger[0].dir + 2) & 3) == info->hugger[1].dir)
        return 1;
    return 0;
}

// Appends hugger's corners and position to path. Returns 0 if they do not
// fit, else 1.
// FUNCTION: 0x00049230 COMDAT
inline u32 wallhug_add_huggers_path(wallhug_path *path, wallhug_hugger *hugger)
{
    u32 i;

    if (hugger->count + path->count + 2 > 0xfc)
        return 0;
    for (i = 0; i < hugger->count; i++) {
        path->route[path->count] = hugger->route[i];
        path->count++;
    }
    path->route[path->count] = hugger->pos;
    path->count++;
    return 1;
}

// Continues path from cur towards path->end, sending a left and a right
// wall hugger around each wall met and keeping the first to get back on
// course, for at most limit steps in all (wallhug_current_count). Returns
// the step count on arriving, 0x70000000 on failure.
// FUNCTION: 0x00048ea4
u32 wallhug_continue_trivial(wallhug_path *path, wallhug_waypoint cur, u32 limit)
{
    wallhug_waypoint from;
    u32 dir;
    wallhug_info info;

    from = cur;
    bresenham_start(from, path->end);
    for (; wallhug_current_count < limit && (cur.x != path->end.x || cur.y != path->end.y); wallhug_current_count++) {
        dir = bresenham();
        if (wall_in_way(cur.x, cur.y, dir)) {
            if (wallhug_looking_for_last) {
                wallhug_last_hugstart = path->count;
                wallhug_last_hug_count = wallhug_current_count;
                wallhug_last_dirn = dir;
            }
            path->route[path->count] = cur;
            path->count++;
            info.hugger[0].pos = cur;
            info.hugger[1].pos = cur;
            info.hugger[0].hit = cur;
            info.hugger[1].hit = cur;
            info.hugger[0].turn = -1;
            info.hugger[1].turn = 1;
            info.hugger[0].dir = (dir + 1) & 3;
            info.hugger[1].dir = (dir - 1) & 3;
            info.hugger[0].start = from;
            info.hugger[1].start = from;
            info.hugger[0].end = path->end;
            info.hugger[1].end = path->end;
            info.hugger[0].count = 0;
            info.hugger[1].count = 0;
            while (wallhug_current_count < limit) {
                wallhug_hugstep(&info.hugger[0]);
                if (huggers_met_again(&info))
                    goto fail;
                wallhug_hugstep(&info.hugger[1]);
                if (huggers_met_again(&info))
                    goto fail;
                if (info.hugger[0].dir == 0x32) {
                    if (!wallhug_add_huggers_path(path, &info.hugger[0]))
                        goto fail;
                    if (wallhug_looking_for_last)
                        wallhug_last_handed = 0xff;
                    break;
                }
                if (info.hugger[1].dir == 0x32) {
                    if (!wallhug_add_huggers_path(path, &info.hugger[1]))
                        goto fail;
                    if (wallhug_looking_for_last)
                        wallhug_last_handed = 1;
                    break;
                }
                if (info.hugger[0].dir == 0x33 && info.hugger[1].dir == 0x33)
                    goto fail;
                wallhug_current_count++;
            }
            if (wallhug_current_count == limit)
                goto fail;
            line_of_sight_cleanup(path, 0);
            cur = path->route[path->count - 1];
            from = cur;
            bresenham_start(from, path->end);
        } else {
            if (dir == 0 || dir == 1 || dir == 2 || dir == 3) {
            }
            cur.x += wallhug_dirn_steps[dir].x;
            cur.y += wallhug_dirn_steps[dir].y;
        }
    }
    if (path->count >= 0xfa)
        return 0x70000000;
    if (cur.x == path->end.x && cur.y == path->end.y) {
        path->route[path->count] = cur;
        path->count++;
        return wallhug_current_count;
    }
    return 0x70000000;
fail:
    path->route[path->count] = cur;
    path->count++;
    return 0x70000000;
}

// Finds path from path->start to path->end by walking and wall hugging
// (up to 60000 steps), without smoothing. Returns as
// wallhug_continue_trivial.
// FUNCTION: 0x000491d8
u32 wallhug_trivial(wallhug_path *path)
{
    path->count = 0;
    wallhug_current_count = 0;
    if (wallhug_looking_for_last)
        wallhug_last_hugstart = 0xff;
    return wallhug_continue_trivial(path, path->start, 60000);
}
