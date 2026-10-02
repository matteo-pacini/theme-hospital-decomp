#include "hosp.h"

s32 wall_in_way(u32 x, u32 y, u32 dir);
s32 person_in_way(u32 x, u32 y, u32 dir);
s32 person_overlap(void);

// Drops the path's first waypoint, which becomes its start (x0, y0).
// FUNCTION: 0x000553e0
void delete_first_path_entry(orthonav_path *path)
{
    s32 i;

    path->x0 = path->steps[0].x;
    path->y0 = path->steps[0].y;
    path->count--;
    for (i = 0; i < path->count; i++)
        path->steps[i] = path->steps[i + 1];
}

// Plans a path from (x0, y0) to (x1, y1) with wallhug_tricky, keeping its
// first four waypoints; path->count is the waypoint count, 0 if no route was
// found.
// FUNCTION: 0x00055454
void orthonav_init(orthonav_path *path, s8 x0, s8 y0, s8 x1, s8 y1)
{
    s32 i;
    wallhug_path hug;

    hug.x0 = x0;
    hug.y0 = y0;
    hug.x1 = x1;
    hug.y1 = y1;
    if (wallhug_tricky(&hug) == 0x70000000) {
        path->count = 0;
        return;
    }
    if (hug.count > 4)
        path->count = 4;
    else
        path->count = hug.count;
    path->x0 = x0;
    path->y0 = y0;
    path->x1 = x1;
    path->y1 = y1;
    for (i = 0; i < path->count; i++) {
        path->steps[i].x = hug.points[i].x;
        path->steps[i].y = hug.points[i].y;
    }
}

// Returns 1 if a person can walk the straight row or column from (x1, y1) to
// (x2, y2) without crossing a wall, else 0. The points must share x or y.
// FUNCTION: 0x00055568
s32 free_orthog_line(s32 x1, s32 y1, s32 x2, s32 y2)
{
    if (x1 == x2) {
        if (y2 < y1) {
            y2 ^= y1;
            y1 ^= y2;
            y2 ^= y1;
        }
        for (; y1 < y2; y1++) {
            if (wall_in_way(x1, y1, 2))
                return 0;
        }
    } else {
        if (x2 < x1) {
            x2 ^= x1;
            x1 ^= x2;
            x2 ^= x1;
        }
        for (; x1 < x2; x1++) {
            if (wall_in_way(x1, y1, 1))
                return 0;
        }
    }
    return 1;
}

extern s32 other_wherefacing;

// Chooses the direction (0-3, see wall_in_way) to step from (x, y) towards
// the path's next waypoint, given the current facing dir, with some
// randomness. Returns 4 on reaching the last waypoint. Consumes reached
// waypoints and leaves the second-best direction in other_wherefacing.
// FUNCTION: 0x0005562c
s32 orthonav_wn(orthonav_path *path, s32 x, s32 y, s32 dir)
{
    s32 ldx;
    s32 ldy;
    s32 fx;
    s32 fy;
    s32 dx;
    s32 dy;
    s32 nx;
    s32 ny;
    s32 xdir;
    s32 ydir;
    s32 deleted;

    deleted = 0;
again:
    if (path->steps[0].x == x && path->steps[0].y == y) {
        if (path->count == 1)
            return 4;
        delete_first_path_entry(path);
        deleted = 1;
    }
    ldx = (path->steps[0].x - path->x0) << 8;
    ldy = (path->steps[0].y - path->y0) << 8;
    dx = (path->steps[0].x - x) << 8;
    dy = (path->steps[0].y - y) << 8;
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
        switch (dir) {
        case 1:
        case 3:
            dir = ydir;
            other_wherefacing = xdir;
            break;
        default:
            dir = xdir;
            other_wherefacing = ydir;
            break;
        }
    } else if (dir == xdir) {
        other_wherefacing = ydir;
    } else {
        other_wherefacing = xdir;
    }
    if (path->count > 1) {
        if (!deleted && !wall_in_way(x, y, dir) &&
            ((path->steps[0].x == x && dir == xdir) || (path->steps[0].y == y && dir == ydir))) {
            s32 ex;
            s32 ey;

            ex = path->steps[1].x - x;
            ey = path->steps[1].y - y;
            switch (dir) {
            case 0:
                if (ey > 0)
                    goto keep;
                break;
            case 2:
                if (ey < 0)
                    goto keep;
                break;
            case 3:
                if (ex > 0)
                    goto keep;
                break;
            case 1:
                if (ex < 0)
                    goto keep;
                break;
            }
            delete_first_path_entry(path);
            deleted = 1;
            goto again;
        }
    keep:
        if ((path->steps[0].x == x && dir == ydir) || (path->steps[0].y == y && dir == xdir))
            return dir;
    } else {
        if (path->steps[0].x == x)
            return path->steps[0].y < y ? 0 : 2;
        if (path->steps[0].y == y)
            return path->steps[0].x < x ? 3 : 1;
    }
    if (ldx == 0 && dx == 0)
        return ydir;
    if (ldy == 0 && dy == 0)
        return xdir;
    if (wall_in_way(x, y, dir))
        return other_wherefacing;
retry:
    switch (dir) {
    case 3:
        nx = x - 1;
        ny = y;
        break;
    case 1:
        nx = x + 1;
        ny = y;
        break;
    case 0:
        nx = x;
        ny = y - 1;
        break;
    case 2:
        nx = x;
        ny = y + 1;
        break;
    }
    if ((s32)(old_newrand() % 0xffff) < 0xa000) {
        if (wall_in_way(nx, ny, dir)) {
            s32 old_dir;

            old_dir = dir;
            dir = other_wherefacing;
            other_wherefacing = old_dir;
            switch (dir) {
            case 3:
                nx = x - 1;
                ny = y;
                break;
            case 1:
                nx = x + 1;
                ny = y;
                break;
            case 0:
                nx = x;
                ny = y - 1;
                break;
            case 2:
                nx = x;
                ny = y + 1;
                break;
            }
        } else if ((s32)(old_newrand() % 0xffff) < 0x8000) {
            s32 nx2;
            s32 ny2;

            switch (dir) {
            case 3:
                nx2 = nx - 1;
                ny2 = ny;
                break;
            case 1:
                nx2 = nx + 1;
                ny2 = ny;
                break;
            case 0:
                nx2 = nx;
                ny2 = ny - 1;
                break;
            case 2:
                nx2 = nx;
                ny2 = ny + 1;
                break;
            }
            if (wall_in_way(nx2, ny2, dir)) {
                s32 old_dir;

                old_dir = dir;
                dir = other_wherefacing;
                other_wherefacing = old_dir;
                switch (dir) {
                case 3:
                    nx = x - 1;
                    ny = y;
                    break;
                case 1:
                    nx = x + 1;
                    ny = y;
                    break;
                case 0:
                    nx = x;
                    ny = y - 1;
                    break;
                case 2:
                    nx = x;
                    ny = y + 1;
                    break;
                }
            }
        }
    }
    switch (dir) {
    case 0:
        if (path->y0 > ny && path->steps[0].y > ny)
            return other_wherefacing;
        break;
    case 2:
        if (path->y0 < ny && path->steps[0].y < ny)
            return other_wherefacing;
        break;
    case 3:
        if (path->x0 > nx && path->steps[0].x > nx)
            return other_wherefacing;
        break;
    case 1:
        if (path->x0 < nx && path->steps[0].x < nx)
            return other_wherefacing;
        break;
    }
    if (dir == ydir) {
        if ((path->x0 > nx && ldx >= 0) || (path->x0 < nx && ldx <= 0))
            goto swap;
        if (ldx == 0)
            return dir;
        fy = (((nx - path->x0) << 8) * ldy + 0x80) / ldx + (path->y0 << 8) + 0x80;
        if ((ny << 8 < fy && dir == 0) || (ny << 8 > fy && dir == 2))
            goto swap;
        fx = nx << 8;
    } else {
        if ((path->y0 > ny && ldy >= 0) || (path->y0 < ny && ldy <= 0))
            goto swap;
        if (ldy == 0)
            return dir;
        fx = (((ny - path->y0) << 8) * ldx + 0x80) / ldy + (path->x0 << 8) + 0x80;
        if ((nx << 8 < fx && dir == 3) || (nx << 8 > fx && dir == 1))
            goto swap;
        fy = ny << 8;
    }
    if (free_orthog_line(nx, ny, fx >> 8, fy >> 8))
        return dir;
swap:
    if ((s32)(old_newrand() % 0xffff) > 0xe000) {
        dir ^= other_wherefacing;
        other_wherefacing ^= dir;
        dir ^= other_wherefacing;
        goto retry;
    }
    if (dir == xdir) {
        if ((path->x0 > nx && ldx > 0) || (path->x0 < nx && ldx < 0))
            goto other;
        if (ldx == 0)
            fy = ny << 8;
        else
            fy = (path->y0 << 8) + (((nx - path->x0) << 8) * ldy + 0x80) / ldx + 0x80;
        fx = nx << 8;
    } else {
        if ((path->y0 > ny && ldy > 0) || (path->y0 < ny && ldy < 0))
            goto other;
        if (ldy == 0)
            fx = nx << 8;
        else
            fx = (path->x0 << 8) + (((ny - path->y0) << 8) * ldx + 0x80) / ldy + 0x80;
        fy = ny << 8;
    }
    if (free_orthog_line(nx, ny, fx >> 8, fy >> 8))
        return dir;
other:
    return other_wherefacing;
}

// Returns the direction (0-3) of the next step from (x, y) along path, given
// the current facing dir, sidestepping walls and people in the way: 4 when
// arrived, -1 when blocked by people, -2 when walled in, -3 when there is no
// path. A detour, or being blocked by people or walls, sets path->x1 to
// 0xff, so the caller replans next time.
// FUNCTION: 0x00056028
s32 orthonav_whatnext(orthonav_path *path, s8 x, s8 y, s32 dir)
{
    s32 next_dir;
    s32 roll;
    s32 blocked;
    s32 alt_dir;

    if (path->count == 0) {
        if (wall_in_way(x, y, 0) && wall_in_way(x, y, 2) && wall_in_way(x, y, 3) && wall_in_way(x, y, 1))
            return -2;
        return -3;
    }
    next_dir = orthonav_wn(path, x, y, dir);
    if (next_dir == 4) {
        if (path->x1 != x || path->y1 != y) {
            orthonav_init(path, x, y, (s8)path->x1, (s8)path->y1);
            next_dir = orthonav_wn(path, x, y, dir);
        } else {
            return 4;
        }
    }
    if (wall_in_way(x, y, next_dir)) {
        orthonav_init(path, x, y, (s8)path->x1, (s8)path->y1);
        next_dir = orthonav_wn(path, x, y, dir);
        if (!wall_in_way(x, y, next_dir))
            goto unblocked;
        next_dir = -2;
        roll = old_newrand() % 0xffff + 0xffff;
        do {
            if (!wall_in_way(x, y, 0))
                next_dir = 0;
            if (path->steps[0].y <= y)
                roll -= 0x4000;
            if (roll < 0)
                break;
            if (!wall_in_way(x, y, 2))
                next_dir = 2;
            if (path->steps[0].y >= y)
                roll -= 0x4000;
            if (roll < 0)
                break;
            if (!wall_in_way(x, y, 3))
                next_dir = 3;
            if (path->steps[0].x <= x)
                roll -= 0x4000;
            if (roll < 0)
                break;
            if (!wall_in_way(x, y, 1))
                next_dir = 1;
            if (path->steps[0].x >= x)
                roll -= 0x4000;
        } while (roll >= 0);
        if (next_dir == -2) {
            path->x1 = 0xff;
            return -2;
        }
    } else {
    unblocked:
        blocked = person_in_way(x, y, next_dir) | person_overlap();
        if (blocked == 0)
            return next_dir;
        if (!wall_in_way(x, y, other_wherefacing) && !person_in_way(x, y, other_wherefacing) &&
            (s32)(old_newrand() % 0xffff) < blocked + 0x3000) {
            next_dir = other_wherefacing;
        } else {
            switch (other_wherefacing) {
            case 3:
                alt_dir = 1;
                break;
            case 1:
                alt_dir = 3;
                break;
            case 0:
                alt_dir = 2;
                break;
            case 2:
                alt_dir = 0;
                break;
            }
            if (!wall_in_way(x, y, alt_dir) && !person_in_way(x, y, alt_dir) &&
                (s32)(old_newrand() % 0xffff) < blocked + 0x3000) {
                next_dir = alt_dir;
            } else if ((s32)(old_newrand() % 0xffff) < blocked - 0x2000) {
                return next_dir;
            } else {
                switch (next_dir) {
                case 3:
                    alt_dir = 1;
                    break;
                case 1:
                    alt_dir = 3;
                    break;
                case 0:
                    alt_dir = 2;
                    break;
                case 2:
                    alt_dir = 0;
                    break;
                }
                if (!wall_in_way(x, y, alt_dir) && !person_in_way(x, y, alt_dir) && blocked + 0xffff > 0x8000) {
                    next_dir = alt_dir;
                } else {
                    path->x1 = 0xff;
                    return -1;
                }
            }
        }
    }
    path->x1 = 0xff;
    return next_dir;
}
