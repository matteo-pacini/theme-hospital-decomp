#include "hosp.h"

s32 wall_in_way(u32 x, u32 y, u32 dir);
void bresenham_start(wallhug_waypoint from, wallhug_waypoint to);
u32 bresenham(void);
u32 line_of_sight(wallhug_waypoint from, wallhug_waypoint to);
u32 wallhug_cleanup(wallhug_path *path, u32 result);
u32 huggers_met_again(wallhug_info *info);

extern wallhug_waypoint wallhug_dirn_steps[];
extern s32 dx;
extern s32 dy;
extern s32 bresval;
extern u32 y_dirn;
extern u32 x_dirn;
extern u32 x_longer;

extern "C" {
extern u32 D_001603c0;
extern u8 D_001603c4;
extern u8 D_001603c5;
extern u8 D_001603c6;
extern u32 D_001603c8;
extern s32 D_001603cc;

u32 fn_00048b64(wallhug_path *path, u32 first);
u32 fn_00048ea4(wallhug_path *path, wallhug_waypoint cur, u32 limit);
u32 fn_000491d8(wallhug_path *path);
u32 fn_00049230(wallhug_path *path, wallhug_hugger *hugger);
void fn_00049400(wallhug_hugger *hugger);
}

// FUNCTION: 0x00048960
void bresenham_start(wallhug_waypoint from, wallhug_waypoint to)
{
    s32 ddx;
    s32 ddy;

    ddx = to.x;
    ddx -= from.x;
    ddy = to.y;
    ddy -= from.y;
    if (fn_000cb2db(ddx) > fn_000cb2db(ddy))
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
    dx = fn_000cb2db(ddx);
    dy = fn_000cb2db(ddy);
    bresval = dx / 2 + dy / 2;
}

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
        if (dir == 0 || dir == 1 || dir == 2 || dir == 3) {}
        cur.x += wallhug_dirn_steps[dir].x;
        cur.y += wallhug_dirn_steps[dir].y;
    }
    return 1;
}

// FUNCTION: 0x00048b64
u32 fn_00048b64(wallhug_path *path, u32 first)
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
    fn_00048b64(path, from);
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
            if (fn_000491d8(&sub) == 0x70000000)
                continue;
            for (k = 0; k + 1 < sub.count; k++) {
                if (sub.route[k].x == path->route[i].x && sub.route[k].y == path->route[i].y)
                    goto next;
            }
            if (sub.count + path->count - 2 > 0xfc)
                continue;
            memmove(path->route + i + sub.count, path->route + i + 2, (path->count - i) * 2 - 4);
            LbDrawBoxCoords(path->route + i, sub.route, sub.count * 2);
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

// FUNCTION: 0x00048e6c
u32 wallhug_tricky(wallhug_path *path)
{
    u32 result;

    result = fn_000491d8(path);
    return wallhug_cleanup(path, result);
}

// FUNCTION: 0x00048ea4
u32 fn_00048ea4(wallhug_path *path, wallhug_waypoint cur, u32 limit)
{
    wallhug_waypoint from;
    u32 dir;
    wallhug_info info;

    from = cur;
    bresenham_start(from, path->end);
    for (; D_001603c0 < limit && (cur.x != path->end.x || cur.y != path->end.y); D_001603c0++) {
        dir = bresenham();
        if (wall_in_way(cur.x, cur.y, dir)) {
            if (D_001603cc) {
                D_001603c4 = path->count;
                D_001603c8 = D_001603c0;
                D_001603c6 = dir;
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
            while (D_001603c0 < limit) {
                fn_00049400(&info.hugger[0]);
                if (huggers_met_again(&info))
                    goto fail;
                fn_00049400(&info.hugger[1]);
                if (huggers_met_again(&info))
                    goto fail;
                if (info.hugger[0].dir == 0x32) {
                    if (!fn_00049230(path, &info.hugger[0]))
                        goto fail;
                    if (D_001603cc)
                        D_001603c5 = 0xff;
                    break;
                }
                if (info.hugger[1].dir == 0x32) {
                    if (!fn_00049230(path, &info.hugger[1]))
                        goto fail;
                    if (D_001603cc)
                        D_001603c5 = 1;
                    break;
                }
                if (info.hugger[0].dir == 0x33 && info.hugger[1].dir == 0x33)
                    goto fail;
                D_001603c0++;
            }
            if (D_001603c0 == limit)
                goto fail;
            fn_00048b64(path, 0);
            cur = path->route[path->count - 1];
            from = cur;
            bresenham_start(from, path->end);
        } else {
            if (dir == 0 || dir == 1 || dir == 2 || dir == 3) {}
            cur.x += wallhug_dirn_steps[dir].x;
            cur.y += wallhug_dirn_steps[dir].y;
        }
    }
    if (path->count >= 0xfa)
        return 0x70000000;
    if (cur.x == path->end.x && cur.y == path->end.y) {
        path->route[path->count] = cur;
        path->count++;
        return D_001603c0;
    }
    return 0x70000000;
fail:
    path->route[path->count] = cur;
    path->count++;
    return 0x70000000;
}

// FUNCTION: 0x000491d8
u32 fn_000491d8(wallhug_path *path)
{
    path->count = 0;
    D_001603c0 = 0;
    if (D_001603cc)
        D_001603c4 = 0xff;
    return fn_00048ea4(path, path->start, 60000);
}

// FUNCTION: 0x00049230
u32 fn_00049230(wallhug_path *path, wallhug_hugger *hugger)
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

// FUNCTION: 0x000492f0
u32 huggers_met_again(wallhug_info *info)
{
    wallhug_waypoint next;

    if (info->hugger[0].dir == 0x33 || info->hugger[0].dir == 0x32)
        return 0;
    next = info->hugger[0].pos;
    if (info->hugger[0].dir == 0 || info->hugger[0].dir == 1 || info->hugger[0].dir == 2
        || info->hugger[0].dir == 3) {}
    next.x += wallhug_dirn_steps[info->hugger[0].dir].x;
    next.y += wallhug_dirn_steps[info->hugger[0].dir].y;
    if (!((info->hugger[1].pos.x - next.x) & 0xff) && !((info->hugger[1].pos.y - next.y) & 0xff)
        && ((info->hugger[0].dir + 2) & 3) == info->hugger[1].dir)
        return 1;
    return 0;
}

// FUNCTION: 0x00049400
void fn_00049400(wallhug_hugger *hugger)
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
        if (hugger->dir == 0 || hugger->dir == 1 || hugger->dir == 2 || hugger->dir == 3) {}
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
