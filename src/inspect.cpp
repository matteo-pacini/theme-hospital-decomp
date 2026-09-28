#include "hosp.h"

extern "C" {
extern char D_00121ae0[];
extern u16 D_0012ddae;
extern u16 D_0012ddb0;
extern u16 D_0012ddb2;
}

u16 create_inspector(u8 owner);
void send_peeps_home(u8 all, u8 owner);

// NONMATCHING: register choice
// FUNCTION: 0x000a1c10
u16 create_inspector(u8 owner)
{
    Thing *w;
    u16 pos;
    u16 f;

    w = allocate_thing(0x19);
    if (w != 0) {
    Thing *obj;
    w->renderer = 0x16;
    w->owner = owner;
    pos = room_info[allhosps[owner].unknown1cf[old_newrand() & 3]].unknown4;
    w->x.tile = pos % 128;
    w->y.tile = pos / 128;
    w->x.frac = 0x80;
    w->y.frac = 0x80;
    w->unknown18 = 0x20;
    w->anim_speed = 1;
    w->unknown17 = 1;
    insert_node(w, w->x, w->y);
    f = check_frame(0x89, w) + w->unknown17;
    w->sprite = start[f].frame;
    w->palette = start[f].flags > 0 ? (u8)1 : (u8)0;
    obj = &thing[listhead[owner].objects[11]];
    if (obj > thing) {
        fn_0004195c(w, obj->use_pos[1]);
        w->kind = 2;
        w->f6e = 8;
        w->f6c = 0;
    } else {
        fn_0004195c(w, room_info[allhosps[w->owner].unknown1cf[old_newrand() & 3]].unknown4);
        w->kind = 2;
        w->f6e = 6;
    }
    return w - thing;
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x000a1e3c
void generate_inspector(u8 owner)
{
    if (allhosps[owner].unknown209[1] != 0)
        return;
    if (allhosps[owner].f201 = create_inspector(owner)) {}
    LbDataLoadSetModifyFilenameFunction(debug.unknown116, D_00121ae0);
    debug.unknown1de = 0xc8;
    allhosps[owner].unknown209[1] = 3;
}

// NONMATCHING: register choice
// FUNCTION: 0x000a1ed4
void inspector_pauses(Thing *w)
{
    AllHosps *h;
    u32 n;

    if (--w->timer != 0)
        return;
    h = &allhosps[w->owner];
    fn_0004195c(w, room_info[h->unknown1cf[old_newrand() & 3]].unknown4);
    w->kind = 2;
    w->f6e = 6;
    n = count_and_flag_contagious_peeps(h->f20b, 2, w->owner);
    if (n == 0) {
        h->unknown209[0] = 0;
    } else if (n <= 2) {
        if (old_newrand() % 5 == 0)
            h->unknown209[0] = 0;
        else
            h->unknown209[0] = 1;
    } else if (n <= 5) {
        if (old_newrand() % 20 == 0)
            h->unknown209[0] = 0;
        else if (old_newrand() % 10 == 0)
            h->unknown209[0] = 1;
        else
            h->unknown209[0] = 2;
    } else if (n < 10) {
        if (old_newrand() % 50 == 0)
            h->unknown209[0] = 0;
        else if (old_newrand() % 25 == 0)
            h->unknown209[0] = 1;
        else
            h->unknown209[0] = 2;
    } else {
        if (old_newrand() % 50 == 0)
            h->unknown209[0] = 0;
        else
            h->unknown209[0] = 3;
    }
    h->f203 = D_0012ddae * n * 2;
    make_a_playercall(5, 3, 0x14, w->owner);
    h->f20c = h->f20b;
    h->f20b = 0;
    h->unknown209[1] = 0;
    h->f20e = 0;
    switch (h->unknown209[0]) {
    case 0:
        h->f203 = old_normal_rand(D_0012ddb0, D_0012ddb2);
        change_balance(0x1f, h->f203, 0, (u16)w->owner);
        break;
    case 1:
        change_balance(0x1e, -h->f203, 0, (u16)w->owner);
        send_peeps_home(0, w->owner);
        break;
    case 2:
        reputation_affected(0, 0xf, 0, w->owner, 0);
        change_balance(0x1e, -h->f203, 0, (u16)w->owner);
        send_peeps_home(0, w->owner);
        break;
    case 3:
        reputation_affected(0, 0xf, 0, w->owner, 0);
        send_peeps_home(1, w->owner);
        break;
    }
}
