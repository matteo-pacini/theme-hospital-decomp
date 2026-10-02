#ifndef HOSP_H
#define HOSP_H

#include "bflib.h"
#include "enums.h"

/*
 * Game types, globals and functions whose real names are known from the 1997
 * demo. Their declarations must mangle exactly to the names in
 * config/functions.txt and config/symbols.txt, so parameter and variable
 * types follow the demo's debug info, including top-level const.
 */

union MapPos {
    u16 raw;
    struct {
        u8 frac;
        u8 tile;
    };
};

/*
 * One map tile (map[], 128 per row).
 */
struct Map {
    // 0x0: Head of the list of things on the tile (Thing.cell_next).
    s16 first;
    // 0x2: Overlay block id.
    u8 overlay;
    // 0x3: Block id of the wall on the tile's north edge, or 0.
    u8 wall_n;
    // 0x4: Block id of the wall on the tile's west edge, or 0.
    u8 wall_w;
    // 0x5: Tile flags.
    u8 flags;
    // 0x6: Floor block id; the low nibble is tested for floor type.
    u8 floor;
    // 0x7: Bit (1 << owner) set on land the hospital owns; 0x10 set on outdoor
    // tiles.
    u8 owner_flags;
};

struct Blocks {
    union {
        u32 data;
        TbSprite *info;
    };
    u8 width;
    s8 height;
};

struct RoomInfo {
    u8 type;
    s8 width;
    s8 height;
    // 0x3: <0 free slot, 0 normal, 1 staffed, 3 blueprint, 4 being edited, 5
    // machine awaiting repair, 6 broken.
    s8 state;
    u16 door_tile;
    u16 door_inside_tile;
    union {
        u16 pos;
        struct {
            u8 pos_lo;
            union {
                u8 unknown9[3];
                struct {
                    u8 unknown9_;
                    u16 queue_head;
                };
            };
        };
    };
    u16 first_object;
    u16 first;
    u16 staff[2];
    u16 count;
    u32 created_turn;
    u8 radiators;
    u8 windows;
    u8 owner;
    // 0x1d: Set when the staff member arrives or finishes a step with a
    // patient; cleared when they walk out.
    u8 staff_free;
    u8 queue_capacity;
    u8 queue_length;
    u8 queue_incoming;
    u8 paid_for;
    u8 being_edited;
};

/*
 * One entry of thing[] (things.cpp): the record shared by everything that
 * lives on the map. `type` says what the thing is and process_things runs
 * the state machine for that type on `state`.
 *
 * The layout is packed (175 bytes) and fixed by the original code. Many bytes
 * mean different things for different types, so they appear as unions with
 * one member per meaning. The comments name the types that use each member.
 * "People" means patients and staff, plus the mayor and inspector where they
 * share the walking code. Offsets are given in hex.
 */
struct Thing {
    // 0x00: links in the list of things on one map cell (Map.first).
    s16 cell_prev;
    s16 cell_next;
    MapPos x;
    MapPos y;
    // Current animation frame (frame[] index).
    u16 sprite;
    union {
        // Turns until the next step to a new tile; the open time of sliding
        // doors; a staff member's pause while wandering.
        s16 countdown;
        // Height above the floor while a patient floats away (person_flys).
        s16 fly_height;
    };
    // 0x0c: 0 = free entry, 2 patient, 4 staff, 6 object, 8 door, 0xa grim
    // reaper, 0xc rat, 0xe explosion debris, 0x10 helicopter, 0x12 floating
    // number, 0x14 mayor, 0x16 inspector, 0x18 shadow.
    u8 type;
    // Even state number. Each type has its own state machine (process_things).
    u8 state;
    // Patient, staff, or object kind (anim_starts, staff, or objects index).
    // For floating numbers, the text colour.
    u8 subtype;
    // The animation advances every anim_speed turns. Floating numbers use it
    // as their start height.
    u8 anim_speed;
    // Nonzero: the sprite is drawn mirrored (the frame's flags).
    u8 flip;
    // Direction the thing faces, 0-3 (around_xy, opposite_angle). 4 is used
    // while a person is stuck.
    u8 facing;
    // Distance moved per turn, in 1/256 tile. Floating numbers count it down
    // as their lifetime and rise with it.
    u8 speed;
    // Tile the thing is walking to.
    u8 dest_x;
    u8 dest_y;
    // Added to the draw-order bucket in do_mapwho.
    s8 depth;
    // Player index.
    u8 owner;
    // Sound of the last frame drawn, so that a looping frame plays it once.
    u8 last_sound;
    // 0x18: status icon over a person's head or a broken machine
    // (icon_table index). Negative until peep_icon announces it.
    s8 icon_id;
    // Turns left to show the icon. 0xffff keeps it.
    u16 icon_timer;
    u16 icon_frame;
    // 0x1d: the element id drawn for each of the 13 animation layers (head,
    // colours, ...). Objects set 10-12.
    u8 layers[13];
    union {
        // 0x2a: the first 13 bytes of the thing's orthonav_path (see
        // THING_PATH in nav.cpp). The struct's last 3 bytes lie over the
        // next field.
        u8 path[13];
        // The same path as rats read it: the next two waypoints and the
        // number of waypoints.
        struct {
            u8 pad2a_[4];
            union {
                u16 path_step0;
                struct {
                    u8 path_step0_x;
                    u8 path_step0_y;
                };
            };
            union {
                u16 path_step1;
                struct {
                    u8 path_step1_x;
                    u8 path_step1_y;
                };
            };
            u8 pad32_[4];
            u8 path_step_count;
        };
    };
    union {
        // 0x37, people: the room they are in.
        RoomInfo *room;
        // People in a save file: room as an index into room_info.
        s32 room_index;
        // Machines: uses since the last repair (compared with strength).
        s32 times_used;
        // Floating numbers: 0 number, 1 text, 2 formatted number, 3 money.
        s32 numfx_type;
        // Rats: turns left before the next waypoint.
        s32 rat_ticks;
        // Shadows: the thing that casts the shadow.
        u16 caster;
        struct {
            // Explosion debris and helicopters: height above the floor.
            s16 altitude;
            union {
                // Debris: x velocity.
                s16 vel_x;
                // Helicopters: their shadow thing.
                s16 shadow;
            };
        };
    };
    union {
        // 0x3b, machines: uses before a breakdown. A repair lowers it.
        s32 strength;
        // Floating numbers: the value shown.
        s32 numfx_amount;
        // Rats: x step per turn in 1/65536 tile.
        s32 rat_dx;
        struct {
            // People: turns to wait in the current state.
            s16 timer;
            // People: position in the current complex's action list.
            u16 complex_pc;
        };
        struct {
            // Debris: y and height velocities.
            s16 vel_y;
            s16 vel_z;
        };
    };
    union {
        // 0x3f, floating numbers: the text (never set; see mapwho_numfx).
        char numfx_text[4];
        // Rats: y step per turn in 1/65536 tile.
        s32 rat_dy;
        // Debris: the room of the explosion (room_info index).
        u16 bang_room;
        struct {
            // People: the complex queued behind the current one (set_complex).
            u16 complex_queued;
            // People: complex_pc to resume at (set_complex).
            u16 complex_resume;
        };
        struct {
            // Objects: where a person stands while using the object.
            MapPos use_x;
            MapPos use_y;
        };
    };
    union {
        // 0x43, people: the complex action in progress (set_complex).
        u16 complex_action;
        // Objects: frame of an animation drawn on top (machine smoke).
        u16 overlay_frame;
    };
    union {
        // 0x45, people: the object (or bed, desk, gate) they use or go to.
        u16 object;
        // Objects: the previous object in the room's list (first_object).
        u16 room_obj_prev;
    };
    union {
        // 0x47, people: the tile to step to after using an object or door.
        u16 exit_tile;
        // Objects: the next object in the room's list.
        u16 room_obj_next;
    };
    union {
        // 0x49, staff: the person they deal with (a patient, an interaction
        // partner). The grim reaper: its victim. Patients: the reaper coming
        // for them.
        u16 slave;
        // Objects: the previous object of the same kind in
        // listhead[owner].objects.
        u16 type_obj_prev;
    };
    union {
        // 0x4b, people: the second queued complex.
        u16 complex_queued2;
        // Objects: the next object of the same kind.
        u16 type_obj_next;
    };
    union {
        // 0x4d, objects: tiles a person uses the object from (0-2), and
        // the tiles they leave to afterwards (3-5).
        u16 use_pos[6];
        struct {
            // People: links in the room's list of people (RoomInfo.first).
            u16 room_prev;
            u16 room_next;
            // People: 0-100.
            union {
                u16 happiness;
                s16 happiness_s;
            };
            // People: how warm they feel, 0-100.
            u16 warmth;
            // People: sprite to restore after a reaction animation.
            u16 saved_sprite;
            // People: game time (othervar.cur_turn) of the next modify_person or
            // modify_staff.
            u16 modify_time;
        };
    };
    union {
        // 0x59, people: the room (room_info index) they are heading for.
        u16 dest_room;
        // Objects and doors: the room they belong to.
        u16 room_id;
    };
    union {
        // 0x5b, people: their pending player call (make_a_playercall).
        u16 playercall;
        // Objects: the person using it.
        u16 user;
    };
    // People: repeats left of the current complex action.
    u8 complex_repeat;
    union {
        // 0x5e, people: state to go back to when a complex fails or is
        // interrupted.
        u8 resume_state;
        struct {
            u8 pad5e_a_;
            // Objects: the other half of a two-part object.
            u16 pair;
        };
        struct {
            u8 pad5e_b_;
            // People: argument tile of complex_queued.
            u8 queued_arg_x;
            u8 queued_arg_y;
        };
    };
    union {
        // 0x61, people: argument tile of complex_queued2.
        u8 queued2_arg_x;
        // Objects: how people use it (objects[].use_style[0]).
        u8 use_style;
    };
    union {
        u8 queued2_arg_y;
        // Objects: 1 while someone uses or has claimed it.
        u8 in_use;
    };
    // Step in an animation sequence (aniseq) or interaction.
    u8 seq_index;
    union {
        // 0x64: 68 bytes whose meaning depends on the type.
        char data[0x44];
        // People and staff.
        struct {
            // 0 none, 1 playing, 2 finished (interact.cpp).
            u8 interact_anim;
            // Repeats left of the interaction's current frame.
            s8 interact_repeat;
            // State to enter on arriving or after the current action.
            u8 next_state;
            // Step within the current state. During a reaction animation
            // (PS_SHOWANIM, setup_showanim): the state to return to.
            u8 substate;
            // While stuck or reacting: what to restore afterwards.
            u8 saved_flip;
            u8 saved_state;
            u8 saved_dest_x;
            u8 saved_dest_y;
            union {
                // 0x6c, staff: wage (month_end).
                s32 salary;
                // Patients: the price of the treatment they are paying for.
                s32 fee;
            };
            union {
                // 0x70, patients: money they can spend.
                s32 cash;
                // Staff: the wage they ask for (appraise_staff).
                s32 wage_demand;
            };
            union {
                // 0x74, patients: status bits (contagious, vaccinated,
                // diagnosed, going home, ...).
                u32 flags32;
                // Staff: game turn of the next resignation check.
                u32 resign_time;
                struct {
                    union {
                        u16 flags;
                        struct {
                            u8 flags0;
                            u8 flags1;
                        };
                    };
                    u8 flags2;
                    u8 flags3;
                };
            };
            union {
                // 0x78, patients: links in a room's queue.
                u16 queue_prev;
                // Staff: 0-1000.
                s16 tiredness;
            };
            union {
                u16 queue_next;
                // Staff: calls left before giving up on a patient
                // (call_person_again).
                s16 call_retries;
            };
            union {
                // 0x7c, staff: skill bits (0x10 doctor, 0x20 surgeon,
                // 0x40 psychiatrist, 0x80 consultant, 0x100 researcher, ...).
                u16 skills;
                // Patients: grows while they wait or are unhappy; past
                // gbv.leave_max they go home.
                u16 impatience;
            };
            union {
                // 0x7e, staff: 30-1000.
                u16 ability;
                // Patients: turns waited since admission or treatment.
                u16 waiting_time;
            };
            union {
                // 0x80, staff: grows while unhappy; past gbv.resign_max they want
                // to resign.
                u16 discontent;
                s16 discontent_s;
                // Patients: grows near litter and after drinking; they then
                // drop litter.
                u16 litter_urge;
            };
            union {
                // 0x82, patients: 0-1000, dead at 0.
                s16 health;
                // Staff: game turn they were hired.
                s16 hire_time;
            };
            // 0x84, patients: health change not yet applied (change_health).
            s16 health_delta;
            // Patients: falls over time; below 30 they are thirsty.
            s16 refreshment;
            // Patients: rises over time; they need the toilet past gbv.bowel_full.
            s16 bladder;
            // Patients: rises near litter or dead plants and when too hot;
            // past gbv.vomit_limit they may vomit.
            u16 nausea;
            // 0x8c, patients: the room whose queue they are in.
            u16 queue_room;
            union {
                struct {
                    // 0x8e, staff: requests to act on (1 rest, 4 training,
                    // 0x10 research, 0x20 resignation pending, ...).
                    u8 orders;
                    // Staff: times they have asked to resign.
                    u8 resign_count;
                };
                // Patients: next in the hospital's vaccination queue.
                u16 vaccine_next;
            };
            // 0x90, patients: subtracted from diagnosis progress.
            u16 diagnosis_handicap;
            union {
                // 0x92, patients: diagnosis progress (done at gbv.diagnosis_complete).
                s16 diagnosis;
                struct {
                    u8 pad92_;
                    // Staff: a bit per player who has offered them a job.
                    u8 poach_offers;
                };
            };
            union {
                // 0x94, patients: treatments received, oldest first.
                u8 treatments[10];
                // Doctors: their assistant. Nurses: their bed or patient.
                // Handymen: the litter tile they head for.
                u16 work_target;
                // Job applicants: game turn they leave the list.
                u32 hire_expiry;
                struct {
                    u16 pad94_;
                    // Handymen: the next litter tile.
                    u16 work_target2;
                };
                struct {
                    u8 pad94b_[2];
                    // Doctors: they have cracked up.
                    u8 crazy;
                    // Doctors: training points towards each skill.
                    u8 surgeon_training;
                    u8 psych_training;
                    u8 research_training;
                };
            };
            // 0x9e, patients: health over time, in tenths.
            s8 health_history[10];
        };
        // Staff: identity shown in the staff windows.
        struct {
            u8 pad64a_[0x20];
            // 0x84: first initial, as a letter.
            u8 initial;
            // staff_names_1 and staff_names_2 indexes.
            u8 surname_a;
            u8 surname_b;
            // A Picture for draw_face.
            u8 face_head;
            u8 face_body;
            u8 face_legs;
            // ref_general_text, ref_good_text and ref_bad_text indexes.
            u8 ref_general;
            u8 ref_good;
            u8 ref_bad;
            // 0x8d: they work only on turns divisible by work_delay + 1.
            u8 work_delay;
            u8 pad8e_[2];
            // 0x90: 1 for nurses and receptionists, 0 for handymen, random
            // for doctors; presumably the gender. Only written.
            u8 female;
            u8 pad91_[0xd];
            // 0x9e, handymen: priorities for cleaning, watering, repairs.
            u8 prio_litter;
            u8 prio_plants;
            u8 prio_repair;
        };
        // Handymen: jobs done, set against the priorities.
        struct {
            u8 pad64b_[0x34];
            u16 litter_count;
            u16 plant_count;
            u16 repair_count;
        };
        // Mayor and inspector.
        struct {
            u8 pad64c_[8];
            // 0x6c: the room being inspected.
            u16 inspect_room;
            // State to enter on arrival.
            u8 goto_state;
        };
        // Grim reaper.
        struct {
            u8 pad64d_[8];
            // 0x6c: animation and state to switch to when the current
            // animation ends (reaper_play_anim).
            u16 next_sprite;
            u16 gate;
            u8 next_flip;
            u8 reaper_next_state;
        };
        // Objects.
        struct {
            union {
                // 0x64, machines: a second frame drawn with the user (see
                // draw_split_frame). Gates: frame to switch to when the
                // animation ends (object_hell).
                u16 aux_sprite;
                // The first half of a double door: the second half.
                u16 door_partner;
                // Plants: 400 when watered; dying below 0.
                s16 plant_health;
                // Bombs.
                struct {
                    u8 fuse;
                    u8 fuse_tick;
                };
                // Projectors: 1 once the consultant has started the lecture;
                // cleared when he leaves the training room.
                u8 lecture_started;
            };
            union {
                struct {
                    // 0x66: flip flag for aux_sprite.
                    u8 aux_flip;
                    // Machines: height at which draw_split_frame splits the
                    // user's sprite.
                    u8 split_height;
                    // Machines: parameter of their drawing effect. Doors:
                    // whether the cell had flag 0x10 before it was cleared.
                    u8 aux_param;
                    // Machines: draw flags for the user (0xff: not drawn).
                    u8 aux_draw_flags;
                };
                struct {
                    // Gates: aux_flip as a word.
                    u16 aux_flip_w;
                    // Gates: removed once the animation ends.
                    u16 remove_after_anim;
                };
                // The second half of a double door: the first half.
                u16 door_main;
            };
            u8 pad6a_[4];
            // 0x6e, machines: smoke puffs shown as they wear out.
            u8 smoke[3];
            // depth while a person uses the object.
            u8 saved_depth;
        };
    };
    // 0xa8, patients: their illness (expertise index).
    u8 illness;
    // The illness they have been diagnosed with (1 while unknown).
    u8 known_illness;
    // subtype before an illness or effect changed their appearance.
    u8 orig_subtype;
    // Expertise index of the room they are sent to next.
    u8 treatment;
    // Treatment that becomes `treatment` next (setup_next_treatment).
    u8 next_treatment;
    // Bit of their insurance company (allhosps.insurance_accepted), or 0.
    u8 insurance;
    // Contagiousness: above 0 contagious, 0xff vaccinated.
    u8 contagion;
};

struct HospitalDrug {
    u16 cost;
    u8 unknown2;
    u8 effectiveness;
};

/*
 * Per-hospital casebook entry for one treatment (illness/expertise index, 0x2f
 * entries in AllHosps.rooms despite the name): reputation, pricing, drug and
 * outcome counts.
 */
struct HospitalRoom {
    // 0x0: Treatment reputation 0..1000, rep_points * 500 / rep_events
    // (evaluate_reputation).
    s32 reputation;
    // 0x4: Accumulated reputation deltas for this treatment.
    s32 rep_points;
    // 0x8: Number of reputation events counted for this treatment.
    s32 rep_events;
    // 0xc: Price charged for the treatment: price_scale * the maximum price /
    // 10000.
    s32 price;
    // 0x10: Price setting as percent * 100 of the maximum price, 5000..20000.
    s32 price_scale;
    // 0x14: Money earned from this treatment.
    s32 income;
    union {
        // 0x18: Drug researched for this treatment.
        HospitalDrug drug;
        struct {
            // 0x18: Same bytes as drug.cost (set to gbv StartCost by
            // setup_drug).
            u16 drug_cost;
            u8 unknown1A;
            // 0x1b: Same bytes as drug.effectiveness (set to gbv StartRating by
            // setup_drug).
            u8 drug_effectiveness;
        };
    };
    // 0x1c: 1 when the hospital can treat it (rooms, staff, drug or machine in
    // place).
    u8 cure_available;
    // 0x1d: 1 once the illness is known to the hospital.
    u8 discovered;
    // 0x1e: Patients cured of it.
    u16 cured;
    // 0x20: Patients who died of it.
    u16 deaths;
    u8 unknown22[2];
    // 0x24: Patients with it sent home or leaving untreated.
    u16 turned_away;
};

/*
 * A request for a staff member with given skills to come to a room (pagers[]).
 */
struct SkillPager {
    // 0x0: Skill bits a responder must have.
    u16 mask;
    // 0x2: room_info index of the paging room.
    u16 room;
    // 0x4: Turn until which the responder holds the page; others may take it
    // after.
    u32 claimed_until;
    // 0x8: Thing id of the staff member answering, or 0.
    u16 responder;
    // 0xa: Hospital that sent the page.
    u8 owner;
};

/*
 * Per-object research state, a view of AllHosps.object_stats entries starting
 * at their byte 12.
 */
struct Equipment {
    // 0x0: 1 once the object can be bought (researched or available from the
    // start).
    u8 available;
    // 0x1: [0] counts research upgrades, +2 each, up to 40; the other bytes
    // belong to the next object_stats entry.
    u8 upgrade_level[13];
};

/*
 * A hospital's state for one object kind (allhosps[].object_stats).
 */
struct ObjectStats {
    // 0x0: Purchase price.
    s32 cost;
    // 0x4: Month the object becomes available (from objects[].when_avail or
    // research), 0 for none pending.
    u32 available;
    // 0x8: Strength given to new objects of this kind.
    u16 strength;
    // 0xa: Incremented for each one placed (inc_hardware_stats), and also in
    // remove_object_from_map.
    u16 count;
    // 0xc: Same byte as equipment[].available: 1 once buyable.
    u8 equip_available;
    // 0xd: Same byte as equipment[].upgrade_level[0].
    u8 upgrade_level;
};

struct ThingPage {
    u16 id;
    u32 time;
    u16 responder;
    u8 priority;
};

struct ThingPager {
    u16 count;
    ThingPage pages[0x40];
};

struct ComputerStrategy {
    u16 type;
    s16 arg;
    s16 count;
};

/*
 * Personality of one computer opponent (computer[]), loaded from the level
 * file's computer keys.
 */
struct ComputerPlayer {
    // 0x0: Skill key; the random spread subtracted from speed when pacing
    // actions.
    u8 skill;
    // 0x1: StaffLevels key; column of staff_max_nbs used for staff numbers.
    u8 staff_levels;
    u8 luck;
    // 0x3: Speed key; base pace of actions (higher waits longer).
    u8 speed;
    // 0x4: Comfort key; column of object_nbs used for comfort objects.
    u8 comfort;
    // 0x5: GuessAt key; diagnosis percentage at which it guesses a treatment.
    u8 guess_at;
    // 0x6: Copied into the hospital's AI strategy at setup.
    u8 strategy;
    u8 unknown7;
    // 0x8: Playing key; nonzero when this opponent takes part.
    u8 playing;
};

struct ComputerDecisions {
    u16 delay;
};

// The computer players' tables (comtable.cpp).
extern ComputerPlayer computer[];
extern ComputerDecisions com_decide[];
extern ComputerStrategy com_strategy[];
extern u16 staff_max_nbs[][10];
extern u16 object_nbs[][11];
extern u8 com_room_orders[][14];

struct RatHole {
    u16 tile;
    u8 side;
};

struct BuildPoint {
    u8 x;
    u8 y;
};

/*
 * A hospital's room-building state (allhosps[].build): the blueprint being
 * dragged, the room being edited and its objects.
 */
struct BuildState {
    union {
        struct {
            // 0x0: Blueprint's top-left tile x.
            u8 x0;
            // 0x1: Blueprint's top-left tile y.
            u8 y0;
            // 0x2: Blueprint's bottom-right tile x (inclusive).
            u8 x1;
            // 0x3: Blueprint's bottom-right tile y (inclusive).
            u8 y1;
        };
        struct {
            // 0x0: Same bytes as x0, y0.
            BuildPoint start;
            // 0x2: Same bytes as x1, y1.
            BuildPoint end;
            // 0x4: Corner of the blueprint held fixed while the player drags
            // it.
            BuildPoint anchor;
            // 0x6: Set to the bottom-right corner when an edge drag (drag 2)
            // starts.
            BuildPoint pivot;
        };
    };
    // 0x8: room_info index of the room being built or edited.
    u16 room;
    // 0xa: room_info index of the room under the pointer while editing, or 0.
    u16 edit_room;
    // 0xc: 0 until a blueprint drag starts, then the drag mode (1 or 2).
    u8 drag;
    // 0xd: Room type (ROOM_*) being built.
    u8 room_type;
    // 0xe: Set to 1 when a blueprint drag starts.
    u8 draw_mode;
    // 0xf: Tile where the door is being placed.
    u16 door_tile;
    // 0x11: doortype[] index of the door being placed.
    u8 door_type;
    // 0x12: Kind of door being placed (from rooms[].door_kind); 2 is a window.
    u8 door_state;
    // 0x13: Object ids the player picked up and may place again.
    u16 objects[20];
    // 0x3b: Building step (2 while waiting for the room to clear).
    u8 state;
    // 0x3c: Map overlay of the blueprint top-left tile before the blueprint was
    // marked.
    u8 saved_overlay;
};

struct ResearchWorkStations {
    u8 count;
    u16 kinds[11];
    u8 unknown17;
};

struct Population {
    u16 month;
    u16 count;
};

struct AllHosps {
    u8 name[0x29];
    u8 level;
    // 0x2a: 0 unused, 1 human, 2 computer.
    u8 player_type;
    // 0x2b: 2 while the player is in the game; 0 after they quit.
    u8 playing;
    u8 win_progress;
    s32 score;
    s32 start_score;
    s32 balance;
    u8 unknown39[4];
    s32 loan;
    s32 max_loan;
    union {
        u8 unknown45[0x176];
        struct {
            u8 unknown45_[0x90];
            s32 drinks_income;
            s32 jukebox_income;
        };
        struct {
            u8 unknown45a_[0x90];
            s32 sales_income[2];
        };
        struct {
            u8 unknown45r_[0x98];
            s32 research_points;
            s32 illness_research[0x2f];
            s32 research_cost[5];
            s16 research_alloc[5];
        };
        // 0x45: Fees owed by each of the three insurance companies, by the
        // month they pay.
        s32 insurance_claims[3][12];
    };
    s32 hospital_value;
    s32 object_value;
    union {
        u8 unknown1c3[8];
        struct {
            s32 year_start_balance;
            s32 year_start_visitors;
        };
    };
    u16 market_share;
    union {
        u8 unknown1cd[2];
        s16 month_visitors;
    };
    s16 entrances[4];
    u16 heli_pad_tile;
    union {
        u16 emergency_timer;
        s16 emergency_timer_s;
    };
    union {
        u8 unknown1db[7];
        struct {
            u8 unknown1db_[2];
            u16 emergency_bonus;
        };
        struct {
            u8 unknown1db__[4];
            u16 emergency_heli;
            u8 heli_landed;
        };
    };
    union {
        // 0x1e2: 0 none, 10 accepted (counts down), 5 summon helicopter, 3
        // helicopter arriving, 2 in progress, 1 over.
        u8 emergency_state;
        s8 emergency_state_s;
    };
    union {
        u8 unknown1e3[2];
        struct {
            s8 emergency_to_spawn;
            u8 emergency_illness;
        };
    };
    u8 emergency_cured;
    union {
        u8 unknown1e6[3];
        struct {
            u8 emergency_patients;
            u8 emergency_present;
            u8 emergency_win_pct;
        };
    };
    u16 cures;
    u16 deaths;
    u16 visitors;
    u16 diagnosed;
    u16 area;
    u32 mayor_arrival_turn;
    union {
        u8 unknown1f7[2];
        u16 mayor;
    };
    u8 mayor_name;
    // 0x1fa: 0 none, 1 visit offered, 2 visit scheduled, 3 mayor on site, 4
    // mayor gone.
    u8 mayor_state;
    union {
        u8 unknown1fb[8];
        struct {
            u16 mayor_verdict;
            u16 mayor_postponed;
            u16 mayor_launch_count;
            u16 inspector;
        };
    };
    // 0x203: Fine charged after the inspection; when no contagious patients are
    // found, the compensation paid instead.
    u16 epidemic_fine;
    u16 contagious_count;
    u16 epidemic_timer;
    // 0x209: [0] inspector verdict 0..3, [1] 3 while the health inspector is on
    // site.
    u8 inspection[2];
    u8 epidemic_illness;
    u8 inspected_illness;
    u8 epidemic_active;
    u8 epidemic_over;
    union {
        u8 unknown20f[7];
        struct {
            u16 disaster_counter;
            u16 saved_heating;
            // 0x213: 0 none, 1 boiler fails cold, 2 boiler fails hot, 3 litter
            // disaster.
            u8 disaster_kind;
            s8 disaster_timer;
        };
    };
    u8 insurance_accepted;
    u16 diag_threshold;
    u16 guess_cure_threshold;
    u16 send_home_threshold;
    u16 rest_threshold;
    u8 staff_leave_rooms;
    HospitalRoom rooms[0x2e];
    union {
        struct {
            u8 unknown8f4[0x32];
            union {
                Equipment equipment[0x3e];
                struct {
                    u8 unknown926[0x358];
                    s32 reputation;
                    s32 rep_points;
                    s32 rep_events;
                };
            };
        };
        struct {
            u8 unknown8f4_[0x26];
            ObjectStats object_stats[0x3e];
        };
    };
    union {
        u8 unknownc8a[4];
        struct {
            s16 avg_patient_happiness;
            s16 avg_staff_happiness;
        };
    };
    s16 price_level;
    u16 extinguisher_count;
    u16 radiator_count;
    u8 heating_level;
    u8 heating_broken;
    u8 object_count;
    u8 machine_count;
    u16 mishaps;
    u16 amenity_count;
    u16 staff_count;
    // 0xc9e: Staff counts: [0] nurses, [1] doctors (surgeons included), [2]
    // handymen, [3] receptionists.
    s16 staff_by_kind[4];
    s16 surgeon_count;
    u16 leisure_count;
    u16 litter_count;
    u16 last_litter_tile;
    u8 rathole_count;
    RatHole ratholes[0x28];
    union {
        s16 rats_killed;
        u16 rats_killed_u;
    };
    u16 rat_shots;
    union {
        u8 unknownd2b[0x242];
        ThingPager plant_pager;
    };
    ThingPager repair_pager;
    union {
        struct {
            u8 unknownf6e[8];
            u16 edited_room;
        };
        BuildState build;
        struct {
            u8 unknownf6e_[0x3d];
            u16 vaccine_queue;
        };
    };
    u16 idle_nurses_prev;
    u16 idle_nurses;
    u16 vaccine_requests;
    u16 held_staff;
    u16 held_staff_tile;
    u32 next_action_turn;
    ComputerStrategy action;
    u16 action_from_note;
    ComputerStrategy notes[8];
    s32 wage_bill;
    u32 rooms_built;
    s32 patient_load;
    s16 strategy_step;
    s16 recheck_step;
    u8 strategy;
    u8 ai_index;
    s16 staff_ability_sum[8];
    // 0x1256: Single player: 1 once the opening countdown has ended. Network
    // games: illness chosen for concentrated research.
    u8 hospital_open;
    // 0x1257: Turns until the hospital opens to patients (from
    // gbv.open_countdown).
    u16 opening_countdown;
};

struct BankEntry {
    s32 amount;
    u8 type;
    u8 day;
    u8 month;
    u8 extra;
};

/*
 * The local player's finances and calendar (instance *mycontrol): month and
 * year totals, the bank statement and the graph history.
 */
struct GameControl {
    union {
        u8 unknown0[0x10];
        struct {
            // 0x0: Wages and hiring costs paid this month (change_balance types
            // 1 and 2); moved to history[0] at month end.
            s32 wages_this_month;
            // 0x4: Money received this month; moved to history[1] at month end.
            s32 income_this_month;
            // 0x8: Money spent this month (positive); moved to history[2] at
            // month end.
            s32 expenditure_this_month;
            // 0xc: Own patients cured this month; cleared at month end.
            s32 cures_this_month;
        };
    };
    // 0x10: Own patients who died this month; cleared at month end.
    u32 deaths_this_month;
    union {
        u8 unknown14[0x63b];
        struct {
            u8 unknown14_[0x634];
            // 0x648: Years since the game started (shown as 2000 + year).
            u16 year;
        };
        struct {
            u8 unknown14c_[0xc];
            // 0x20: Graph data [row][scale][month], month 11 the newest: rows
            // wages, income, expenditure, balance, visitors, deaths, cures,
            // reputation.
            s32 history[8][3][12];
        };
        struct {
            // 0x14: Sum of the last 12 months of income, computed at year end.
            s32 income_last_year;
            // 0x18: Sum of the last 12 months of expenditure, computed at year
            // end.
            s32 expenditure_last_year;
            u8 unknown1cb_[0x484];
            // 0x4a0: Hospital reputation when the year ended.
            s32 year_start_reputation;
            // 0x4a4: Total cures when the year ended; cures this year are
            // counted from it.
            s32 year_start_cures;
            // 0x4a8: Total deaths when the year ended.
            s32 year_start_deaths;
            // 0x4ac: Staff count when the year ended.
            s32 year_start_staff;
            // 0x4b0: Staff count when the year ended (second copy, never read).
            s32 year_start_staff2;
            // 0x4b4: Opening balance of the bank statement: grows by the
            // amounts of entries dropped when the statement is full.
            s32 statement_balance;
            // 0x4b8: Bank statement lines, oldest first.
            BankEntry entries[0x32];
            u8 unknown648b_[2];
            // 0x64a: Hospital area when the year ended.
            u16 year_start_area;
            // 0x64c: Heating cost factor (750): monthly bill is radiators *
            // heating setting * heating_price / 100.
            u16 heating_price;
            // 0x64e: Number of used entries.
            u8 entry_count;
        };
        struct {
            u8 unknown14a_[0x278];
            // 0x28c: Alias of history[4][0][11]: visitors counted so far this
            // month.
            s32 history_visitors_now;
            u8 unknown290_[0x11c];
            // 0x3ac: Alias of history[6][0][11]: the cures entry for this
            // month.
            s32 history_cures_now;
        };
    };
    // 0x64f: Day of the month, from 1.
    u8 day;
    // 0x650: Month of the year, 0-11.
    u8 month;
};

struct ListHead {
    u16 objects[0x3e];
    u16 staff[0x10];
    u16 rooms[0x1e];
    u16 first[9];
};

struct PlayerCall {
    u16 type;
    u16 x;
    s16 timer;
    u16 id;
    u8 active;
    u8 slot;
    u8 flash;
    u8 owner;
};

struct Window;

/*
 * A button or slider of a Window, in its linked list.
 */
struct WindowButton {
    // 0x0: x relative to the window.
    s16 x;
    // 0x2: y relative to the window.
    s16 y;
    // 0x4: Width.
    u16 width;
    // 0x6: Height.
    u16 height;
    // 0x8: Bits: 1 disabled (drawn with disabled_sprite and not processed), 2
    // held, 4 pressed (toggled when 0x4000000 is set), 0x200000 cycles the
    // window's size; the others choose which mouse events press it.
    u32 flags;
    // 0xc: Next button of the window, or null.
    WindowButton *next;
    // 0x10: Called with the window when the button is pressed.
    void (*callback)(Window *w);
    // 0x14: Normal sprite (button_sprite index), drawn at x, y.
    s32 sprite;
    // 0x18: Sprite drawn while held or pressed; a slider's knob.
    s32 pressed_sprite;
    // 0x1c: Sprite drawn while disabled.
    s32 disabled_sprite;
    union {
        struct {
            // 0x20: x of disabled_sprite relative to the window.
            s16 disabled_x;
            // 0x22: y of disabled_sprite.
            s16 disabled_y;
        };
        // 0x20: A slider's value.
        s32 *value;
    };
    union {
        struct {
            // 0x24: x of pressed_sprite relative to the window.
            s16 pressed_x;
            // 0x26: y of pressed_sprite.
            s16 pressed_y;
        };
        // 0x24: A slider's minimum value.
        s32 min;
    };
    // 0x28: A slider's maximum value.
    s32 max;
};

// WindowButton's layout without the unions: Watcom 10.6 cannot initialise a
// union holding an anonymous struct, so the button tables (wininfo.cpp,
// mwininfo.cpp) are defined with this type.
struct WindowButtonData {
    s16 x;
    s16 y;
    u16 width;
    u16 height;
    u32 flags;
    WindowButton *next;
    void (*callback)(Window *w);
    s32 sprite;
    s32 pressed_sprite;
    s32 disabled_sprite;
    s16 disabled_x;
    s16 disabled_y;
    s16 pressed_x;
    s16 pressed_y;
    s32 max;
};

/*
 * A window of the in-game window system (winproc.cpp, windraw.cpp);
 * open_windows[] holds up to two, front first. It has a normal, a minimised
 * and a maximised size.
 */
struct Window {
    // 0x0: Screen x of the top-left corner.
    s16 x;
    // 0x2: Screen y of the top-left corner.
    s16 y;
    // 0x4: x saved on leaving the normal size, restored on returning to it.
    s16 restore_x;
    // 0x6: y saved with restore_x.
    s16 restore_y;
    // 0x8: Width at the normal size.
    u16 width;
    // 0xa: Height at the normal size.
    u16 height;
    // 0xc: Width while minimised (flags 0x80).
    u16 min_width;
    // 0xe: Height while minimised.
    u16 min_height;
    // 0x10: Width while maximised (flags 0x100).
    u16 max_width;
    // 0x12: Height while maximised.
    u16 max_height;
    // 0x14: Bits: 1 no frame (only marks its area for redraw), 2 and 4 frame
    // style 0 and 1 (draw_a_window); 0x20 and 0x40 centre horizontally and
    // vertically; 0x80 minimised; 0x100 maximised; 0x600 on the front window
    // stops OpenWindow.
    u16 flags;
    u8 unknown16[4];
    // 0x1a: First of the window's buttons (linked through WindowButton.next).
    WindowButton *buttons;
    // 0x1e: Draws the window's contents after its frame and buttons; may be
    // null.
    void (*draw)(Window *w);
};

/*
 * One entry of expertise[], indexed by illness (ILL_*): how the illness or
 * diagnosis is treated and researched. Level-file keys expertise.* set some
 * fields (vloader.cpp).
 */
struct Expertise {
    // 0x0: Skill bits (SKILL_*) of the staff needed to treat it.
    u16 staff_mask;
    // 0x2: StartPrice key: the treatment's starting price.
    u16 start_price;
    // 0x4: Object kind (OBJ_*) the treatment uses; OBJ_PHARMACY_CABINET for
    // drug cures.
    u8 kind;
    // 0x5: What research can buy for it: 1 a drug, 2 a treatment machine, 3 a
    // diagnosis machine, 0 nothing.
    u8 research_type;
    // 0x6: possible_diagnoses index listing the treatments a doctor may guess
    // for it.
    u8 diag_set;
    // 0x7: Bit 0 set: patients go to the ward (ILL_DIAG_WARD) before this
    // treatment.
    u8 needs_ward;
    // 0x8: What setup_next_treatment does after it: 0 clears the treatment, 1
    // sends the patient back to the GP, 2 cures (or kills) them, 4 diagnoses
    // them, 5 moves on to next_treatment (else the GP); 3 nothing.
    u8 outcome;
    // 0x9: Room type (ROOM_*) where it is treated.
    u8 category;
    // 0xa: ContRate key: chance a new patient with it is contagious
    // (add_contagiousness).
    u8 cont_rate;
    // 0xb: Nonzero: never made contagious or spread by an epidemic, and a
    // failed drug treatment kills the patient (administer_drugs).
    u8 not_contagious;
    // 0xc: RschReqd key: research points needed for its drug or machine, and
    // the base of the improvement costs.
    u32 cost;
    // 0x10: MaxDiagDiff key: upper bound of a new patient's random
    // diagnosis_handicap.
    u16 max_diag_diff;
    // 0x12: Known key: 1 if the hospital knows it from the start (copied to
    // rooms[].discovered).
    u8 known;
};

/*
 * One entry of objects[], indexed by object kind (OBJ_*): an object's fixed
 * properties. Level-file keys objects.* set some fields (vloader.cpp).
 */
struct ObjectControl {
    // 0x0: Index into object_names.
    u16 name;
    // 0x2: shapes[] index of the object's footprint for each facing; 0 when it
    // cannot face that way.
    u8 shapes[4];
    // 0x6: face_dirns index giving the direction a user faces for each object
    // facing, or -1.
    u8 face_dirn;
    // 0x7: StartAvail key: 1 if buyable from the start (copied to
    // object_stats[].equip_available).
    u8 start_avail;
    // 0x8: StartCost key: purchase price.
    s32 cost;
    // 0xc: WhenAvail key: month it becomes available (copied to
    // object_stats[].available).
    u16 when_avail;
    // 0xe: start[] index of the object's frames, plus the facing.
    u16 anim_start;
    // 0x10: Not read by the game code.
    u16 preview_anim;
    // 0x12: Not read by the game code.
    u16 repair_aniseq;
    // 0x14: Object category; placing one of category 7 counts as a reception
    // desk for the AI (computer_place_object).
    u8 category;
    // 0x15: Copied to Thing.use_style when the object is created.
    u8 use_style[1];
    // 0x16: Not read by the game code.
    s8 walls;
    // 0x17: next_part[] index of the second part of a two-part object, or -1.
    s8 next_part_index;
    // 0x18: wall_hug[] index used to place it against a wall, or -1
    // (gamevar.place_wall_hug).
    u8 wall_hug;
    u8 unknown19;
    // 0x1a: Tile offset from the machine to where its explosion starts.
    u16 explosion_offset;
    // 0x1c: AvailableForLevel key: 1 if the object exists in this level.
    u8 available_for_level;
    // 0x1d: StartStrength key: a new machine's strength.
    u8 start_strength;
    // 0x1e: start[] index of a machine's overlay frame, or 0.
    u16 overlay_anim;
    union {
        // 0x20: Blueprint id (plus the facing) of the wreck an exploded machine
        // leaves; nonzero only for machines, which the mouse and the
        // find-machine button test it for.
        u16 debris;
        u8 debris_lo;
    };
    // 0x22: Bit 0: cannot be picked up; bit 1: hovering shows panel info 6; bit
    // 2: clicking sends no query event (0x42); bit 3: can be picked up from
    // inside a room (radiators, extinguishers, plants).
    u8 flags;
    // 0x23: Thing.state a new object starts in.
    u8 initial_state;
    // 0x24: Nonzero for machines.
    u8 is_machine;
};

/*
 * One entry of rooms[], indexed by room type (ROOM_*): a room's fixed
 * properties. The level-file key rooms.Cost sets cost (vloader.cpp).
 */
struct RoomControl {
    // 0x0: Name index in language section 52.
    u16 id;
    // 0x2: Wall colour style recolour_all_rooms paints the room in (3-6; 1 and
    // 2 none).
    u8 wall_style;
    // 0x3: staff_states row giving the state of a staff member placed in the
    // room, by staff kind.
    u8 staff_state_row;
    // 0x4: Cost key: price of building the room.
    s32 cost;
    // 0x8: Skill bits of the doctor the room pages for.
    u16 doctor_skills;
    // 0xa: Object kind the room is built around, or 0.
    u16 main_object;
    // 0xc: Number of leading objects entries that must be placed.
    u8 compulsory_count;
    union {
        // 0xd: Object kinds placed in the room; the first compulsory_count are
        // compulsory.
        u8 objects[0x15];
        struct {
            u8 objects_[0xa];
            u8 buyable_count;
            u8 buyable[0xa];
        };
    };
    // 0x22: Map overlay set on the floor tiles of the room while it is a
    // blueprint.
    u8 overlay;
    u8 unknown23;
    // 0x24: 0 for double doors (init_double_doors), else a single door that
    // patients enter straight through; copied to build.door_state.
    u8 door_kind;
    u8 unknown25;
    // 0x26: Staff kind (STAFF_*) the AI hires for the room, or -1.
    s8 staff_kind;
    // 0x27: Smallest blueprint width allowed.
    u8 min_width;
    // 0x28: Smallest blueprint height allowed.
    u8 min_height;
    // 0x29: Nonzero: hovering over the room's door shows its queue info.
    u8 has_queue;
};

/*
 * A department of the build-room requester (departments[]) and the room types
 * it offers.
 */
struct DepartmentControl {
    // 0x0: Number of entries in rooms.
    u8 count;
    // 0x1: Nonzero if the department appears in the requester list.
    u8 listed;
    // 0x2: Room types offered.
    u8 rooms[0xa];
};

/*
 * One entry of shapes (ObjectControl.shapes per facing): an object's map
 * footprint and use tiles as tile offsets (y * 128 + x) from the object's
 * tile.
 */
struct Shapes {
    // 0x0: Number of footprint points in points.
    s8 count;
    union {
        // 0x1: Footprint tile offsets; only the first count are used, the rest
        // of the array overlays the fields below.
        u16 points[0x15];
        struct {
            u8 unknown1[0x12];
            // 0x13: Tile offsets of use positions 0-2 (Thing.use_pos[0..2]);
            // marked with map flag 4 while the object stands.
            s16 use_offsets[3];
            // 0x19: Tile offsets of use positions 3-5 (Thing.use_pos[3..5]);
            // marked like use_offsets.
            s16 extra_use_offsets[3];
            // 0x1f: [0] and [1]: x and y tile delta of the use point (use_x,
            // use_y); [2]: the object's depth.
            u8 use_xy_depth[3];
            // 0x22: Per footprint point: bit 0 and bit 1 forbid placing the
            // point on a cell with the first or second wall byte set.
            u8 point_wall_flags[9];
        };
    };
    // 0x2b: Same wall test as point_wall_flags, applied to the three
    // use_offsets tiles.
    u8 use_wall_flags;
};

/*
 * One entry of blueprint[], indexed by small-thing id: a wall, door or window
 * piece drawn on the map.
 */
struct Blueprint {
    // 0x0: start[] index of the piece's frame.
    u16 anim;
    // 0x2: Wall block (or door kind) the piece builds.
    u16 kind;
    // 0x4: How the piece is built (the switch in build_all_rooms); for doors,
    // the placing direction plus 1.
    u8 build_type;
    // 0x5: Map cell flags set on the cell the piece stands on; build_all_rooms
    // also tests bits 0 and 1.
    u8 flags;
    // 0x6: Index, from wall_n, of the cell wall byte that kind is written to.
    u8 wall_layer;
    // 0x7: Object kind of the door the piece shows (OBJ_DOOR,
    // OBJ_SWING_DOOR...).
    u8 object_kind;
};

/*
 * One entry of doortype[]: the pieces of a door or window kind and facing.
 */
struct DoorInfo {
    // 0x0: Number of tiles (and pieces) the door covers.
    u8 count;
    // 0x1: Tile offsets of its pieces from the door tile.
    u16 tiles[3];
    // 0x7: start[] index of the frame drawn under the pointer while placing it.
    u16 cursor_anim;
    // 0x9: Blueprint ids of its pieces.
    u16 sprites[3];
    // 0xf: Tile offset of the room's door_tile.
    u16 door1;
    // 0x11: Tile offset of the room's door_inside_tile.
    u16 door2;
    // 0x13: Map overlays set on its tiles.
    u16 overlays[3];
    // 0x19: Tile offset subtracted for the overlay and preview tiles.
    u16 offset;
    // 0x1b: Nonzero for doors: placing one sets the room's two door-side tiles
    // from door1 and door2; zero for the window entries.
    u8 is_entrance;
};

struct VXY {
    s16 x;
    s16 y;
};

struct AnimStart {
    union {
        u16 anims[0x8c];
        struct {
            u16 walk;
            u8 unknown2[8];
            u16 staff_stand;
            u8 unknownC[6];
            u16 patient_stand;
            u8 unknown14[0x8e];
            u16 unknownA2;
            u8 unknownA4[0x74];
        };
    };
};

/*
 * One entry of staff[], indexed by staff kind (STAFF_*).
 */
struct StaffControl {
    u8 unknown0[2];
    // 0x2: MinSalary key: salary before the ability and skill additions
    // (appraise_staff).
    u16 min_salary;
    // 0x4: anims[] index in staff_anim_starts of the frames shown for a new or
    // held staff member.
    u16 anim;
    u8 unknown6[5];
    // 0xb: Number of layer-5 graphic variants a new applicant picks from; 0 for
    // none.
    u8 layer5_count;
    u8 unknownc;
};

struct AnimTable {
    char name[0x28];
    u16 cd_install_level;
    u16 f2a;
    u16 f2c;
    u32 text_frame;
    u32 smk_flags;
    u32 play_flags;
};

struct CompFrame {
    u32 list;
    u8 width;
    u8 length;
    u8 sound;
    u8 flags;
    u16 next;
};

struct CompElement {
    u16 sprite;
    u8 x;
    u8 y;
    u16 flags;
};

/*
 * One entry of icon_table, indexed by a person's icon_id: the mood/status icon
 * drawn over a person.
 */
struct IconTable {
    // 0x0: A new icon replaces the current one only if its priority value is
    // lower or equal.
    u8 priority;
    // 0x1: start[] index of the icon's animation (sets icon_frame).
    u16 anim;
    union {
        u8 unknown3[3];
        struct {
            // 0x3: Turns the icon stays up (copied to icon_timer) unless mode
            // is positive.
            u16 duration;
            // 0x5: Below 0: shown later with a sound by peep_icon (icon_id
            // stored negated); 0: timed by duration; above 0: stays until
            // reset.
            s8 mode;
        };
    };
};

/*
 * One entry of sfx_details, indexed by sound effect number: how play_sound
 * picks and plays its sample.
 */
struct SfxDetail {
    union {
        u8 unknown0[0xc];
        struct {
            u8 unknown0_[4];
            // 0x4: Passed to play_sample_using_heap as the loop count.
            s8 loop;
            u8 unknown5_[3];
            // 0x8: How the sample is chosen: 1 plays sample, 2 stops sample, 3
            // sample at a random pitch from pitch_group[group], 4 random sample
            // from sample_group[group], 5 sample at a pitch in the range of
            // pitch_group[group].
            u8 type;
            // 0x9: Sample number for types 1, 2, 3 and 5.
            u8 sample;
            // 0xa: pitch_group or sample_group index for types 3-5.
            u8 group;
            // 0xb: Channel policy passed to PlaySampleFromAddress as its type
            // (1 any free channel, 2 skip if already playing for this source, 3
            // reuse this source's channel).
            u8 play_type;
        };
    };
    // 0xc: When set, an animation frame's sound plays every time the frame is
    // drawn, not only when it differs from the person's last sound.
    u16 allow_repeat;
};

struct CompStarts {
    u32 frame;
    u32 flags;
};

struct ToDraw {
    ToDraw *next;
    Thing *widget;
    int x;
    int y;
};

/*
 * The refresh box that follows refresh in memory is declared separately as
 * D_001567b0; x0 is reached through refresh where the code loads it as the
 * low half of a dword.
 */
struct Refresh {
    union {
        u8 unknown0[4];
        struct {
            // 0x0: Horizontal wrap offset of the background screen (bscreen),
            // which scrolls circularly with the bscreen window's view.
            s16 scroll_x;
            // 0x2: Vertical wrap offset of the background screen.
            s16 scroll_y;
        };
    };
    // 0x4: The first word of the refresh box D_001567b0 declared after refresh.
    s16 x0;
};

struct PixBuffer {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s32 size;
    s32 saved_width;
    s32 saved_height;
    u8 *saved_ptr;
    s32 saved_x;
    s32 saved_y;
    s32 saved_w;
    s32 saved_h;
    u8 *data;
};

/*
 * One item of a pulldown menu.
 */
struct PulldownItem {
    // 0x0: The item still works while the game is paused.
    u8 usable_paused;
    // 0x1: The item still works on the rat level (gamevar.rat_fest_level).
    u8 usable_rat_level;
    // 0x2: Menu text index.
    u8 text;
    // 0x3: Byte toggled when the item is chosen and shown as its tick, or 0.
    u8 *check;
    // 0x7: Byte shown as a tick without being toggled, or 0.
    u8 *check2;
    // 0xb: Pulldown index of the item's submenu, 0 for none.
    u8 submenu;
    // 0xc: Modifier keys sent with key.
    u8 shift;
    // 0xd: Key sent as if pressed when the item is chosen, 0 for none.
    u8 key;
    // 0xe: Value stored in othervar.pulldown_arg before the callback runs.
    u8 arg;
    // 0xf: Bit 0: the item is disabled while statvar.num_players > 1
    // (multiplayer).
    u8 flags;
    // Called when the item is chosen.
    void (*callback)(void);
};

struct Pulldown {
    u8 active;
    u8 count;
    u8 text;
    u16 length;
    u16 width;
    u8 parent;
    u16 y;
    PulldownItem items[30];
};

/*
 * The pulldown menu bar (instance pmenu).
 */
struct menu {
    // 0x0: Index of the open pulldown, 0 when none is open.
    u8 open_pulldown;
    union {
        u8 unknown1[6];
        struct {
            u8 font;
            u8 font_hi;
            u8 line_height;
            u8 char_width;
            u8 count;
            u8 subcount;
        };
    };
    // 0x7: Set while a modal requester is open; the menu bar then ignores the
    // mouse.
    u8 disabled;
    Pulldown pulldowns[13];
};

struct HospPlayerDetails {
    union {
        char name_chars[0x11];
        s8 name[0x11];
        struct {
            u8 unknown0_[0xc];
            s8 music_volume;
            s8 sfx_volume;
            s8 ambient_volume;
            u8 sfx_on;
            u8 ambients_on;
        };
        struct {
            u8 unknown0a_[9];
            u16 years_played;
            u8 fb;
        };
    };
    u8 music_on;
    u8 track_enabled[0xa];
    u8 f1c;
    u8 shadows_on;
    union {
        u16 game_speed;
        s16 game_speed_s;
    };
    u8 speed_choice;
    u8 language;
    u8 autosave;
    u8 room_help_seen[0x1e];
    u8 advisor_flags[0x15];
    u8 serial_config[6];
};

/*
 * Palette indices of fixed colours, looked up by RGB in setup_colors (instance
 * color), plus the wall colour chosen while building rooms.
 */
struct Colors {
    // 0x0: Palette the colours were looked up in.
    u8 *palette;
    // 0x4: Wall colour used by build_all_rooms (corner_wall_colour, else
    // side_wall_colour, else 7).
    s8 wall_colour;
    // 0x5: Colour of a wall found beside the blueprint by
    // search_for_outer_color, -1 if none.
    s8 side_wall_colour;
    // 0x6: Colour of a corner wall found by search_for_outer_color, -1 if none.
    s8 corner_wall_colour;
    // 0x7: RGB 63,63,63.
    u8 white;
    // 0x8: RGB 0,0,0.
    u8 black;
    // 0x9: RGB 0,0,16.
    u8 dark_blue;
    // 0xa: RGB 0,63,0.
    u8 green;
    // 0xb: RGB 63,48,23.
    u8 orange;
    // 0xc: RGB 63,23,23.
    u8 light_red;
    // 0xd: RGB 63,0,0.
    u8 red;
    // 0xe: RGB 27,55,57.
    u8 cyan;
    // 0xf: RGB 53,31,66.
    u8 purple;
    // 0x10: RGB 23,67,16; not read anywhere.
    u8 light_green;
    // 0x11: RGB 38,41,47.
    u8 grey;
    // 0x12: RGB 0,38,86.
    u8 blue;
    // 0x13: RGB 36,80,47.
    u8 pale_green;
    // 0x14: RGB 63,30,30.
    u8 salmon;
};

/*
 * Received network messages shown scrolling over the view (current_text).
 */
struct CurrentText {
    // 0x0: The messages, each appended as "name:text".
    char text[0x5e6];
    u8 unknown5e6[2];
};

/*
 * The four bytes before D_001549d0 (the players' Events). packet is the base
 * of the per-player 0xf-byte block exchanged over the network and recorded or
 * played back each turn, so player p's block starts 4 bytes before
 * D_001549d0[p].
 */
struct Packet {
    u8 unknown0[4];
};

/*
 * One player's network chat message (net_text[]), used as a string through
 * NET_TEXT.
 */
struct NetText {
    // 0x0: First character of the message; 0 empties it.
    u8 text;
    u8 unknown1[0x31];
};

/*
 * Per object type, objects bought but not yet placed (gamevar.buy_list).
 */
struct Blob63 {
    // 0x0: Count per object type, at most 0x63.
    u8 to_place[0x3f];
};

/*
 * A pending emergency offer (gamevar.emergency_calls[]), copied into the
 * accepting hospital by give_emergency_stats.
 */
struct EmergencyCall {
    // 0x0: Player call id the offer belongs to.
    u16 call;
    // 0x2: Bonus paid per cured patient.
    u16 bonus;
    // 0x4: Nonzero while the slot holds an offer.
    u8 used;
    // 0x5: Illness of the emergency patients.
    u8 illness;
    // 0x6: Number of patients.
    u8 count;
    // 0x7: Percent of the patients that must be cured to earn the bonus.
    u8 win_percent;
};

struct Game {
    union {
        u8 unknown0[0x1e];
        struct {
            u32 staff_seed;
            u16 applicant_count;
            u16 applicants[4];
            u8 unknowne[2];
            u16 applicants_wanted[4];
            u8 unknown18_[2];
            u16 peeps_arrived;
            s16 peep_interval;
        };
    };
    u8 place_blueprint;
    u8 unknown1f;
    // 0x20: room_info index of the room whose objects the local player was last
    // sent to place.
    u8 new_room_index;
    u8 paused;
    union {
        u8 unknown22[3];
        struct {
            u8 unknown22_;
            u16 place_tile;
        };
    };
    u8 place_kind;
    u8 place_mode;
    union {
        u8 unknown27[8];
        struct {
            s8 place_wall_hug;
            u8 place_dir;
            u8 place_redraw;
            u8 place_ok;
            s8 place_max_x;
            s8 place_max_y;
            s8 place_min_x;
            s8 place_min_y;
        };
    };
    u16 place_room;
    s16 place_offset;
    union {
        Blob63 buy_list;
        struct {
            u8 unknown33_[0x11];
            // 0x44: buy_list entry for OBJ_LITTER_BOMB: bombs bought and not
            // yet placed.
            u8 litter_bombs_to_place;
            u8 unknown45[0x2c];
            u8 buy_kind_count;
        };
        u8 buy_count[0x3f];
    };
    u8 staff_place_ok;
    u16 staff_cursor_anim;
    u16 held_staff;
    u16 held_staff_kind;
    u16 drawn_staff_kind;
    u16 announce[14][4];
    u32 quake_timer;
    u8 quake_phase;
    union {
        u8 quake_strength;
        s8 quake_strength_s;
    };
    u8 quake_shake_chance;
    s8 quake_amplitude;
    u8 quake_shake_index;
    union {
        s8 quake_ramp;
        u8 quake_ramp_u;
    };
    // 0xf5: Nonzero while aliens are abducting a patient; cleared when the
    // saucer leaves.
    u16 abductee;
    u16 ff7;
    u8 unknownf9[4];
    EmergencyCall emergency_calls[4];
    u32 next_emergency_turn;
    u16 emergency_bonus;
    u8 emergency_pending;
    u8 emergency_illness;
    u8 emergency_patients;
    u8 emergency_win_percent;
    u32 next_quake_turn;
    u8 quake_severity;
    u8 quake_pending;
    u8 turn_skipped;
    u16 mouse_mode;
    u16 hover_action;
    u16 scroll_min_tile;
    u16 scroll_max_tile;
    u16 research_illness;
    u8 max_speed;
    u8 turbo;
    u16 queue_selected_person;
    u8 unknown13c[2];
    u8 emergencies_active;
    u16 rat_fest_kills_mark;
    u32 rat_fest_rats;
    u16 visitor_change;
    u16 visitors_per_month;
    u16 peep_owner_counter;
    u8 quake_used[16];
    u8 emergency_used[20];
    u8 shrink_rate;
    u8 surgeon_rate;
    u8 researcher_rate;
    u8 consultant_rate;
    u8 junior_rate;
    u8 rat_fest_level;
    u8 rat_fest_powerup;
    // 0x176: Times the player declined the next level after a win.
    u8 win_declined_count;
    u8 prev_year_deaths;
};

struct Bounds {
    s8 minX;
    s8 minY;
    s8 maxX;
    s8 maxY;
};

struct Statics {
    s32 save_game_size;
    u16 view_height;
    s16 panel_top;
    union {
        u8 unknown8[0x64];
        struct {
            u16 visuals_total;
            u16 non_visuals_total;
        };
        struct {
            u8 unknown8a_[0x62];
            u16 illness_kind_count;
        };
        struct {
            u8 unknown8h_[4];
            u16 help_start[0x1c];
        };
        struct {
            u8 unknown8s_[0x3a];
            u16 aniseq_start[0x15];
        };
    };
    union {
        char save_prefix[0x79];
        struct {
            u8 unknown6c_[0x64];
            char sound_path[0x14];
            u8 sound_ok;
        };
    };
    u8 music_ok;
    u8 unknowne6;
    u8 unknowne7;
    u8 interaction_start[0x14];
    u8 quiet;
    u8 num_players;
    u8 unknownfe;
    Bounds object_bounds[62][4];
    u16 help_page0_start[31];
    u16 help_page1_start[19];
    u16 winlose_text_start[12];
    u16 net_help_start[9];
    // 0x56d: net_result_start indexed from 1: [n] is net_result_start[n - 1].
    u16 net_result_base[1];
    // 0x56f: Start of each network game result text in language group 0x43
    // (beta_demo_text_short).
    u16 net_result_start[3];
    char save_root[100];
    u8 ref_general_count;
    u8 ref_good_count;
    u8 ref_bad_count;
    u8 f5dc;
    u8 midi_count;
    u8 f5de;
};

/*
 * Scroll requests for this turn from the player's input flags (scroll);
 * move_windows turns them into the active window's scroll velocity and clears
 * them.
 */
struct Scroll {
    // 0x0: Scroll up requested.
    u8 up;
    // 0x1: Scroll down requested.
    u8 down;
    // 0x2: Scroll left requested.
    u8 left;
    // 0x3: Scroll right requested.
    u8 right;
};

/*
 * Scratch results of the isometric cell/window helpers (instance info):
 * find_cell_at_point and friends fill the cell_* fields,
 * setup_stats_for_window_scan the scan_* fields read by the draw_* scanners,
 * find_common_window/find_biggest_window the clip_* rectangle.
 */
struct Info {
    union {
        u8 unknown0[0x54];
        struct {
            // 0x0: Map index (col + row * 128) of the first cell of the window
            // scan.
            s32 scan_cell;
            // 0x4: Column of scan_cell.
            s32 scan_col;
            // 0x8: Row of scan_cell.
            s32 scan_row;
            // 0xc: Screen x of scan_cell.
            s32 scan_x;
            // 0x10: Half-tile x offset between consecutive scan lines (+ or -
            // resvars.tile_width / 2).
            s32 scan_dx;
            // 0x14: Screen y of scan_cell.
            s32 scan_y;
            // 0x18: Cells in the first scan line.
            s32 line_len;
            // 0x1c: Line length change applied on alternate scan lines (-1, 0
            // or 1).
            s32 line_len_step;
            // 0x20: Scan lines needed to cover the window's floor.
            s32 floor_lines;
            // 0x24: Scan lines including the extra height for tall sprites.
            s32 full_lines;
        };
    };
    // 0x54: X of the cell last found or given (screen x, or absolute x from
    // give_absolute_xy_of_cell_at_column_row).
    s32 cell_x;
    // 0x58: Y of that cell.
    s32 cell_y;
    // 0x5c: Column of the cell found by find_cell_at_point.
    s32 cell_col;
    // 0x60: Row of that cell.
    s32 cell_row;
    // 0x64: Map index of that cell (cell_col + cell_row * 128).
    s32 cell;
    // 0x68: Left edge of the rectangle clipped by find_common_window /
    // find_biggest_window.
    s32 clip_x0;
    // 0x6c: Top edge of that rectangle.
    s32 clip_y0;
    // 0x70: Right edge of that rectangle.
    s32 clip_x1;
    // 0x74: Bottom edge of that rectangle.
    s32 clip_y1;
    // 0x78: 1 when the clipped rectangle has zero width or height.
    u8 clip_empty;
    u8 unknown79[7];
};

/*
 * The game view windows (window): up to two isometric views of the map, each
 * with a scroll position, an on-screen rectangle (zone) and an open/close
 * animation. Arrays are indexed by window number.
 */
struct HospWindow {
    union {
        u8 unknown0[0x30];
        struct {
            // 0x0: Map scroll position: world screen x shown at the view
            // centre.
            s32 scroll_x[3];
            s32 scroll_y[3];
            // 0x18: scroll_x when the window was last drawn.
            s32 prev_scroll_x[3];
            s32 prev_scroll_y[3];
        };
    };
    union {
        struct {
            s32 zone0_x;
            u8 unknown34[8];
        };
        s32 zone_x[3];
    };
    union {
        struct {
            s32 zone0_y;
            u8 unknown40[8];
        };
        s32 zone_y[3];
    };
    union {
        struct {
            s32 zone0_x1;
            u8 unknown4c[8];
        };
        // 0x48: Right edge of the window on screen (exclusive bound used by the
        // clip code).
        s32 zone_x1[3];
    };
    union {
        struct {
            s32 zone0_y1;
            u8 unknown58[0x38];
        };
        // 0x54: Bottom edge of the window on screen.
        s32 zone_y1[3];
        struct {
            s32 zone_h_[3];
            // 0x60: Zone when the window was last drawn; the old rectangle to
            // erase when it changes.
            s32 prev_zone_x[3];
            s32 prev_zone_y[3];
            s32 prev_zone_x1[3];
            s32 prev_zone_y1[3];
        };
    };
    union {
        s32 center0_x;
        // 0x90: Screen x of the view centre, (zone_x + zone_x1) / 2.
        s32 center_x[3];
    };
    union {
        s32 center0_y;
        s32 center_y[3];
    };
    union {
        u8 unknowna8[0x218];
        struct {
            u8 unknowna8_[0x30];
            // 0xd8: Set when the scroll position changed since the last draw.
            u8 scrolled[3];
            // 0xdb: Set when the zone changed since the last draw.
            u8 zone_changed[3];
            // 0xde: Set for a window that shows none of the map
            // (open_new_window's first argument).
            u8 blank[3];
            // 0xe1: blank when the window was last drawn.
            u8 prev_blank[3];
        };
        struct {
            // 0xa8: Scroll velocity, 16.16 fixed point; decays by 0xcd/0x100
            // each turn.
            s32 scroll_vel_x[3];
            s32 scroll_vel_y[3];
            // 0xc0: Fractional scroll carried between turns, 16.16.
            s32 scroll_frac_x[3];
            s32 scroll_frac_y[3];
            u8 fd8_[12];
            // 0xe4: Preset target rectangles (16 used) for open_new_window with
            // source 1.
            s32 preset_x0[20];
            s32 preset_y0[20];
            s32 preset_x1[20];
            s32 preset_y1[20];
            // 0x224: Animation steps left; the zone moves from its start
            // towards anim_to_* by anim_step_* per step.
            s32 anim_steps[3];
            // 0x230: Zone the window opened from.
            s32 open_x0[3];
            s32 open_y0[3];
            s32 open_x1[3];
            s32 open_y1[3];
            s32 anim_to_x0[3];
            s32 anim_to_y0[3];
            s32 anim_to_x1[3];
            s32 anim_to_y1[3];
            s32 anim_step_x0[3];
            s32 anim_step_y0[3];
            s32 anim_step_x1[3];
            s32 anim_step_y1[3];
        };
    };
    union {
        struct {
            u8 anim_mode0;
            u8 unknown2c1[2];
        };
        // 0x2c0: 1 while opening, 2 to remove the window when its animation
        // ends, 0 idle.
        u8 anim_mode[3];
    };
    // 0x2c3: Clip rectangle (x0, y0, x1, y1) for all window drawing; requesters
    // swap in their own.
    s32 clip_x0;
    s32 clip_y0;
    s32 clip_x1;
    s32 clip_y1;
    union {
        u8 unknown2d3[0x18];
        struct {
            u8 unknown2d3_[0x17];
            // 0x2ea: Window drawn through the background screen; the others are
            // drawn without it.
            u8 bscreen_window;
        };
        struct {
            u8 unknown2d3a_[0x14];
            u8 f2e7;
            u8 f2e8;
            u8 f2e9;
        };
    };
    // 0x2eb: Index of the active window.
    u8 current_window;
    // 0x2ec: Number of open windows (at most 2).
    u8 window_count;
    // 0x2ed: 1: open_new_window runs the opening animation at once; 0: animates
    // over 12 turns.
    u8 instant_open;
};

/*
 * What a hospital still lacks to treat an illness, filled by what_is_required
 * (instance required).
 */
struct Required {
    union {
        // 0x0: Byte view: [0] is the low byte of flags, [2] the drug's
        // effectiveness when the illness is already treatable.
        u8 raw[3];
        // 0x0: Skill bits of staff the illness needs and the hospital lacks.
        u16 flags;
    };
    // 0x3: Bit index of the first missing skill (5 for surgeons).
    u8 missing_skill;
    // 0x4: Surgeons still to hire (the illness needs two).
    u8 surgeons_needed;
    // 0x5: The illness can already be treated; nothing else is filled in.
    u8 treatable;
    // 0x6: Room type the illness needs and the hospital lacks.
    u8 missing_room;
    // 0x7: That room type cannot be built yet.
    u8 room_unavailable;
    // 0x8: A ward is needed and missing.
    u8 needs_ward;
    // 0x9: A nurse is needed and missing.
    u8 needs_nurse;
};

struct Debug {
    union {
        u8 unknown0[8];
        struct {
            u32 unknown0_;
            u32 last_button;
        };
        u16 f0;
    };
    u32 last_action;
    // 0xc: Text buffer; complex retries print a message into its second half.
    u8 scratch_text[0xc8];
    u8 screenshot_request;
    u8 unknownd5[2];
    // 0xd7: Suppresses help text and the advisor and keeps the user counted as
    // busy.
    u8 suppress_help;
    u8 advisor_off;
    u8 unknownd9;
    u32 thing_count;
    u8 unknownde[4];
    u8 unknowne2;
    union {
        u8 unknowne3[0x25];
        struct {
            u8 unknowne3_[0x19];
            u8 ffc;
            u8 ffd;
            u8 ffe;
            u8 fff;
            u8 f100;
            u8 f101;
            u8 f102;
            u8 f103;
            u8 f104;
            u8 f105;
            u8 f106;
        };
    };
    u16 unknown108;
    // 0x10a: Save slot from the -G option, used by the Alt+S/Alt+L keys and
    // loaded at start.
    u8 save_slot;
    u8 unknown10b[0xb];
    char msg_text[0xc8];
    u8 msg_timer;
    u8 unknown1df;
    s32 screenshot_number;
    u8 *screenshot_buffer;
    union {
        u8 unknown1e8[0x12];
        struct {
            u8 unknown1e8_[0xd];
            u16 bang_count;
            u16 bang_max;
        };
        struct {
            u8 unknown1e8a_[8];
            u8 nav_debug;
        };
    };
    u8 direct_start;
    u8 unknown1fb[0x1e];
    u8 force_bonus_level;
};

struct RatFestLevel {
    u16 litter_max;
    u16 litter_chance;
    u16 holes_max;
    u16 holes_chance;
    u32 seed;
    u16 staff_max;
    u16 staff_chance;
};

/*
 * Trophy and award thresholds and bonuses (awards_trophies), loaded from the
 * level file's #awards_trophies.<Key> entries; field names follow those keys.
 * Trophies give a cash or reputation bonus; awards compare a year's figures
 * with an Award (good) and a Poor (bad) threshold.
 */
struct TrophyBalance {
    union {
        struct {
            // 0x0: Rats to kill in a year for the rat trophy.
            u8 rat_kills_absolute;
            u8 unknown1[0xd];
            // 0xe: Percentage of shots that must kill a rat for the marksman
            // trophy.
            u8 rat_kills_percentage;
            u8 unknownf[0x54];
        };
        struct {
            u8 unknown0_;
            // 0x1: Reputation bonus of the rat-kills trophy.
            s16 rat_kills_absolute_bonus;
            // 0x3: Drinks to sell in a year for the drinks trophy.
            u8 cans_of_coke;
            // 0x4: Cash bonus of the drinks trophy.
            s16 cans_of_coke_bonus;
            // 0x6: Reputation to stay above all year for the reputation trophy.
            s16 reputation;
            // 0x8: Cash bonus of the reputation trophy.
            s16 trophy_reputation_bonus;
            // 0xa: Mean plant health percentage needed for the plant trophy.
            u8 plant;
            // 0xb: Reputation bonus of the plant trophy.
            u8 plant_bonus;
            // 0xc: Mean staff happiness percentage to keep all year for the
            // staff trophy.
            u8 trophy_staff_happiness;
            // 0xd: Reputation bonus of the staff happiness trophy.
            u8 trophy_staff_happiness_bonus;
            u8 unknowne_;
            // 0xf: Cash bonus of the marksman trophy.
            s16 rat_kills_percentage_bonus;
            // 0x11: Cash bonus of the cures trophy.
            s16 trophy_cures_bonus;
            // 0x13: Cash bonus of the no-deaths trophy.
            s16 trophy_death_bonus;
            // 0x15: Mean mayor score needed for the mayor trophy (low is good).
            u8 trophy_mayor;
            // 0x16: Reputation bonus of the mayor trophy.
            u8 trophy_mayor_bonus;
            // 0x17: Cures above which the cures award is won.
            u8 cures_award;
            // 0x18: Cures below which the poor cures award is given.
            u8 cures_poor;
            // 0x19: Deaths below which the deaths award is won.
            u8 deaths_award;
            // 0x1a: Deaths above which the poor deaths award is given.
            u8 deaths_poor;
            // 0x1b: Share of the town's patients above which the population
            // award is won.
            u8 population_percentage_award;
            // 0x1c: Share of the town's patients below which the poor
            // population award is given.
            u8 population_percentage_poor;
            // 0x1d: Cures-to-deaths ratio above which the award is won.
            u8 cures_v_deaths_award;
            // 0x1e: Cures-to-deaths ratio below which the poor award is given.
            u8 cures_v_deaths_poor;
            // 0x1f: Reputation above which the reputation award is won.
            u16 reputation_award;
            // 0x21: Reputation below which the poor reputation award is given.
            u16 reputation_poor;
            // 0x23: Hospital value above which the value award is won.
            s32 hosp_value_award;
            // 0x27: Hospital value below which the poor value award is given.
            s32 hosp_value_poor;
            // 0x2b: Litter level below which the cleanliness award is won.
            u8 cleanliness_award;
            // 0x2c: Litter level above which the poor cleanliness award is
            // given.
            u8 cleanliness_poor;
            // 0x2d: Percentage of emergency patients saved above which the
            // emergency award is won.
            u8 emergency_award;
            // 0x2e: Percentage of emergency patients saved below which the poor
            // emergency award is given.
            u8 emergency_poor;
            // 0x2f: Mean staff happiness above which the staff happiness award
            // is won.
            u8 staff_happiness_award;
            // 0x30: Mean staff happiness below which the poor staff happiness
            // award is given.
            u8 staff_happiness_poor;
            // 0x31: Mean patient happiness above which the patient happiness
            // award is won.
            u8 peep_happiness_award;
            // 0x32: Mean patient happiness below which the poor patient
            // happiness award is given.
            u8 peep_happiness_poor;
            // 0x33: Waiting time (percent of leave_max) below which the waiting
            // times award is won.
            u8 waiting_times_award;
            // 0x34: Waiting time (percent of leave_max) above which the poor
            // waiting times award is given.
            u8 waiting_times_poor;
            // 0x35: Machine wear below which the well-kept machines award is
            // won.
            u8 well_kept_tech_award;
            // 0x36: Machine wear above which the poor machines award is given.
            u8 well_kept_tech_poor;
            // 0x37: Research points spent in the year above which the research
            // award is won.
            u32 new_tech_award;
            // 0x3b: Research points spent in the year below which the poor
            // research award is given.
            u32 new_tech_poor;
            // 0x3f: Cash bonus of the cures award.
            s16 cures_bonus;
            // 0x41: Cash penalty of the poor cures award.
            s16 cures_penalty;
            // 0x43: Cash bonus of the deaths award.
            s16 deaths_bonus;
            // 0x45: Cash penalty of the poor deaths award.
            s16 deaths_penalty;
            // 0x47: Reputation bonus of the population award.
            s8 population_percentage_bonus;
            // 0x48: Reputation penalty of the poor population award.
            s8 population_pct_penalty;
            // 0x49: Cash bonus of the cures-to-deaths award.
            s16 cures_v_deaths_bonus;
            // 0x4b: Cash penalty of the poor cures-to-deaths award.
            s16 cures_v_deaths_penalty;
            // 0x4d: Bonus of the reputation award.
            s16 award_reputation_bonus;
            // 0x4f: Penalty of the poor reputation award.
            s16 award_reputation_penalty;
            // 0x51: Reputation bonus of the hospital value award.
            s16 hosp_value_bonus;
            // 0x53: Reputation penalty of the poor hospital value award.
            s16 hosp_value_penalty;
            // 0x55: Reputation (bonus, penalty) pairs of the cleanliness,
            // emergency, staff happiness, patient happiness, waiting times,
            // machines and research awards, in that order.
            s8 bonus_penalty[14];
        };
    };
};

/*
 * One row of the level's staff_levels table (vloader.cpp keys staff_levels.*):
 * from the given month on, the job market is topped up to these applicant
 * counts with these specialist chances.
 */
struct StaffLevels {
    // 0x0: Game month (year*12 + month) from which this row applies.
    u16 month;
    // 0x2: Applicants wanted per staff kind: nurses, doctors, handymen,
    // receptionists (copied to gamevar.applicants_wanted).
    u16 counts[4];
    // 0xa: 1-in-N chances that a new doctor is a psychiatrist, surgeon,
    // researcher, consultant or junior (ShrkRate, SurgRate, RschRate, ConsRate,
    // JrRate); 0 never.
    u8 rates[5];
    // 0xf: Level-file key staff_levels.Seed: seeds staffrand
    // (gamevar.staff_seed) when the row takes effect.
    u32 fund;
};

/*
 * Real-time clocks used to pace the game loop (instance gamecounts).
 */
struct GameCounts {
    u8 unknown0[0x14];
    // 0x14: LbTimerClock() at the last game turn; start_turn waits until
    // 1000/game speed ms have passed.
    u32 last_turn_clock;
    // 0x18: LbTimerClock() at the last frame; limit_framerate busy-waits for 40
    // ms between frames.
    u32 last_frame_clock;
};

/*
 * State of one full-screen panel (bank, drugs, town, slist, yearend, ...).
 */
struct Menu {
    // 0x0: Per-screen value: e.g. the hovered save slot, the town map filter
    // bits, the selected staff member, or the year-end countdown.
    u16 value;
    // 0x2: The screen's state machine; 0 makes it (re)initialise.
    u8 state;
    u8 unknown3;
};

struct Req {
    u16 mode;
    u8 id;
    u8 item;
    u16 arg;
    u16 arg2;
    u8 kind;
    // 0x9: Byte argument whose use depends on the requester: the object kind
    // being placed for requester 10, set once the staff view window is open for
    // requester 0xc.
    u8 arg3;
    u8 unknownA;
    u8 unknownB;
};

/*
 * Contents of a levels/level.lN map file, loaded whole into *level by
 * load_level.
 */
struct Level {
    // 0x0: Hospitals the map has plots for (1 to 4).
    u16 player_count;
    // 0x2: Per hospital, the tiles of its four entrance rooms.
    u16 entrances[4][4];
    union {
        u8 unknown22[0x28002];
        struct {
            u8 unknown22_[0x20000];
            // 0x20022: Plot number of each map cell, 0 for unbuyable land.
            u16 plots[0x4000];
        };
        // 0x22: Initial map cells.
        Map tiles[0x4000];
    };
    // 0x28024: Per hospital, the map cell the view starts centred on.
    u16 camera_start[4];
    // 0x2802c: Per hospital, the map cell where emergency patients arrive.
    u16 emergency_tile[4];
    // 0x28034: Map cell of the scroll limit's top corner.
    u16 scroll_min;
    // 0x28036: Map cell of the scroll limit's bottom corner.
    u16 scroll_max;
};

/*
 * The current panel icon table (panel: panel_icons_normal, _over or _ratfest),
 * an array of PanelIcon; this type is used only for the x and y of the entry
 * at 0x62 (icon 7).
 */
struct Icons {
    u8 unknown0[0x62];
    // 0x62: x of panel icon 7; origin of a text window over the panel.
    s16 x;
    // 0x64: y of panel icon 7.
    s16 y;
};

/*
 * Saved choice-box and display state (choice_backup, year_end_backup),
 * restored when the interrupting screen closes.
 */
struct ScreenInfo {
    union {
        u8 unknown0[0xa];
        struct {
            // 0x0: Saved display_mode.
            u16 mode;
            // 0x2: Saved bytes of the current choice box (choice).
            u8 choice_state[4];
            // 0x6: Saved choice_spare_data.
            s16 spare_data;
            // 0x8: Saved choice_time_out.
            s16 time_out;
        };
    };
};

/*
 * Running counts the advisor reads that persist between count passes
 * (NRAdcounts), kept up to date as objects are placed and removed.
 */
struct NonResettingAdvisorCounts {
    // 0x0: Plants placed.
    u16 plants;
    // 0x2: Plants needing water (handyman.cpp).
    u16 thirsty_plants;
    // 0x4: Benches placed.
    u16 benches;
    // 0x6: Up by 1 when a patient who wants to sit finds no free bench, down by
    // 5 when one does.
    s32 bench_shortage;
    // 0xa: Strength of the last earthquake; the advisor quotes half of it on
    // the Richter scale.
    u8 last_quake_size;
    // 0xb: Sum of the plants' health, at most 0x37 per plant; divided by plants
    // for the averages.
    s32 plant_health_sum;
    u8 quake_damage_report;
    // 0x10: Rooms opened and not since deleted.
    u8 open_rooms;
};

/*
 * State of the advisor character and its speech bubble (advisor).
 */
struct Advisor {
    union {
        // 0x0: advice[] entry being shown, 0 for none.
        s16 advice_id;
        // 0x0: Unsigned view of advice_id.
        u16 advice_id_u;
        // 0x0: Byte view of the start of the struct, used to clear it.
        u8 advice_id_bytes[2];
    };
    union {
        // 0x2: Event message to show next (forced_advice_text text), set by game
        // events; 0 for none.
        u16 forced_advice_id;
        // 0x2: Signed view of forced_advice_id.
        s16 forced_advice_id_s;
    };
    // 0x4: Mouse-help message to show (mouse_advice_text text); 0 for none.
    s16 mouse_advice_id;
    // 0x6: Current animation frame of the advisor.
    u16 anim_frame;
    union {
        // 0x8: Screen x of the advisor.
        s16 x;
        // 0x8: Unsigned view of x.
        u16 x_u;
    };
    union {
        // 0xa: Screen y of the advisor.
        s16 y;
        // 0xa: Unsigned view of y.
        u16 y_u;
    };
    union {
        // 0xc: Treatment last priced for a patient, named in the advice about
        // patients who will not pay.
        s16 unpaid_treatment;
        // 0xc: Unsigned view of unpaid_treatment.
        u16 unpaid_treatment_u;
    };
    // 0xe: Turns until the advisor leaves after its message.
    s16 time_left;
    // 0x10: Text of the current message.
    char text[270];
    // 0x11e: Animation state (advisor_anims, next_advisor_state index).
    u8 anim_state;
    // 0x11f: Bit 0: tutorial running; bit 1: the cursor tile is invalid for
    // building; bit 2: the room is too small.
    u8 flags;
    // 0x120: Control the tutorial is pointing at (panel button 0x10..0x1b, or a
    // requester control).
    u16 tutorial_highlight;
    // 0x122: Lines of wrapped text in the speech bubble.
    u8 line_count;
    // 0x123: Speech bubble segments beyond the minimum width.
    u8 extra_segments;
    // 0x124: Set when new text must be wrapped into the bubble.
    u8 rewrap;
    union {
        // 0x125: Left x of the speech bubble.
        s16 box_left;
        // 0x125: Unsigned view of box_left.
        u16 box_left_u;
    };
    // 0x127: Set when an emergency helicopter landed, for the matching advice.
    u8 heli_landed;
    union {
        // 0x128: Events since the last advice: 1 staff resigning, 2 a patient
        // would not pay, 4 new illness or treatment, 0x20 balance under 7/8 of
        // the start cash, 0x40 cure, 0x80 death; find_advice clears what it
        // reports.
        u8 event_flags;
        // 0x128: Dword view of event_flags.
        u32 event_flags_u32;
        // 0x128: Byte view of event_flags.
        u8 event_flag_bytes[4];
    };
    // 0x12c: Set once the idle advisor stepped to its next animation state;
    // cleared on each state change.
    u8 state_advanced;
    // 0x12d: Set when a new member of staff started work in a room, for the
    // matching advice.
    u8 new_staff_pending;
    // 0x12e: That staff member's kind.
    u8 new_staff_kind;
    // 0x12f: That room's type.
    u8 new_staff_room;
    // 0x130: Set by the patient count pass; patient-based advice waits for it.
    u8 has_patients;
    // 0x131: Current tutorial step.
    u8 tutorial_step;
    u8 unknown132[0x2];
};

struct SampleInfo;

/*
 * Grab-bag of global game and UI state (instance othervar): screen and
 * requester state, help text, mouse and drag state, sound and music state,
 * level loading, net results. Zeroed at level start (start.cpp) except where
 * saved.
 */
struct General {
    // 0x0: Random 0..99 rolled each turn by start_turn; staff code re-rolls it
    // before each test.
    s16 rand_pct;
    u8 unknown2[0xc];
    // 0xe: Random 0..5 rolled each turn: a patient this many tiles from a queue
    // counts as arrived.
    u8 queue_near_dist;
    // 0xf: Player whose commands or blueprints are being processed.
    u8 cur_player;
    // 0x10: Set to force a full redraw of the main view on the next frame.
    u8 full_redraw;
    u8 unknown11[0x5];
    u8 f16;
    // 0x17: Set when the blueprint drawing marked a tile invalid.
    u8 blueprint_has_invalid;
    // 0x18: Per player: the door position being placed for the blueprint is
    // invalid.
    u8 door_invalid[4];
    // 0x1c: Set while drawing goes into the pixel buffer (DrawIntoPixBuffer).
    u8 drawing_pixbuf;
    u8 unknown1d[0x2];
    // 0x1f: Palette to fade to on the next frame, or null.
    u8 *fade_palette;
    // 0x23: Set to fade the screen to black on the next frame.
    u8 fade_to_black;
    // 0x24: Palette to switch to (without fading) on the next frame, or null.
    u8 *pending_palette;
    // 0x28: Map cell (x + y * 128) under the mouse.
    u16 mouse_cell;
    // 0x2a: mouse_cell before its last update.
    u16 prev_mouse_cell;
    // 0x2c: Screen mode to switch to on the next frame, or 0.
    TbScreenMode pending_screen_mode;
    // 0x2d: Graphs screen period: 0 months, 1 years, 2 four-year steps.
    u16 graph_period;
    // 0x2f: Random 1..4 rolled each turn: a person this many tiles from an
    // object counts as arrived.
    u8 object_near_dist;
    // 0x30: Index of the current mouse pointer sprite.
    u8 pointer_sprite;
    // 0x31: Pointer sprite before the last set_mouse_sprite.
    u8 prev_pointer_sprite;
    // 0x32: Set on turns that are a multiple of 16.
    u8 every_16_turns;
    u8 f33;
    // 0x34: Per info-requester slot (indexed 0..3 past its declared size):
    // window to re-centre on the slot's tracked thing.
    u8 track_window[1];
    // 0x35: track_window entry for slot 1.
    u8 track_window_1;
    // 0x36: track_window entry for slot 2.
    u8 track_window_2;
    u8 unknown37[0x1];
    // 0x38: Info-requester slot whose thing the main view should centre on, or
    // 0.
    u8 center_on_slot;
    // 0x39: Illness most recently passed on by contagion.
    u16 last_spread_illness;
    // 0x3b: Bank balance as currently shown; rolls toward the real balance.
    s32 shown_balance;
    // 0x3f: Step (0..3) of the balance roll animation.
    u8 balance_roll_step;
    // 0x40: Index (across all pulldowns) of the pulldown item last clicked.
    u8 clicked_pulldown_item;
    union {
        // 0x41: Thing under the mouse, found while drawing.
        u16 pointed_thing;
        // 0x41: Signed view of pointed_thing.
        s16 pointed_thing_s;
    };
    // 0x43: Pointed thing that can be picked up or edited; drawn highlighted.
    u16 pickup_thing;
    // 0x45: Person at queue_pos in the queue window; drawn highlighted.
    u16 queue_hilight_thing;
    u8 unknown47[0x2];
    // 0x49: Staff list kinds shown: 1 doctors, 2 nurses, 4 handymen, 8
    // receptionists.
    u8 slist_filter;
    // 0x4a: Set while the player is typing a network message.
    u8 net_text_entering;
    u8 f4b;
    // 0x4c: Bitmask of players the network message goes to.
    u8 net_text_to;
    union {
        // 0x4d: 16.16 x position of the scrolling received network text.
        s32 net_text_scroll;
        struct {
            u16 unknown4d;
            // 0x4f: Integer part of net_text_scroll.
            s16 net_text_x;
        };
    };
    u8 unknown51[0x10];
    // 0x61: room_info index of the room shown in the queue window.
    u16 queue_win_room;
    // 0x63: Selected position in that room's queue, or 0.
    u16 queue_pos;
    // 0x65: Map cell of the thing clicked to open the queue window, or 0.
    u16 queue_win_cell;
    // 0x67: Queue window drag: 0 none, 1 dragging a person, 2 dragging the
    // queue bar.
    u8 queue_drag;
    // 0x68: Mouse mode saved while the pointer is over a window.
    u8 saved_mouse_mode;
    // 0x69: Set when the town overhead map must be redrawn.
    u8 town_map_dirty;
    // 0x6a: Heating level last requested on the town screen.
    u8 pending_heating_level;
    union {
        s32 f6b;
        u32 f6b_u32;
    };
    union {
        // 0x6f: Rat under the mouse, or 0.
        u16 pointed_rat;
        // 0x6f: Signed view of pointed_rat.
        s16 pointed_rat_s;
    };
    // 0x71: Set when the mouse is near a rat but not on it.
    u8 near_rat;
    // 0x72: Level outcome to show: 1 won, 2 lost, 6 left the network game.
    u16 winlose_result;
    union {
        // 0x74: Money offered for the next level after a win; becomes the next
        // hospital's f2d.
        u32 next_level_offer;
        // 0x74: Signed view of next_level_offer.
        s32 next_level_offer_s;
    };
    // 0x78: Plot number being bid for.
    u8 bid_plot;
    // 0x79: Cycling index into hilight_plot.
    u8 hilight_plot_idx;
    // 0x7a: Contagious patient under the mouse that can be vaccinated, or 0.
    u16 vaccine_target;
    // 0x7c: What the bottom panel describes: 0 none, 1 staff, 2 patient, 3
    // object, 4 room, 5 mayor or inspector, 6 machine usage.
    u8 panel_info;
    // 0x7d: panel_info before its last change.
    u8 prev_panel_info;
    // 0x7e: Thing described in the bottom panel.
    u16 panel_thing;
    // 0x80: Frames left before panel_info reverts to 0 after the mouse leaves.
    u16 panel_info_timer;
    // 0x82: Current game turn (actual_game_turn), copied each turn.
    u16 cur_turn;
    // 0x84: Turn at which a person is next rebalanced (cur_turn + gbv
    // ModifyFreq).
    u16 next_modify_turn;
    // 0x86: Set on turns that are a multiple of 32; gates the advisor counts.
    u8 every_32_turns;
    // 0x87: Number of contagious patients from spreading.
    u16 contagious_count;
    // 0x89: Own patients who left through impatience since the advisor last
    // said so.
    u8 patients_left_count;
    // 0x8a: Set while the mouse is over the menu bar or a pulldown is open.
    u8 menu_bar_active;
    // 0x8b: Set once the pointer was switched to the arrow for the menu bar.
    u8 menu_arrow_set;
    // 0x8c: Expertise shown on the drugs screen.
    u16 selected_illness;
    // 0x8e: Number of level balance files found.
    u8 balance_file_count;
    // 0x8f: arg of the pulldown item being run.
    s8 pulldown_arg;
    // 0x90: Index of the pulldown item being run.
    u8 pulldown_item;
    // 0x91: Page of 10 rows shown in the staff list.
    u8 slist_page;
    // 0x92: Clock when the current help hotspot was first hovered.
    u32 help_hover_start;
    // 0x96: Text of the help message box.
    char help_msg[2048];
    // 0x896: Help box left.
    s16 help_x;
    // 0x898: Help box top.
    s16 help_y;
    // 0x89a: Help box width.
    s16 help_w;
    // 0x89c: Help box height.
    s16 help_h;
    // 0x89e: Clock when the help box was last shown, 0 when cancelled.
    u32 help_shown_time;
    union {
        // 0x8a2: LbTimerClock() at the start of the frame.
        u32 clock_ms;
        // 0x8a2: Signed view of clock_ms.
        s32 clock_ms_s;
    };
    // 0x8a6: Help hotspot group of the current help.
    u8 help_group;
    // 0x8a7: Box style of the current help.
    u8 help_style;
    // 0x8a8: x of the close icon of the composite help box.
    s16 help_close_x;
    // 0x8aa: y of the close icon of the composite help box.
    s16 help_close_y;
    // 0x8ac: Current help hotspot (resvars.help_hotspots index), 0 none.
    u16 help_hotspot;
    // 0x8ae: Countdown decremented each turn while positive; cleared when a
    // receptionist finishes or abandons calling a patient.
    s16 recept_call_timer;
    // 0x8b0: Set while the fast-forward key is held; turns run without the
    // speed delay.
    u8 turbo_key;
    // 0x8b1: No windows or screens are open, so requesters may pop up.
    u8 user_idle;
    // 0x8b2: Clock when user_idle last became set.
    u32 idle_since;
    u8 f8b6;
    // 0x8b7: Build palette counts saved on entering the shop.
    Blob63 saved_build_palette;
    // 0x8f6: Per object kind: count taken off the build palette while in the
    // shop.
    u8 shop_palette_adjust[63];
    // 0x935: Text of the yes/no requester.
    char yesno_msg[105];
    // 0x99e: Command type sent when yes is pressed.
    u8 yes_type;
    // 0x99f: Command value sent when yes is pressed.
    u16 yes_value;
    u8 unknown9a1[0x1];
    // 0x9a2: Command arg sent when yes is pressed.
    u16 yes_arg;
    u8 unknown9a4[0x9];
    // 0x9ad: Command type sent when no is pressed.
    u8 no_type;
    // 0x9ae: Command value sent when no is pressed.
    u16 no_value;
    u8 unknown9b0[0x1];
    // 0x9b1: Command arg sent when no is pressed.
    u16 no_arg;
    u8 unknown9b3[0x4];
    // 0x9b7: Set once the win criteria are met.
    u8 level_won;
    // 0x9b8: Slot of the open info requester, or 0.
    u8 irequester_slot;
    // 0x9b9: Set while a MIDI tune is playing.
    u8 midi_playing;
    // 0x9ba: midi_files index of the current tune.
    u8 midi_track;
    // 0x9bb: Set while adjust_mapwho re-places map objects; can_place_object
    // then skips its checks.
    u8 force_place_objects;
    // 0x9bc: Frame counter for flashing the button the advisor points at;
    // redrawn every 10th frame.
    u8 flash_counter;
    // 0x9bd: HelpHotspot index shown regardless of the mouse (set_force_help);
    // 0 = none.
    u16 forced_help_hotspot;
    // 0x9bf: Help group in which forced_help_hotspot applies.
    u8 forced_help_group;
    // 0x9c0: Text index (misc_text) of the hint in the low-resolution
    // edit-room requester.
    u16 edit_room_hint;
    // 0x9c2: Panel popup icon under the mouse; 0 = none.
    u8 hovered_popup_icon;
    // 0x9c3: Requester to reopen when the yes/no box closes (0x63 = none).
    u8 yesno_saved_requester;
    // 0x9c4: Mouse mode to restore when the yes/no box closes.
    u8 yesno_saved_mouse_mode;
    // 0x9c5: Set while an existing room is being edited; open_room clears it.
    u8 editing_room;
    // 0x9c6: Tile bounds of the object drag shape (0xff/0 when empty).
    u8 drag_x0;
    // 0x9c7: See drag_x0.
    u8 drag_y0;
    // 0x9c8: See drag_x0.
    u8 drag_x1;
    // 0x9c9: See drag_x0.
    u8 drag_y1;
    u8 f9ca;
    u16 f9cb;
    u16 f9cd;
    u8 unknown9cf[0x2];
    // 0x9d1: Months since the start of the level (year * 12 + month); compared
    // with level-file month fields.
    u16 game_month;
    u8 unknown9d3[0x4];
    // 0x9d7: lbDisplay.wscreen saved while drawing to another buffer.
    u8 *saved_screen_ptr;
    // 0x9db: lbDisplay.graphics_screen_width saved alongside saved_screen_ptr.
    s32 saved_screen_width;
    union {
        // 0x9df: lbDisplay.graphics_screen_height saved alongside
        // saved_screen_ptr.
        s32 saved_screen_height;
        // 0x9df: Unsigned view of saved_screen_height.
        u32 saved_screen_height_u;
    };
    u8 unknown9e3[0x1];
    // 0x9e4: 1 while the hall of fame (not shame) is on screen.
    u8 showing_fame;
    // 0x9e5: Pixels added around a rat's box for a near miss (sets f71).
    u16 rat_near_margin;
    // 0x9e7: Pixels around a rat's sprite box that still count as a hit;
    // widened by rat power-ups.
    u16 rat_hit_margin;
    // 0x9e9: If nonzero, layer 0 for the next rat made (a power-up rat);
    // cleared on use.
    u8 next_rat_layer;
    u8 unknown9ea[0x1];
    // 0x9eb: mmouse_x + mmouse_y at the last check, to detect mouse movement.
    s16 last_mouse_sum;
    // 0x9ed: Frames without mouse movement left before a screen advances on its
    // own.
    u16 idle_countdown;
    // 0x9ef: display_mode to return to after the town bid screen; 0 = return to
    // the game.
    u16 town_bid_return_mode;
    // 0x9f1: Step for held +/- buttons (drug price, research allocation), 8.8
    // fixed point; grows while held, 0x100 after release.
    u16 repeat_step;
    // 0x9f3: Clock (f8a2) at the last held-button step.
    u32 repeat_time;
    // 0x9f7: Palette colour of the bank manager's text underline.
    u8 manager_line_colour;
    // 0x9f8: Palette colour of its shadow line.
    u8 manager_line_shadow_colour;
    // 0x9f9: Level (or picture map) currently held in level_block; 0 = none.
    u8 loaded_level;
    u8 unknown9fa[0xc];
    // 0xa06: Cache block holding the loaded level file or in-game picture.
    TbCacheUsedBlock *level_block;
    u8 unknowna0a[0x4];
    // 0xa0e: 1 per save slot whose file exists.
    u8 save_slot_used[11];
    // 0xa19: Pulldown radio ticks for the sound-effect volume (10 steps).
    u8 sfx_volume_checked[11];
    // 0xa24: Pulldown radio ticks for the ambient volume.
    u8 ambient_volume_checked[11];
    // 0xa2f: Pulldown radio ticks for the music volume.
    u8 music_volume_checked[11];
    // 0xa3a: Game speed radio ticks, used as a 5-entry array: [0] slowest, then
    // the four fields that follow.
    u8 speed_checked[1];
    // 0xa3b: speed_checked[1]: slower.
    u8 speed_checked_slower;
    // 0xa3c: speed_checked[2]: normal.
    u8 speed_checked_normal;
    // 0xa3d: speed_checked[3]: max speed.
    u8 speed_checked_max;
    // 0xa3e: speed_checked[4]: fastest (frame skipping).
    u8 speed_checked_and_more;
    u8 unknowna3f[0x1];
    // 0xa40: Resolution radio tick: low resolution.
    u8 lores_checked;
    // 0xa41: Resolution radio tick: high resolution.
    u8 hires_checked;
    // 0xa42: Sound id of the ambient sample playing; 0 = none.
    u8 ambient_playing;
    // 0xa43: Calls since the last ambient sound; a random one plays after 2000.
    u16 ambient_timer;
    // 0xa45: Sample of the ambient sound playing.
    SampleInfo *ambient_sample;
    // 0xa49: 1 = normal panel icons, 2 = mouse-over icons; a change forces a
    // panel redraw.
    u8 panel_icon_set;
    // 0xa4a: Win-criteria group closest to completion (player only).
    u8 best_win_group;
    union {
        // 0xa4b: Room type the player last built, for room-specific advice;
        // cleared once advised.
        u8 last_built_room;
        // 0xa4b: Signed view of last_built_room.
        s8 last_built_room_s;
    };
    // 0xa4c: Frame index into bank_manager_face1; 0 = idle.
    u8 manager_face1_frame;
    // 0xa4d: Frame index into bank_manager_face2; 0 = idle.
    u8 manager_face2_frame;
    // 0xa4e: Frame index into bank_manager_face3; 0 = idle.
    u8 manager_face3_frame;
    // 0xa4f: 1 while a MIDI track is playing (not the music option itself).
    u8 music_playing;
    // 0xa50: Jukebox loop button: replay the current track instead of moving
    // on.
    u8 loop_track;
    // 0xa51: Copy of the emergency watch countdown when it starts; not read.
    u8 emergency_timer_start;
    // 0xa52: Set once the icon element of the thing being drawn has been
    // handled; cleared after each thing.
    u8 frame_icon_drawn;
    // 0xa53: Right button pressed over the advisor; turned into
    // dismiss_advisor.
    u8 advisor_right_click;
    // 0xa54: Ends the current advice and the advisor's animation.
    u8 dismiss_advisor;
    // 0xa55: Panel popup shutter: 1 closed, 2 opening, 3 open, 4 closing.
    u8 popup_shutter_state;
    // 0xa56: Shutter segments drawn (0 open .. 0x18 closed).
    u8 popup_shutter_width;
    // 0xa57: Frames the shutter has been open; closes after 0x28.
    u8 popup_shutter_timer;
    // 0xa58: Index into panel_emergency_spr for the emergency popup icon
    // animation.
    u8 emergency_anim_frame;
    // 0xa59: Rats killed, for the rat-kills trophy plaque text.
    u8 plaque_rat_kills;
    // 0xa5a: Percentage of rats killed, for the rat-percentage trophy plaque
    // text.
    u8 plaque_rat_kill_pct;
    // 0xa5b: Rows drawn on the current staff list page.
    u8 slist_rows;
    // 0xa5c: Show the level briefing help page when possible.
    u8 show_level_brief;
    // 0xa5d: Show the current network criterion help page when possible.
    u8 show_net_criteria;
    // 0xa5e: Network result help page (sub-page of page 3) to show; 0 = none.
    u8 net_result_page;
    // 0xa5f: Set when the player keeps playing after winning; skips one
    // quarterly win check.
    u8 win_declined;
    // 0xa60: Selected staff list row, 1-based across pages (page * 10 + row +
    // 1); 0 = none.
    u8 slist_cursor;
    // 0xa61: Right mouse button state in the previous requester frame.
    u8 right_button_was_down;
    // 0xa62: Left mouse button state in the previous requester frame.
    u8 left_button_was_down;
    // 0xa63: Extra game turns to run this frame, beyond two.
    u16 extra_turns;
    // 0xa65: A full-screen picture with its own pointers is loaded
    // (load_ingame_pic).
    u8 screen_pic_loaded;
    // 0xa66: Text index (misc_text) shown in the panel info bar, e.g. save or
    // load result; 0 = none.
    u8 panel_message;
    // 0xa67: Draws left before panel_message is cleared.
    u16 panel_message_time;
    // 0xa69: Frames the info bar stays up after pausing or resuming.
    u16 panel_info_hold;
    // 0xa6b: Mouse is over the emergency/epidemic watch (sound plays on entry).
    u8 watch_hovered;
    // 0xa6c: Per player, 1 if they met the last network criterion.
    u8 net_winners[6];
    // 0xa72: People drawn this turn; sets the crowd noise volume.
    u16 visible_people;
    // 0xa74: Volume last set on the crowd sample.
    u8 crowd_volume;
    // 0xa75: Level to continue with after the rat bonus level.
    u8 level_after_bonus;
    // 0xa76: Bonus level (13 or 14) to play next; 0 = none.
    u8 bonus_level;
    // 0xa77: Shots left with the current rat power-up.
    u8 rat_powerup_shots;
    // 0xa78: Frames the drugs screen shows the percentage instead of the price.
    u16 drug_pct_display_time;
    // 0xa7a: Volume last set on the emergency clock sound.
    u8 emergency_clock_volume;
    // 0xa7b: Per player: 0 active, 1 or 2 when the game window lost focus
    // (network game).
    u8 lost_focus[4];
    // 0xa7f: display_mode to restore when focus returns.
    u16 lost_focus_display_mode;
    // 0xa81: Frames to do full screen swaps instead of special_refresh after
    // focus returns.
    u8 full_swap_frames;
    // 0xa82: Staff member whose list entry the staff list opens on.
    u16 slist_staff;
    // 0xa84: Set when a research category has nothing left; cleared by the
    // research screen.
    u8 research_exhausted;
    u16 fa85;
    // 0xa87: CD speed from GetCDSpeed; picks the intro movie.
    u8 cd_speed;
    // 0xa88: Last emergency patient viewed from the watch.
    u16 emergency_patient;
    // 0xa8a: Frames the heating icon stays on the town map.
    u8 heat_icon_time;
    // 0xa8b: Calls since the last random background noise; one plays after 400.
    u16 ambient_noise_timer;
    // 0xa8d: Use low-resolution faces and money digits; never set by the game
    // code.
    u8 use_lores_assets;
    // 0xa8e: Cheat keys enabled (fax cheat).
    u8 cheats_enabled;
    u8 unknowna8f[0x1];
    // 0xa90: Last contagious patient viewed from the epidemic watch.
    u16 epidemic_patient;
    // 0xa92: Page of the help text shown (show_help_page).
    u8 help_page;
    // 0xa93: Sub-page of the help text shown.
    u8 help_sub;
    // 0xa94: Network game result to resend each turn: 1 lost, 2 won.
    u8 net_result;
    // 0xa95: The player won the network game.
    u8 net_won;
};

/*
 * Level balance variables (gbv), loaded from the level file's #gbv.<Key>
 * entries through balance_variables[] in vloader.cpp; field names follow those
 * keys.
 */
struct Balance {
    u8 unknown0[0x7c];
    u16 land_cost_per_tile;
    u8 unknown7e[0x3a];
    // 0xb8: SalaryAdd[3..8]: salary added for junior, doctor, surgeon,
    // psychiatrist, consultant and researcher skills (the level key's array
    // starts at 0xb2).
    s16 salary_add[6];
    u16 salary_ability_divisor;
    u8 unknownc6[2];
    u8 start_rating;
    u8 unknownc9;
    u16 start_cost;
    u8 unknowncc[2];
    u16 how_contagious;
    u16 reduce_cont_months;
    u16 reduce_cont_peep_count;
    u16 reduce_cont_rate;
    u16 hold_visual_months;
    u16 hold_visual_peep_count;
    s16 rest_standing;
    s16 rest_sofa;
    s16 rest_game;
    s16 rest_snooker;
    u16 work_light;
    u16 modify_freq;
    u16 not_tired;
    u16 tired;
    u16 very_tired;
    u16 crack_up_tired;
    u16 recovery_factor;
    u16 recovery_minimum;
    u16 max_object_strength;
    u16 research_increment;
    u16 training_value[3];
    // 0xfc: Diagnosis progress at which a patient counts as diagnosed;
    // percentages of diagnosis are taken against it.
    u16 diagnosis_complete;
    s16 diagnosis_hopeless;
    u16 q_points;
    u16 dist_points;
    u16 no_staff_points;
    s16 litter_drop;
    s16 leave_max;
    s16 resign_max;
    s16 happy;
    s16 unhappy;
    s16 very_unhappy;
    s16 drink_happy;
    s16 toilet_happy;
    s16 bowel_full;
    s16 bowel_overflows;
    s16 vomit_limit;
    u16 litter_random;
    u16 salary_too_low;
    u16 salary_too_high;
    u8 unknown122[2];
    u16 score_max_inc;
    u16 vac_cost;
    u16 mayor_launch;
    u8 unknown12a[2];
    u16 disaster_launch;
    u16 epidemic_fine;
    u16 epidemic_comp_lo;
    u16 epidemic_comp_hi;
    u8 unknown134[4];
    u16 autopsy_rsch_percent;
    u8 unknown13a[2];
    u8 alloc_rand;
    u8 alloc_total_rep;
    u8 alloc_ind_rep;
    u8 alloc_delay;
    u8 drug_improve_rate;
    u16 training_rate;
    u8 promo_doc;
    u8 promo_con;
    u8 contagious_spread_factor;
    // 0x146: Training progress needed to qualify as a surgeon
    // (AbilityThreshold[0]).
    u16 ability_threshold_surgeon;
    // 0x148: Training progress needed to qualify as a psychiatrist
    // (AbilityThreshold[1]).
    u16 ability_threshold_psych;
    // 0x14a: Training progress needed to qualify as a researcher
    // (AbilityThreshold[2]).
    u16 ability_threshold_research;
    // 0x14c: Walking speed given to new patients.
    u8 peep_speed;
    u8 unknown14d;
    // 0x14e: Animation speed given to new patients.
    u8 peep_anim_speed;
    u8 unknown14f;
    // 0x150: Percent chance that a tile near an exploding litter bomb gets
    // litter.
    u8 bomb_litter_chance;
    // 0x151: Largest ring radius a litter bomb spreads litter over.
    u8 bomb_litter_radius;
    // 0x152: Turns per fuse step of a litter bomb; it explodes after 8 steps.
    u8 bomb_fuse_ticks;
    // 0x153: Pieces of litter one litter bomb scatters.
    u8 bomb_litter_count;
    // 0x154: Turns before a new hospital opens to patients (the panel watch); 0
    // opens it at once.
    u16 open_countdown;
    u16 doctor_threshold;
    u16 consultant_threshold;
    u8 rsch_improve_cost_percent;
    u8 rsch_improve_inc_percent;
    u8 unknown15c[2];
    u16 ins_control_base[1];
};

/*
 * Figures for the year's awards and trophies (award_counts): monthly arrays
 * indexed by month, the rest totals reset each year.
 */
struct AwardCounts {
    // 0x0: Monthly mean staff happiness (scaled by 256).
    s32 staff_happiness[13];
    // 0x34: Monthly mean patient happiness (scaled by 256).
    s32 peep_happiness[13];
    // 0x68: Monthly mean plant health.
    s32 plant_health[13];
    // 0x9c: Monthly mean patient waiting time (scaled by 256).
    s32 waiting_times[13];
    // 0xd0: Monthly machine wear sum.
    s32 machine_wear[13];
    // 0x104: Monthly machine count.
    s32 machines[13];
    // 0x138: Drinks sold this year.
    u16 cans_of_coke;
    // 0x13a: Sum of this year's mayor scores.
    s32 mayor_score_total;
    // 0x13e: Mayor visits this year.
    u8 mayor_visits;
    // 0x13f: Emergencies this year; the emergency award needs more than 2.
    u8 emergencies;
    // 0x140: Percentage of emergency patients saved this year.
    s32 emergency_saved_pct;
    // 0x144: Set when a machine blew up; forces the poor machines award.
    u8 machine_exploded;
    // 0x145: Research points researchers produced this year.
    s32 research_earned;
    // 0x149: Research points allocated to research this year.
    s32 research_spent;
    // 0x14d: Rats killed this year.
    s32 rats_killed;
    // 0x151: Shots fired at rats this year.
    s32 rat_shots;
};

/*
 * One network-game objective (net_criteria[], and current_net_criteria for the
 * one in force), loaded from the level file's net_criteria keys.
 */
struct WinLoseNet {
    // 0x0: Objective kind (Criteria key); 0 ends the list.
    u16 criterion;
    // 0x2: 1 once the objective was taken from the list or achieved.
    u16 done;
    // 0x4: Value to reach.
    s32 value;
    // 0x8: Month the objective starts (Month key); in current_net_criteria, the
    // month it must be met by.
    u16 start;
    // 0xa: Months allowed (TimeToDo key); in current_net_criteria for kind 4,
    // the cure count when the objective started.
    u16 time_to_do;
};

/*
 * One trophy of the year (trophy[], trophies_won[]).
 */
struct Trophies {
    // 0x0: Trophy number, 0 for none; 2 when a reputation bonus had to be paid
    // as cash.
    u8 id;
    // 0x1: Index of the trophy's text.
    u8 text_index;
    // 0x2: Cash or reputation change given.
    s16 value;
    // 0x4: 0 reputation, 1 cash: index of the reward text.
    u8 reward_kind;
};

/*
 * One award of the year (all_awards[], awards_won[]).
 */
struct Awards {
    // 0x0: 1 for an award won, -1 for a poor award, 0 for none.
    u8 direction;
    // 0x1: Award number.
    u8 id;
    // 0x2: Index of the award's title text.
    u8 text_index;
    // 0x3: Cash or reputation change given.
    s16 value;
    // 0x5: 0 reputation, 1 cash: index of the reward text.
    u8 reward_kind;
};

struct ResearchTasks {
    u16 chance;
    u16 object;
    u16 action;
};

/*
 * Counts of the player's patients, staff and machines, rebuilt every count
 * pass (adcounts) for the advisor, awards and mayor.
 */
struct AdvisorCounts {
    union {
        u8 unknown0[0x8e];
        struct {
            u8 unknown0a_[4];
            // 0x4: Sum of staff happiness.
            u16 staff_happiness_sum;
        };
        struct {
            u8 unknown0s_[4];
            // 0x4: Signed view of staff_happiness_sum.
            s16 staff_happiness_sum_s;
        };
        struct {
            // 0x0: Patients counted (starts at 1 so it can divide).
            u16 patients;
            // 0x2: Sum of staff tiredness.
            s16 staff_tiredness_sum;
            u8 unknown4_[6];
            // 0xa: Sum of patient happiness.
            s16 happiness_sum;
            u8 unknownc_[4];
            // 0x10: Sum of patient refreshment.
            s16 refreshment_sum;
            u8 unknown12_[2];
            // 0x14: Sum of patient warmth.
            s16 warmth_sum;
            // 0x16: Sum of staff warmth.
            s16 staff_warmth_sum;
            u8 unknown18_[4];
            // 0x1c: Sum of patient bladder.
            s16 bladder_sum;
            u8 unknown1e_[2];
            // 0x20: Consultants counted.
            u8 consultants;
            // 0x21: Researchers counted.
            u8 researchers;
            u8 unknown22_[0x26];
            // 0x48: Sum of patient impatience.
            u16 impatience_sum;
            // 0x4a: Sum of patient waiting time.
            s32 waiting_sum;
            // 0x4e: wait_sums[0], read by the advice about too few nurses.
            s32 nurse_wait_sum;
            // 0x52: wait_sums[1], read by the advice about too few doctors.
            s32 doctor_wait_sum;
            u8 unknown56_[0xc];
            // 0x62: wait_counts[0].
            s32 nurse_wait_count;
            // 0x66: wait_counts[1].
            s32 doctor_wait_count;
            u8 unknown6a_[0x10];
            // 0x7a: Sum of tiredness per staff kind (STAFF_*).
            s16 staff_tiredness_by_kind[4];
            u8 unknown82_[0xc];
        };
        struct {
            u8 unknown0e_[6];
            // 0x6: Doctors (including surgeons) and nurses counted.
            u16 doctors_and_nurses;
            // 0x8: Highest staff tiredness.
            s16 max_staff_tiredness;
            u8 unknownae_[2];
            // 0xc: Highest patient happiness.
            s16 max_happiness;
            // 0xe: Lowest patient happiness (starts at 999).
            s16 min_happiness;
            u8 unknown10e_[2];
            // 0x12: Lowest patient refreshment (starts at 999).
            s16 min_refreshment;
            u8 unknown14e_[4];
            // 0x18: Highest patient warmth.
            s16 max_warmth;
            // 0x1a: Lowest patient warmth (starts at 999).
            s16 min_warmth;
            u8 unknown1ce_[2];
            // 0x1e: Highest patient bladder.
            s16 max_bladder;
            u8 unknown20e_[2];
            // 0x22: Psychiatrists counted.
            u8 psychiatrists;
            // 0x23: Surgeons counted.
            u8 surgeons;
            // 0x24: Patients counted per illness.
            u8 illness_counts[0x26];
            u8 unknown4ae_[4];
            // 0x4e: Per-staff-kind patient waiting sums; person_count adds
            // every patient to all five.
            s32 wait_sums[5];
            // 0x62: Per-staff-kind patient counts; person_count adds every
            // patient to all five.
            s32 wait_counts[5];
            u8 unknown76e_[0xe];
            // 0x84: Sum of happiness per staff kind (STAFF_*).
            s16 staff_happiness_by_kind[4];
        };
    };
    // 0x8e: Doctors who cracked up since the advisor last said so (not cleared
    // by the count pass).
    u8 crackups;
    union {
        u8 unknown8f[0x19];
        struct {
            // 0x8f: Machines counted.
            u8 machines;
            // 0x90: Sum of machine wear (times_used * 512 / (strength * 3)).
            s32 machine_wear_sum;
            u8 unknown94_[4];
            // 0x98: Object kind of that machine.
            u8 worst_machine;
            // 0x99: idle_staff[STAFF_NURSE].
            u8 idle_nurses;
            // 0x9a: idle_staff[STAFF_DOCTOR].
            u8 idle_doctors;
            u8 unknown9b_[3];
            // 0x9e: Overall win-criteria progress from evaluate_criteria.
            s32 win_progress;
            // 0xa2: Overall lose-criteria progress from losing().
            s32 lose_progress;
            u8 unknowna6_[2];
        };
        struct {
            u8 unknown8fe_[5];
            // 0x94: Highest wear of a machine past its strength.
            s32 max_machine_wear;
            u8 unknown98e_;
            // 0x99: Staff per kind wandering or in a room with nobody queueing.
            u8 idle_staff[4];
        };
    };
};

struct PathStep {
    u8 x;
    u8 y;
};

/*
 * One win or lose criterion of the level (win_criteria[], lose_criteria[]),
 * loaded from the level file's win_criteria/lose_criteria keys.
 */
struct WinLose {
    // 0x0: What is measured: 1 reputation, 2 balance (minus the loan when
    // winning), 3 and 5 percentages of the patients seen, 4 cures, 6 hospital
    // value; 0 is always met.
    u16 criterion;
    // 0x2: MaxMin key: 1 met when the value is at least value, 0 when it is at
    // most value.
    u16 op;
    // 0x4: Value to reach.
    s32 value;
    // 0x8: Bound key: the value at which progress counts as 0%.
    s32 base;
    // 0xc: Criteria with the same group must all be met together.
    u16 group;
    // 0xe: 1 once the measured value reached value (check_criteria_values).
    u16 met;
    // 0x10: The value measured at the last check.
    s32 current;
    // 0x14: Percent progress from base to value, clamped to 0..200.
    s16 progress;
};

/*
 * Score growth per criteria-progress band (rating[]): the first entry whose
 * threshold reaches the score applies.
 */
struct ScoreRatings {
    // 0x0: Upper bound of criteria progress for this entry; 0 ends the table.
    s16 threshold;
    // 0x2: Score growth in hundredths of a percent.
    u16 growth;
    // 0x4: Score growth used once the score is over 1000000 or over
    // score_max_inc percent of the hospital's reference value.
    u16 capped_growth;
};

/*
 * An insurance company (insurance_co[], indexed through ins_control) that pays
 * for its patients' treatment.
 */
struct InsuranceCo {
    // 0x0: Index into insurance_names.
    u8 name;
    // 0x1: Months after the treatment before the claim is paid (finance.cpp).
    u8 payment_delay;
    u8 unknown2;
    // 0x3: Percent its patients will pay above the normal maximum price
    // (sim.cpp).
    u8 extra_pay_pct;
};

struct Sector {
    s32 start;
    s32 end;
    s32 radius;
    u8 top_colour;
    u8 side_colour1;
    u8 side_colour2;
    u8 rim_colour;
    u8 edge_colour;
    s32 text_offset;
    s32 text_x;
    s32 text_y;
    s32 text_right;
};

/*
 * A short planned path (at most four waypoints) kept in Thing.path, built by
 * orthonav_init.
 */
struct orthonav_path {
    // 0x0: Current start tile x.
    u8 x0;
    // 0x1: Current start tile y.
    u8 y0;
    // 0x2: Destination tile x.
    u8 x1;
    // 0x3: Destination tile y.
    u8 y1;
    // 0x4: Waypoints; when one is reached it becomes the start
    // (delete_first_path_entry).
    PathStep steps[4];
    // 0xc: Number of waypoints in steps, 0 if no route was found.
    u8 count;
    u8 unknownd[3];
};

struct wallhug_waypoint {
    u8 x;
    u8 y;
};

/*
 * A route for the wall-hugging path finder (wallhug.cpp): from start to end
 * through up to 0xfd waypoints. orthonav fills one through the x0..y1 and
 * points view.
 */
struct wallhug_path {
    // 0x0: Waypoints in route.
    u8 count;
    u8 unknown1;
    union {
        struct {
            // 0x2: Signed view of start.x.
            s8 x0;
            // 0x3: Signed view of start.y.
            s8 y0;
            // 0x4: Signed view of end.x.
            s8 x1;
            // 0x5: Signed view of end.y.
            s8 y1;
            // 0x6: PathStep view of route.
            PathStep points[0xfd];
        };
        struct {
            // 0x2: Start tile.
            wallhug_waypoint start;
            // 0x4: Destination tile.
            wallhug_waypoint end;
            // 0x6: Waypoints in order; on success the last is end.
            wallhug_waypoint route[0xfd];
        };
    };
};

/* One of the two wall followers of wallhug_info; its route has room for 0xfc steps. */
struct wallhug_hugger {
    // 0x0: Corners recorded in route.
    u8 count;
    u8 unknown1;
    // 0x2: Tile the straight line was walked from before the wall was hit.
    wallhug_waypoint start;
    // 0x4: The path's destination.
    wallhug_waypoint end;
    // 0x6: Corner tiles passed while following the wall.
    wallhug_waypoint route[0xfc];
    // 0x1fe: Heading 0-3 (see wall_in_way); 0x32 once back on the line to end,
    // 0x33 when route is full.
    u32 dir;
    // 0x202: Side the wall is kept on: -1 or 1, added to dir to turn.
    s32 turn;
    // 0x206: Current tile.
    wallhug_waypoint pos;
    // 0x208: Tile where the wall was hit.
    wallhug_waypoint hit;
};

struct wallhug_info {
    wallhug_hugger hugger[2];
};

u32 wallhug_tricky(wallhug_path *path);

/*
 * Layouts of globals that are still named after their address (D_XXXXXXXX)
 * but are used by more than one source file.
 */

struct MemRegion {
    u32 addr;
    s32 size;
};

/*
 * A player's command for the turn (D_001549d0[player]), carried out by
 * process_turn.
 */
struct Event {
    u8 unknown0;
    // 0x1: Command (EV_ enumerators).
    u8 type;
    // 0x2: Main argument.
    u16 value;
    // 0x4: View scroll requests: 1 up, 2 down, 4 left, 8 right; 0x10 recentres
    // on the level's camera start.
    u8 flags;
    union {
        // 0x5: Second argument.
        u16 arg;
        // 0x5: Byte view of arg.
        u8 argb[2];
    };
    // 0x7: check_sum() of the sender's game in network games; a mismatch
    // cancels the command and resyncs.
    u32 checksum;
    u8 unknownB[4];
};

/*
 * Two boxes of ScreenBoxes (resvars.screen_boxes) read through a struct: box 0
 * and box 46.
 */
struct View {
    // 0x0: Box 0: text window of the drugs screen (display_drugs).
    s32 drugs_text_x;
    // 0x4: Box 0 y.
    s32 drugs_text_y;
    // 0x8: Box 0 width.
    s32 drugs_text_w;
    // 0xc: Box 0 height.
    s32 drugs_text_h;
    u8 unknown10[0x2d0];
    // 0x2e0: Box 46: graph area of the charts screen (draw_graphs x).
    s32 graph_x;
    // 0x2e4: Box 46 y.
    s32 graph_y;
    // 0x2e8: Box 46 width, passed to draw_graphs as its step.
    s32 graph_step;
    // 0x2ec: Box 46 height, passed to draw_graphs as its height.
    s32 graph_h;
};

struct Point {
    s32 x;
    s32 y;
};

/*
 * A clickable button of a full-screen panel (resvars.screen_buttons).
 */
struct Button {
    s16 x;
    s16 y;
    u16 width;
    u16 height;
    void (*callback)(int index);
    // 0xc: X where the pressed sprite is drawn.
    u16 sprite_x;
    // 0xe: Y where the pressed sprite is drawn.
    u16 sprite_y;
    // 0x10: Pressed sprite (screen_sprites_tab_start index).
    u16 sprite;
};

struct AdvisorLayout;
struct AreaLayout;
struct Box {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct Button;
struct ChoiceBox;
struct GlassFrame;
struct GlassScreen;
struct GraphRect;
struct HelpHotspot;
struct IconScreen;
struct ManagerBox;
struct ManagerHelp;
struct MenuItem;
struct NetBox;
struct Offset;
struct PanelLayout;
struct Point;
struct PolicyRect;
struct QueueButtons;
struct QueueLayout;
struct QueuePanel;
struct Rect;
struct Req13Layout;
struct Req2Layout;
struct ReqLayout;
struct ScreenBox;
struct ScrollLayout;
struct SlistArea;
struct SlistMenuItem;
struct Struct_0015f410;
struct TownView;
struct View;
struct WindowButton;

/*
 * Resolution-dependent settings and layout tables (resvars). swap_game_sprites
 * and setup_host point the tables at the high-resolution (640x480) or
 * low-resolution (320x200) set; the typed views (suffix after the name) are
 * the same pointer read as the element type each screen expects.
 */
struct ResolutionInfo {
    // 0x0: Screen mode currently set up (setup_any_screen).
    TbScreenMode screen_mode;
    // 0x1: Resolution of the game graphics: selects the low- or high-resolution
    // sprite, block and requester files.
    TbScreenMode game_mode;
    // 0x2: Mode the game graphics were last loaded or swapped for.
    TbScreenMode loaded_mode;
    // 0x3: Height in pixels of a floor tile (0x20 high resolution, 0x10 low).
    u8 tile_height;
    // 0x4: Width in pixels of a floor tile (0x40 high resolution, 0x20 low).
    u8 tile_width;
    // 0x5: 2 while the game graphics files are loaded (load_game_files), 0
    // otherwise.
    u8 game_files_state;
    union {
        // 0x6: Requester file names (hires_req_filenames).
        u8 *req_filenames;
        char **req_filenames_charpp;
        const char *const *req_filenames_names;
    };
    union {
        // 0xa: Font file names for the font cache (hires_ or
        // lores_font_filenames).
        int *font_filenames;
        char **font_filenames_charpp;
    };
    union {
        // 0xe: Requester button numbers (hires_ or lores_requester_buttons).
        u8 *requester_buttons;
        QueueButtons *requester_buttons_QueueButtonsp;
    };
    union {
        // 0x12: Requester button sprite numbers (hires_ or
        // lores_requester_sprites).
        u16 *requester_sprites;
        u8 *requester_sprites_u8p;
    };
    // 0x16: Fewest middle segments of the advisor's speech bubble (3 high
    // resolution, 4 low).
    u8 advisor_min_segments;
    // 0x17: Most middle segments the advisor's speech bubble may grow to while
    // wrapping text.
    u8 advisor_max_segments;
    union {
        // 0x18: Boxes of the in-game panels and requesters (demo: HiresBoxes,
        // LoresBoxes).
        Box *boxes;
        s32 *boxes_s32p;
        u8 *boxes_u8p;
        PanelLayout *boxes_PanelLayoutp;
        QueueLayout *boxes_QueueLayoutp;
        Rect *boxes_Rectp;
        Req13Layout *boxes_Req13Layoutp;
        Req2Layout *boxes_Req2Layoutp;
        ReqLayout *boxes_ReqLayoutp;
    };
    union {
        // 0x1c: Positions on the in-game screen (demo: HiresCoords,
        // LoresCoords).
        Point *coords;
        IconScreen *coords_IconScreenp;
        u8 *coords_u8p;
        AdvisorLayout *coords_AdvisorLayoutp;
        PanelLayout *coords_PanelLayoutp;
        QueuePanel *coords_QueuePanelp;
        ReqLayout *coords_ReqLayoutp;
        ScrollLayout *coords_ScrollLayoutp;
        Struct_0015f410 *coords_Struct_0015f410p;
    };
    union {
        // 0x20: Requester windows (demo: window_list, mcgawindow_list).
        Window *window_list;
        u8 *window_list_u8p;
    };
    union {
        // 0x24: Requester window buttons (demo: button_list, mcgabutton_list).
        u8 *button_list;
        WindowButton *button_list_WindowButtonp;
    };
    // 0x28: Buttons of the in-game screen (demo: hiresbuttons, loresbuttons).
    u8 *buttons;
    union {
        // 0x2c: Help hotspots, grouped and terminated by negative x0 (demo:
        // hireshelp, loreshelp).
        MenuItem *help_hotspots;
        HelpHotspot *help_hotspots_HelpHotspotp;
        int *help_hotspots_intp;
        ManagerHelp *help_hotspots_ManagerHelpp;
        SlistMenuItem *help_hotspots_SlistMenuItemp;
    };
    u8 unknown30[0x4];
    union {
        // 0x34: Offsets of the panel's popup icons (demo: panel_popup_coord,
        // mcgapanel_popup_coord).
        u8 *panel_popup_coords;
        Point *panel_popup_coords_Pointp;
    };
    union {
        // 0x38: Litter offsets within a map cell, chosen by a hash of the cell
        // (demo: litter_pos).
        u8 *litter_pos;
        Offset *litter_pos_Offsetp;
    };
    union {
        // 0x3c: Boxes of the full-screen menus (demo: ScreenBoxes); same table
        // in both resolutions.
        View *screen_boxes;
        ChoiceBox *screen_boxes_ChoiceBoxp;
        GraphRect *screen_boxes_GraphRectp;
        NetBox *screen_boxes_NetBoxp;
        Box *screen_boxes_Boxp;
        ManagerBox *screen_boxes_ManagerBoxp;
        PolicyRect *screen_boxes_PolicyRectp;
        ScreenBox *screen_boxes_ScreenBoxp;
        TownView *screen_boxes_TownViewp;
        u8 *screen_boxes_u8p;
    };
    union {
        // 0x40: Positions on the full-screen menus (demo: ScreenCoords).
        Point *screen_coords;
        s32 *screen_coords_s32p;
    };
    union {
        // 0x44: Staff list area of the full-screen menus (demo: screenlistpos).
        u8 *screen_list_pos;
        SlistArea *screen_list_pos_SlistAreap;
    };
    // 0x48: Buttons of the full-screen menus (demo: screenbuttons).
    Button *screen_buttons;
    u8 unknown4c[0x4];
    union {
        // 0x50: Glass frame animations (demo: hiresglass).
        u8 *glass_frames;
        AreaLayout *glass_frames_AreaLayoutp;
        GlassFrame *glass_frames_GlassFramep;
        GlassScreen *glass_frames_GlassScreenp;
    };
    u8 unknown54[0x4];
    union {
        // 0x58: Width of the border drawn left and right of a staff face
        // (draw_face).
        s32 face_border_w;
        u32 face_border_w_u32;
    };
    union {
        // 0x5c: Height of the border drawn above and below a staff face.
        s32 face_border_h;
        u32 face_border_h_u32;
    };
    union {
        // 0x60: Width of a staff face; the face file holds 18 heads, then
        // bodies, then legs.
        s32 face_w;
        u32 face_w_u32;
    };
    union {
        // 0x64: Height of a face's head part.
        s32 face_head_h;
        u32 face_head_h_u32;
    };
    union {
        // 0x68: Height of a face's body part.
        s32 face_body_h;
        u32 face_body_h_u32;
    };
    union {
        // 0x6c: Height of a face's legs part.
        s32 face_legs_h;
        u32 face_legs_h_u32;
    };
};

extern ResolutionInfo resvars;

/*
 * Per hospital level (area_towns[], indexed by level): where the town map sits
 * on the town screen and its scale. The list ends at an entry whose w or h is
 * 0.
 */
struct ClickArea {
    u8 unknown0[4];
    // 0x4: Width; 0 ends the list.
    u16 w;
    // 0x6: Height; 0 ends the list.
    u16 h;
    // 0x8: Read by draw_area_select, which does not use it.
    u8 kind;
    // 0x9: Screen x of the town map's map cell 0.
    u16 x;
    // 0xb: Screen y of the town map's map cell 0.
    u16 y;
    // 0xd: Screen pixels per map cell across.
    u8 width;
    // 0xe: Screen pixels per map cell down.
    u8 height;
    u8 unknownf;
};

/*
 * Properties of a floor overlay (background[], indexed by Map.overlay).
 */
struct Background {
    // 0x0: Cells with this overlay are drawn on the town and plot-sale maps.
    u8 show_on_map;
    // 0x1: Initial Map bits for unbought cells: [0] Map.owner_flags, [1]
    // Map.flags.
    u8 map_defaults[2];
};

/*
 * The land auction on the town screen (plotbid).
 */
struct PlotBid {
    // 0x0: Frames left on the auction clock; each bid resets it to 100. When it
    // runs out the plot goes to bidder.
    u32 timer;
    // 0x4: Next bid: the starting price, raised by step after each bid.
    s32 price;
    // 0x8: Bid increment: a tenth of the starting price, at least 10.
    s32 step;
    // 0xc: Per player: their last bid.
    s32 bids[4];
    // 0x1c: Player with the highest bid, 0x63 for none.
    u8 bidder;
    u8 unknown1d[3];
};

struct Picture {
    u8 head;
    u8 body;
    u8 legs;
};

/*
 * A help-tooltip rectangle of a requester
 * (resvars.help_hotspots_HelpHotspotp).
 */
struct HelpHotspot {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    // 0x8: Help text index.
    u16 text;
    // 0xa: Requester slot (req_indices index) of the room or object the hotspot
    // describes.
    u16 req_slot;
    // Builds the help text when set.
    void (*func)(void);
};

struct PlotAdjacent {
    u8 level;
    u8 plot;
};

/*
 * Outcome of a mayor visit per score band (mayor_result[]).
 */
struct MayorPoints {
    // 0x0: Highest mayor score in this band.
    s16 max_score;
    // 0x2: Event passed to reputation_affected, 0 for none.
    s16 reputation_event;
    // 0x4: Cash given (or taken), 0 for none.
    s16 cash_bonus;
};

/*
 * One entry of towns[], indexed by level: the level's town settings.
 * Level-file keys towns.* set some fields (vloader.cpp).
 */
struct Town {
    // 0x0: StartCash key: opening bank balance.
    u32 start_cash;
    u8 unknown4;
    // 0x5: Argument passed to rat_fest_builder or training_builder.
    u8 special_arg;
    // 0x6: 1 for the rat fest level, 2 for the training level, 0 otherwise.
    u8 special_kind;
    // 0x7: IllRate key: a new patient has a 1 in ill_rate chance of a visual
    // illness; 0 never.
    u8 ill_rate;
    u8 unknown8[6];
    // 0xe: InflationRate key.
    u16 inflation_rate;
    // 0x10: InterestRate key.
    u16 interest_rate;
    // 0x12: A new patient is insured with a 1 in insurance_odds chance.
    u16 insurance_odds;
    u8 unknown14[9];
};

struct FaceDirns {
    s8 dirs[8];
};

struct ObjectWalls {
    u8 count;
    u16 pos[4];
    u8 side[4];
};

struct WallHug {
    u8 kind;
    u8 angle[4];
};

struct PossibleDiag {
    u8 count;
    u8 ids[9];
};

struct RoomNavControl {
    u8 door_count;
    u8 door_objects[10];
    u8 count;
    u8 objects[10];
};

/*
 * An object kind that changes the mood of people near it (feely[], read by
 * check_feelgood); only plants have an effect.
 */
struct FeelGoodies {
    // 0x0: Object kind (OBJ_).
    u16 kind;
    // 0x2: Distance in tiles, on each axis, within which it counts.
    u8 range;
    // 0x3: Nonzero for a live entry; the list ends at the first 0.
    u8 valid;
    u16 unknown4;
};

struct FillInWindow {
    u8 wall;
    u8 window;
};

struct SlidingDoorControl {
    u16 open_anim;
    u16 close_anim;
    s16 offset[4];
};

/*
 * One line of the graphs screen (graph[]).
 */
struct Graphs {
    // 0x0: 0 plots values upward, 1 downward.
    u8 inverted;
    // 0x1: The line is drawn.
    u8 visible;
    // 0x2: Line colour (colour index).
    u8 colour;
    // 0x3: Legend text index (report_text).
    u8 label;
};

struct ShowAnim {
    u16 anim;
    u8 subtypes[4];
    u8 any_subtype;
    u8 illness;
    u16 condition;
    u8 ability;
};

/*
 * One entry of the panel popup's icon list (popup_icons[], 0-terminated from
 * index 1).
 */
struct PopupIconControl {
    // 0x0: Panel sprite (game_panel index).
    u32 sprite;
};

/*
 * A step of a two-person interaction sequence (interaction[];
 * statvar.interaction_start holds where each sequence starts), played by
 * setup_next_interaction.
 */
struct DoorAnim {
    // 0x0: Animation (anim_starts anims[] index) the thing plays, 0 none; -1
    // ends a sequence, -99 the table.
    s16 anim;
    // 0x2: Animation its partner (slave) plays, 0 none.
    s16 partner_anim;
    // 0x4: Times the step plays.
    u8 count;
    u8 unknown5;
};

struct PeepType {
    u8 kind;
    u8 count;
    u8 head[5];
    u8 palette[5];
    u8 weight;
};

struct Coord {
    s32 x;
    s32 y;
};

struct PeepPalette {
    u8 count1;
    u8 layer1[5];
    u8 count2;
    u8 layer2[5];
    u8 count3;
    u8 layer3[11];
};

struct FloorDef {
    u16 anim;
    u16 animated;
};

/*
 * A step of a staff or patient action sequence (staff_complex[],
 * person_complex[]), reached through staff_complex_group /
 * person_complex_group at Thing.complex_pc and run by process_staff_complex /
 * process_person_complex.
 */
struct Complex {
    // 0x0: Object kind (OBJ_) the step needs; the nearest free one is found,
    // and the complex fails without one. 0 for none.
    u8 object;
    // 0x1: What the step does (the switch in process_staff_complex and
    // process_person_complex).
    u8 action;
    // 0x2: The action's argument: usually an anim_starts anims[] index.
    u16 anim;
    // 0x4: If nonzero, the Thing.state set before the action.
    u8 state;
    u8 unknown5;
    // 0x6: Extra times the step runs (0 runs it once); negative for a random 1
    // to -repeat extra runs.
    s8 repeat;
    // 0x7: Run the next step at once rather than on a later turn.
    u8 chain;
};

struct ComplexAnims {
    s8 count;
    s16 anims[5];
};

/*
 * A bottom-panel icon.
 */
struct PanelIcon {
    // 0x0: Screen left, laid out by setup_panel_icons.
    s16 x;
    // 0x2: Screen top.
    s16 y;
    // 0x4: Screen right.
    s16 x2;
    // 0x6: Screen bottom.
    s16 y2;
    // 0x8: Normal sprite.
    u8 sprite;
    u8 unknown9[2];
    // 0xb: 1 for a button with a pressed sprite, 0x40 for an icon without one.
    u16 kind;
    // 0xd: Pressed sprite.
    u8 pressed;
};

struct ambient {
    u8 sound;
    u8 priority;
};

struct PitchGroup {
    u8 count;
    u8 values[5];
};

struct SampleGroup {
    u8 count;
    u8 samples[5];
};

extern Town towns[];
extern MayorPoints mayor_result[];
extern s16 opposite_angle[];
extern Blocks blocks[];
extern const u8 blocks_backup[];
extern const u8 *pointer_second_frame_check;
extern VXY around_vxy[];
extern s16 around[];
extern s16 next_angle[];
extern Blueprint blueprint[];
extern DoorInfo doortype[];
extern ObjectControl objects[];
extern Shapes shapes[];
extern s8 change_person_who[][15];
extern RoomControl rooms[];
extern DepartmentControl departments[];
extern const u8 ingame_pointer_second_frame[];
extern const u8 screen_pointer_second_frame[];
extern const u8 ingame_pointer_hotspots[];
extern const u8 screen_pointer_hotspots[];
extern u8 abilities[][6];
extern u8 staff_abilities[][2];
extern Expertise expertise[];
extern Population popn[];
extern ResearchWorkStations research_workstations;
extern menu pmenu;
extern AnimStart anim_starts[];
extern AnimStart staff_anim_starts[];
extern TbLoadFiles mcgaspritefiles[];
extern TbLoadFiles hiresspritefiles[];
extern TbLoadFiles hiresblockfiles[];
extern TbLoadFiles loresblockfiles[];
extern TbLoadFiles hiresanifiles[];
extern TbLoadFiles loresanifiles[];
extern TbLoadFiles reqfiles[];
extern TbLoadFiles mcgareqfiles[];
extern TbLoadFiles langfile[];
extern TbLoadFiles hiresgamepointerfiles[];
extern char *hiresscreenpointerfiles[];
extern TbLoadFiles lorespointerfiles[];
extern const char *const hires_req_filenames[];
extern char *hires_font_filenames[];
extern char *lores_font_filenames[];
extern TbLoadFiles palettefile[];
extern TbLoadFiles complexfiles[];
extern TbLoadFiles screenfiles[];
extern TbLoadFiles memfiles[];
extern TbLoadFiles thingfiles[];
extern TbLoadFiles soundfiles[];
extern TbLoadFiles nosoundfiles[];
extern TbLoadFiles midifiles[];
extern const char *const hiresscreen_sprites_data[];
extern const char *const hiresscreen_sprites_tabs[];
extern TbLoadFiles hiresratfestpanelfiles[];
extern Complex staff_complex[];
extern s8 group_ia[][5];
extern long face_file;
extern const char *const hires_faces;
extern const char *const lores_faces;
extern u8 hires_requester_buttons[];
extern u8 lores_requester_buttons[];
extern u16 hires_requester_sprites[];
extern u16 lores_requester_sprites[];
extern ambient Ambients[];
extern s16 showanim_angle[];
extern const u8 calendar[];
extern u8 suitable_illness[][5];
extern u16 visuals[];
extern u16 non_visuals[];
extern u16 recept_tasks[];
extern u8 hilight_plot[];
extern s16 standing_place[];
extern const u16 advisor_specials[];
extern const u16 next_advisor_state[];
extern const u16 advisor_up_states[];
extern const u8 chart_in_use[];
extern u8 bid_hammer[];
extern u8 ani_seq_choice[][10];
extern s8 bank_manager_face3[];
extern u8 watch_face[];
extern u8 mouse_mode_help_allowed[];
extern u8 display_mode_control[];
extern AnimTable animation_table[];
extern u32 wasteOfTime;
extern TbSprite *screen_pointers_tab_start;
extern TbSprite *screen_sprites_tab_start;
extern TbSprite *sprite;
extern TbSprite *pointer;
extern u8 *current_palette;
extern u8 *bscreen;
extern TbSprite *watch;
extern CompFrame *frame;
extern CompElement *element;
extern u16 *elelist;
extern const IconTable icon_table[];
extern SfxDetail sfx_details[];
extern Map *map;
extern Map *end_map;
extern TbSprite *game_panel;
extern TbSprite *pulldv;
extern ToDraw draw_cells[];
extern PixBuffer pixBuf;
extern Refresh refresh;
extern u8 req_indices[];
extern u8 bw_remap[];
extern s32 loaded_requester;
extern RoomInfo *room_info;
extern RoomInfo *end_room_info;
extern AllHosps *allhosps;
extern AllHosps *myhosp;
extern GameControl *mycontrol;
extern HospPlayerDetails myplayer;
extern ListHead *listhead;
extern PlayerCall *playercall;
extern PlayerCall *end_playercall;
extern SkillPager *pagers;
extern SkillPager *end_pagers;
extern CompStarts *start;
extern u32 seed;
extern char balance_id[];
extern Colors color;
extern char **text_starts;
extern Thing *thing;
extern Thing *end_things;
extern u16 quit_game;
extern u16 display_mode;
extern u16 chart_type;
extern u16 year_end_type;
extern u32 game_turn;
extern u32 actual_game_turn;
extern u16 version;
extern Packet packet[];
extern u32 player;
extern NetText net_text[];
extern CurrentText current_text;
extern Game gamevar;
extern Statics statvar;
extern Scroll scroll;
extern Info info;
extern HospWindow window;
extern Required required;
extern Debug debug;
extern GameCounts gamecounts;
extern StaffLevels staff_levels[];
extern StaffControl staff[];
extern RatFestLevel rat_fest_level[];
extern TrophyBalance awards_trophies;
extern u32 safeseed;
extern u8 HospShift;
extern u8 level_number;
extern Menu mainmenu;
extern Menu town;
extern Menu drugs;
extern Menu bank;
extern Menu pol;
extern Menu research;
extern Menu reputation;
extern Menu variables;
extern Menu manager;
extern Menu slist;
extern Menu graphs;
extern Menu hall;
extern Menu yearend;
extern Menu winlose;
extern Menu que;
extern Req crequester;
extern Req prequester;
extern Level *level;
extern void *midi_buffer;
extern u16 popup_id[];
extern u16 panel_popup_pos[];
extern Icons *panel;
extern u16 available_buttons;
extern s16 choice_spare_data;
extern s16 choice_time_out;
extern ScreenInfo choice_backup;
extern ScreenInfo year_end_backup;
extern NonResettingAdvisorCounts NRAdcounts;
extern AwardCounts award_counts;
extern Balance gbv;
extern General othervar;
extern Advisor advisor;
extern AdvisorCounts adcounts;
extern ResearchTasks research_tasks[];
extern u8 illness_abilities[][1];
extern WinLoseNet current_net_criteria;
extern char cd_install_path[];
extern u8 install_type;
extern u8 sub_error_number;
extern u16 key_mappos[];
extern u8 exit_the_yearend_screen;
extern Trophies trophy[];
extern Awards all_awards[];
extern char **help_text;
extern char **yesno_text;
extern char **month_text;
extern char **staff_names_1;
extern char **staff_names_2;
extern char **advisor_text;
extern char **insurance_names;
extern char **hall_text;
extern char **extra_advice;
extern char **nickname_text;
extern char **room_help;
extern char **bank_text;
extern char **object_help;
extern char **load_text;
extern char **mayor_names;
extern char **choice_text;
extern char **cheat_text;
extern char **ref_general_text;
extern char **ref_good_text;
extern char **ref_bad_text;
extern char **info_bar_text;
extern char **advisor_situations;
extern u8 *text_buffer;
extern Window *open_windows[];
extern s8 *button_sprite_data;
extern s8 *end_button_sprite_data;
extern TbSprite *button_sprite;
extern TbSprite *bord;
extern TbSprite *end_button_sprites;
extern u8 mouse_was_on_window;
extern s32 CurrentWindow;
extern u8 OldLeftButton;
extern u8 OldRightButton;
extern WindowButton *buttonstore;

void init_game_vars(void);
void get_cd_speed(void);
void setup_blocks(void);
void setup_game_sprites(void);
void reset_blocks(void);
u8 setup_host(void);
void reset_host(void);
void get_input(void);
void load_game_files(void);
void unload_game_files(void);
u8 setup_any_screen(const u8 *const palette, TbScreenMode mode);
void game(void);
void limit_framerate(void);
void start_turn(void);
u8 change_res(TbScreenMode mode);
void do_charts(void);
void do_year_end(void);
void draw_pause_mode(void);
void check_if_active(void);
u8 process_lost_focus_display(u8 draw);
void display_mission(void);
void display_drugs(void);
void process_drug_panel(void);
void price_up(void);
void price_down(void);
void process_drug_keys(void);
void process_drug_button(u8 action);
u8 get_next_illness(const u8 forward, const u8 from);
void setup_drug(u16 hospital, u16 illness);
void drugs_help_concentrate(void);
void drugs_help_cure_type(void);
void drugs_help_handled(void);
void update_cure_costs(void);
void update_single_cure_cost(u8 illness, u8 hospital);
void do_mapwho(const Map *const cell, const s32 x, const s32 y, const s32 left, const s32 top);
void mapwho_person(Thing *person, s32 x, s32 y, s32 left, s32 top);
void mapwho_staff(Thing *staff, s32 x, s32 y, s32 left, s32 top);
void mapwho_object(Thing *obj, const s32 x, const s32 y, const s32 left, const s32 top);
void mapwho_reaper(Thing *reaper, s32 x, s32 y);
void mapwho_rat(Thing *rat, const s32 x, const s32 y, const s32 left0, const s32 top0);
void mapwho_bang(Thing *bang, s32 x, s32 y);
void mapwho_helicopter(Thing *helicopter, s32 x, s32 y);
void mapwho_numfx(Thing *fx, s32 x, s32 y);
void mapwho_mayor(Thing *mayor, s32 x, s32 y, s32 left, s32 top);
void mapwho_shadow(Thing *shadow, s32 x, s32 y);
void insert_node(Thing *node, MapPos x, MapPos y);
u16 move_node(Thing *node, MapPos x, MapPos y);
void person_waits_for_busy(Thing *person);
void person_arrives_at_door(Thing *person);
void person_in_queue(Thing *person);
void person_entertains_themself(Thing *person);
u16 find_standing_place(Thing *person);
void person_go_home(Thing *person);
void remove_person(Thing *person);
void move_person_in_room_list(Thing *person, u16 tile);
void insert_person_in_room_list(RoomInfo *room, Thing *person);
void nav_debug_state(Thing *person);
u8 transform_peep_who(Thing *person, u8 level);
u16 call_person_to_room(Thing *caller, u16 id, u16 action, u16 arg, u8 follow);
u8 call_person_again(Thing *caller, u16 action, u16 arg);
void send_person_to_bed(Thing *person);
void person_in_bed(Thing *person);
u16 find_peep(u16 room_id, u8 random, u8 type, u8 state, u8 owner);
void start_person_going(Thing *person);
void person_nav_failure(Thing *person);
void person_fills(Thing *person);
void person_energises(Thing *person);
void person_in_bogs(Thing *person);
void leave_toilets(Thing *person);
void return_to_queue_area(Thing *person);
void play_person_showanim(Thing *person);
void page_staff_i_need(Thing *person, RoomInfo *room);
void send_person_somewhere_else(Thing *person);
void refer_patient(Thing *person, u8 owner);
void person_waits_for_decision(Thing *person);
u16 make_a_playercall(const u16 type, const u16 subject, const s16 timeout, u8 owner);
void get_new_popup_head(u16 type, u8 owner);
void save_popup(u16 type);
void restore_popup(u16 type);
u16 get_playercall(const u16 type, u16 id, u8 owner);
u16 check_playercall(u16 type, u16 subject, u8 owner);
void expire_playercalls(void);
u16 call_no_treatment(Thing *person, char **strs);
void process_no_treatment_buttons(u8 button);
void person_arrived_from_call(Thing *person);
u32 person_ok_to_call(Thing *person);
u16 announce_new_illness_fax(const u8 kind, char **strs);
void process_announce_new_illness_buttons(u8 button);
void force_peep_to_be_cured(Thing *person);
void interrupt_person(Thing *person);
u16 div_err(void);
void setup_colors(void);
u16 can_i_get_to(Thing *mover, u16 pos);
void remove_thing(Thing *target);
void count_hospital_area(u16 owner);
Thing *search_mapwho_for_room_door(Map *cell, u16 id);
void screen_shot(void);
RoomInfo *get_room_info_rand(u8 kind, u8 mode, u8 owner);
RoomInfo *get_room_info(u8 kind, u8 mode, u8 owner);
RoomInfo *get_nearest_room_info(Thing *person, u8 kind);
void cheat(u32 flags);
void set_mouse_mode(const u8 mode);
void generate_bw_remap(u8 *palette, u8 *out);
u8 check_points_connected(u16 from, u16 to);
u8 direction_to_face(Thing *self, Thing *target);
void convert_MW_to_xy(u16 pos, s8 *x, s8 *y);
void measure_objects(void);
int descend(const void *a, const void *b);
int ascend(const void *a, const void *b);
void set_mouse_sprite(const u8 idx);
u16 can_i_go_near_point(Thing *person, u16 target);
u8 am_i_near_point(Thing *person, u16 pos);
void vary_speed(s16 speed);
void compute_whether_user_not_busy(void);
u32 calc_room_value(RoomInfo *room);
Thing *find_person_in_room(RoomInfo *room, u16 kind, u16 sub);
void setup_abort_all_requesters(void);
void copy_screen_to_wscreen(const u8 *src);
void copy_screen_from_wscreen(u8 *dst);
void cancel_top_level_requesters(s32 keep);
Thing *allocate_thing(u32 reserve);
s32 over_hotspot(s16 x, s16 y, s16 rx, s16 ry, s16 w, s16 h);
void draw_frame(const s16 x, const s16 y, const s16 w, const s16 h, const u16 color);
u8 hosp_play_smk(const u8 idx);
void hosp_dos_printf(char *fmt, ...);
void recall_mappos(u8 i);
void setup_key_mappos(void);
void hosp_fade_palette(u8 *palette, u8 steps, TbPaletteFadeFlag flag);
void close_crequester(u8 i);
void process_turn(void);
void temp_place_door(u16 pos, u8 owner);
void place_bp_door(u8 owner, u8 swap);
RoomInfo *which_room(u16 tile);
RoomInfo *is_this_a_door(u16 tile);
void object_placed(void);
void init_place_object(u8 kind, u8 mode, u16 room, s16 offset);
void init_edit_room(RoomInfo *room);
void setup_place_object_requester(void);
u8 next_object_to_build(void);
void open_room(u8 owner);
void pick_up_object(u16 id, u8 both);
void clear_build_objects(u8 owner);
void clear_object_drag_shape(void);
void sack_staff(Thing *staff, u16 mode);
s32 pay_staff_bonus(Thing *staff, u16 amount, u16 percent, u16 morale, u8 owner);
void staff_picked_up(Thing *staff);
void interrupt_staff(Thing *staff);
void staff_join_new_employer(u16 id, u16 owner);
void increase_staff_salaries(Thing *staff, u16 amount, u16 percent, u8 owner);
u8 set_group_address(s32 group, char **address);
s16 setup_lang(void);
void draw_a_frame(const u16 frame_id, const s16 x, const s16 y, const u8 flags, const u8 flip);
void draw_a_one_colour_frame(const u16 frame_id, const s16 x, const s16 y, const u16 colour, const u8 flip);
void draw_special_id_frame(Thing *w, const s16 x, const s16 y);
void draw_split_frame(Thing *w, const s16 x, const s16 y);
void draw_energise_frame(Thing *w, const s16 x, const s16 y);
void draw_a_frame_scaled(const u16 frame_id, const s16 x, const s16 y, u8 flags, const u8 flip, s32 scale);
void draw_hilight_frame(Thing *w, const s16 x, const s16 y);
void full_draw_frame(Thing *w, const u16 frame_id, const s16 x, const s16 y, const u8 flags, const u8 hidden,
                     const u8 flip);
void draw_from_frame(const u16 frame_id, const s16 x, const s16 y, u8 *remap, const u8 flip);
void InitPixBuffer(s16 x, s16 y, s16 w, s16 h, u8 colour, u8 clear);
void FillPixBuffer(u8 colour);
void DrawIntoPixBuffer(void);
void RestoreFromPixBuffer(void);
void DrawPixBuffer(u8 flags, s32 x, s32 y, s32 w, s32 h);
void FillBoxInPixBuffer(u8 colour, s32 x, s32 y, s32 w, s32 h);
void draw_wibble(s32 x, s32 y, u8 *buf, s32 w, s32 h, s32 phase);
s32 sprite_draw_glow(s32 x, s32 y, TbSprite *spr, u8 glow);
void draw_from_special_id_frame(const u16 frame_id, const s16 x, const s16 y, u8 *remap, u8 *layers, const u8 flip);
void icon_draw_from_special_id_frame(const u16 icon, const s16 x, const s16 y, const s16 dx, const s16 dy);
void draw_text_on_map(char *text, const u8 colour, const s32 x, const s32 y);
void draw_money(s32 x, s32 y, s32 value);
void setup_panel_icons(const u8 mode);
void clear_panel_info_bar(void);
void delete_panel_popup(PlayerCall *call);
void add_panel_popup(PlayerCall *call);
void remove_panel_popup(const u8 slot);
u8 which_icon_is_mouse_over(void);
u16 find_object(u16 tile, u16 kind, u8 random, u8 flag, u8 owner);
u16 find_object_anywhere(u16 kind, u8 random, u16 after, u8 owner);
u16 find_nearest_free_object(u16 id, u8 kind);
u16 find_nearest_manned_reception(Thing *person, const u8 room, const u8 need_space);
s16 place_small_thing(u16 tile, u16 id, u8 keep_existing);
s16 find_small_thing(u16 tile);
u8 check_room_access_with_blueprint(u8 owner);
void replace_machine(Thing *machine);
void setup_next_treatment(Thing *staff, Thing *patient);
void next_treatment_now_setup(Thing *patient, u8 in_room);
void best_guess_treatment(Thing *patient);
u16 find_possible_diagnosis(Thing *patient);
void record_person_as_patient(Thing *patient);
void done_treatment(Thing *staff, Thing *patient);
void affect_diagnosis(Thing *staff, Thing *patient);
void change_health(Thing *patient, s16 amount);
void person_dies(Thing *patient);
void person_dead(Thing *patient);
void add_health_to_history(Thing *patient);
void diagnose_patient(Thing *staff, Thing *patient);
s32 calc_ability(Thing *staff);
void add_treatment_to_history(Thing *patient);
u8 treatment_already_done(Thing *patient, u16 treatment);
void person_go_for_treatment(Thing *patient);
s16 pre_administer_drugs(Thing *patient);
void administer_drugs(Thing *staff, Thing *patient);
void announce_new_illness(const u16 illness, const u16 owner);
u16 call_no_diagnosis(Thing *patient, char **strs);
void process_no_diagnosis_buttons(u8 button);
u16 pay_for_treatment(Thing *patient, u16 kind);
u16 shall_peep_pay(Thing *patient);
void effect_illness(Thing *patient, u8 illness, u8 cured);
void clean_contagious_peep(Thing *peep);
Thing *find_door(Thing *person);
u32 old_normal_rand(u32 low, u32 high);
s16 get_state(const u16 type, const Thing *staff);
u8 evacuate_staff(Thing *staff);
u8 take_a_break(Thing *staff);
u32 shall_i_resign(Thing *staff);
u8 obey_pager(Thing *staff);
u8 could_nurse_vaccinate(Thing *nurse);
u8 am_i_lead_staff(Thing *staff);
u8 overlapping_staff_check(Thing *staff, s32 state);
void alert_epidemic(u8 illness, u8 owner);
u8 summon_the_reaper(Thing *victim);
void send_person_for_research(Thing *patient);
u8 process_person_complex(Thing *person);
u8 set_complex(Thing *w, u16 action, u16 arg);
void abandon_complex(Thing *w);
void interrupt_complex(Thing *w);
void display_policy(void);
void init_blueprint(u8 owner, u8 type);
void modify_blueprint(u8 owner, u8 dragging, u16 tile);
u8 commit_blueprint(u8 owner);
void uncommit_blueprint(u8 owner);
void edit_blueprint(u8 owner);
void process_blueprint_zones(void);
void commit_doors(u8 owner);
u8 move_peep_from_blueprint(Thing *peep);
void delete_blueprint_room(u8 owner);
void allocate_roominfo(const u8 owner, const u8 type);
u16 page_member_of_staff(u16 skill_mask, u16 room_id, u8 count_present, s8 wanted, u8 owner);
RoomInfo *get_expertise_dest(u16 type, Thing *patient);
void set_expertise_ward(u8 owner);
void remove_expertise_ward(u8 owner);
void front_queue(const u16 room_id, const u16 pos);
void back_queue(const u16 room_id, const u16 pos);
void leave_queue(Thing *person);
u16 which_queue(Thing *person);
void queue_person_go_home(const u16 room_id, const u16 pos);
void queue_person_go_reception(const u16 room_id, const u16 pos);
void queue_person_to_new_queue(const u16 room_id, const u16 id);
void join_queue(Thing *person, RoomInfo *room);
u16 queue_size(RoomInfo *room);
void process_queue_button(u8 button);
u16 get_real_person_from_room(const u16 room_id, const u16 pos);
s16 spare_queue_capacity(const u16 type, const u8 waiting, RoomInfo *room, const u8 owner);
s16 all_queue_capacity(const u16 type, const u8 waiting, const u8 owner);
void restart_all_queue_peeps(RoomInfo *room);
void flag_all_queue_members(RoomInfo *room);
void draw_buttons(Window *win);
void draw_a_window(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_window(Window *win);
void DrawWindows(Window **windows);
void reposition_buttons(Window *win, s32 dx, s32 dy);
void clear_buttons(Window *win);
u8 mouse_over(s32 x1, s32 y1, s32 x2, s32 y2);
u8 mouse_pressed_over(s32 x1, s32 y1, s32 x2, s32 y2);
void calculate_currentbuttonstore(Window *win);
u8 process_buttons_normal_state(Window **windows, Window *win, u16 *pos);
void process_buttons(Window **windows, Window *win);
u8 insert_window(Window **windows, Window *win);
u8 make_front_window(Window **windows, Window *win);
u8 remove_window(Window **windows, Window *win);
u8 OpenWindow(Window **windows, Window *list, u16 idx);
u16 CountWindows(Window **windows);
void GetWindowDimensions(Window *win, u16 *width, u16 *height);
u8 MouseOnWindow(Window *win);
u8 MousePressedOnWindow(Window *win);
void process_window(Window **windows, Window *win);
void change_balance(const u8 type, const s32 amount, const u16 arg, const u16 owner);
void month_end(void);
void year_end(void);
void bank_statement(void);
u16 exit_quick_requester(void);
void daily_admin(void);
void twice_daily_admin(void);
void calculate_value(const u8 owner);
void adjust_build_palette_for_shop(RoomInfo *room);
void clear_req_indices(void);
void set_buyable_items(u16 room);
void buy_compulsory_items(const u16 room);
void req1_reset_rooms(u16 dept);
u16 room_available(u16 room);
void req1_buy_object(const u16 obj, const s32 amount);
void orthonav_init(orthonav_path *path, s8 x0, s8 y0, s8 x1, s8 y1);
void display_peep_overview(void);
void reputation_affected(const u8 treatment, const u16 event, const u16 month, const u8 owner, const s32 amount);
void check_game_over(void);
void check_quarterly_game_over(void);
s32 max_price_for_treatment(const u16 treatment, const u8 owner);
void satisfied_net_criteria(const u8 criterion, const u8 owner);
void process_mouse(void);
void process_things(Thing *first, Thing *last);
void display_advisor(void);
void announce_this(const u8 type, const u32 value);
void draw_main_menu(void);
void process_keys_main_menu(void);
void process_mouse_main_menu(void);
void process_mouse_town_select(void);
void buy_land(u16 plot);
void plot_bid_money(u8 who);
void setup_town_bid(void);
u8 draw_area_select(void);
void process_keys_area_select(void);
void process_mouse_area_select(void);
void req2_reset(void);
u16 req2_alter_staff(u8 dir, u16 cur, u16 kind);
void req2_mark_employing(const u16 id);
void req2_exit_employing(const u16 id);
void open_staff_resign_requester(Thing *staff);
void req12_clicked_sack(void);
void open_yes_no_requester(u8 yes_type, u16 yes_value, u16 yes_arg, u8 no_type, u16 no_value, u16 no_arg);
void close_yes_no_requester(u8 reopen);
void req13_cancel_check(void);
void modify_person(Thing *person);
void close_query(u8 i);
s32 SWIKeyboardOpen(void);
s32 SWIKeyboardClose(void);
void machine_disaster(Thing *machine);
void create_explosion(u16 tile, u16 anim);
void what_is_required(u8 illness, u8 owner);
void draw_graphs(u16 x, u16 y, u16 h, u16 step);
s32 read_config_file(void);
void place_litter(u16 tile, s8 owner, u8 type);
u8 thingpage_request(ThingPager *p, u16 id, u8 priority);
void display_manager(void);
void draw_network_select(void);
void process_keys_network_select(void);
void process_mouse_network_select(void);
void do_net_text(void);
void net_text_display(u8 from);
void net_text_keypress(u8 key, u8 who);
void net_text_delete(u8 who);
u8 network_num_players(void);
void network_lost_focus(s32 who);
void network_gained_focus(s32 who);
void draw_lost_focus_screen(void);
u8 has_any_player_lost_focus(void);
void close_irequesters(void);
void adjust_handyman_values(const u16 id, const u8 button);
u8 setup_sound(void);
void do_ambients(void);
void play_ambient(u8 sound);
void play_sound(u32 source, u16 sfx, s32 x, s32 y);
void manage_sound(void);
void manage_background_sounds(void);
void play_next_midi(void);
void setup_menus(void);
void check_save_games(void);
u16 check_frame(u16 anim, Thing *peep);
void stop_midi(void);
void play_midi(u8 track);
void draw_nickname(void);
void process_keys_nickname(void);
void process_mouse_nickname(void);
void display_slist(void);
void request_vaccination(Thing *person);
void cancel_vaccination_request(Thing *person);
u8 remove_person_from_vaccination_queue(Thing *person);
void compute_help_text(s16 x, s16 y, u8 help_group);
void draw_help_text(void);
void cancel_helpmsg(void);
void show_help_page(u32 page, u32 sub);
void finish_composite_help(void);
void check_cd_available(char *path);
void req11_sell_object(const u16 idx, const s32 amount);
void create_numfx(u8 type, s16 value, u16 pos, u8 colour, u8 height);
void draw_load_game(void);
void process_keys_load_game(void);
void process_mouse_load_game(void);
void play_winlose(void);
void set_edit_room_requester_mode(const u8 mode);
void open_edit_room_requester(void);
void go_back_to_place_object_requester(u8 restore_purchases);
u32 count_and_flag_contagious_peeps(const u8 illness, const u8 mode, const u8 owner);
void close_sound_heap(void);
void process_sound_heap(void);
u8 earthquake(const s8 magnitude, const u8 chance);
void display_choice(void);
void display_choice_screen(const u8 owner, const u8 type, const s16 data, const s16 timeout);
void packet_choice_result(u8 button);
void restore_display_screen(void);
void draw_it(void);
void special_refresh(void);
void set_all_refresh_zones_in_window(const s32 x1, const s32 y1, const s32 x2, const s32 y2, const u8 refresh_count);
void find_common_window(s32 x0, s32 y0, s32 x1, s32 y1, const s32 wx0, const s32 wy0, const s32 wx1, const s32 wy1);
void goto_cell_in_window(const u8 win, const s32 cell);
void goto_column_row_in_window(const u8 win, const s32 x, const s32 y);
void find_cell_at_point(const u8 win, const s32 x, const s32 y);
void open_new_window(const u8 a, const u8 source, const u8 preset, const s32 x0, const s32 y0, const s32 x1,
                     const s32 y1);
void give_emergency_stats(u16 call, u8 owner);
void give_screen_xy_of_cell_at_column_row(const u8 win, const s32 x, const s32 y);
u32 old_newrand(void);
u16 saferand(void);
void set_icon_value(Thing *const peep, const s8 icon);
void reset_icon_value(Thing *const peep, const s8 icon);
void initialise_shadows(void);
void display_research(void);
void alter_research(u8 cat, u8 value, u8 owner);
void set_font_from_cache(const s32 font);
void return_from_screen(const u8 mouse_mode, const u8 in_game, const u8 sprites);
void load_ingame_pic(const u8 idx, const u8 fade, const u8 mouse_mode, const u8 map, TbScreenMode mode);
void fade_out_menu(void);
u8 display_loading_screen(void);
void clear_loading_screen(u8 *palette);
void set_default_player_details(void);
void recreate_map(const u8 flag);
void add_static_shadows(void);
void clear_sound_heap(void);
s8 load_level(const u8 lvl, const u8 full);
s16 save_game_valid(const s8 slot);
void setup_addresses(void);
void free_level(void);
void reset_earthquake_variables(void);
s32 read_level_number(s32 slot);
void load_balance(const u16 level);
void reset_advisor(void);
void init_ambients(void);
u8 save_player_details(void);
extern u8 for_sale[];

/*
 * The retail game defines or calls these with a different parameter list
 * from the demo function whose name config/functions.txt gives them, so they
 * keep their address names and are bound to that symbol explicitly.
 */
extern "C" {
u16 save_game(s8 slot, s8 port);
#pragma aux save_game "W?save_game$n(xc)s"
s16 load_game(s8 slot, s8 port);
#pragma aux load_game "W?load_game$n(xc)s"
u16 place_staff_on_map(u16 tile, u16 id, u8 force, u8 owner);
#pragma aux place_staff_on_map "W?place_staff_on_map$n(xusxusuc)s"
u16 place_object(u16 tile, u16 kind, u8 dir, u8 owner);
#pragma aux place_object "W?place_object$n(ususuc)us"
void set_destination_coords(Thing *w, u16 pos);
#pragma aux set_destination_coords "W?set_destination_coords$n(pn$Thing$$us)v"
void process_keys(u8 modifiers, u8 key);
#pragma aux process_keys "W?process_keys$n(ucuc)v"
u8 LbPaletteFindColour(u8 *palette, int r, int g, int b);
void open_requester(u8 idx, u8 flag);
u8 process_staff_complex(Thing *employee);
#pragma aux process_staff_complex "W?process_staff_complex$n(pn$Thing$$)uc"
#pragma aux open_requester "W?open_requester$n(uc)v"
void clear_cell_of_ratholes(u8 owner, u16 tile);
#pragma aux clear_cell_of_ratholes "W?clear_cell_of_ratholes$n(ucus)v"
u8 door(u16 tile, u8 parity);
#pragma aux door "W?door$n(xusxuc)uc"
u8 check_okay_to_save(u8 warn);
#pragma aux check_okay_to_save "W?check_okay_to_save$n(uc)uc"
void setup_new_level(int level, u8 reload_balance);
#pragma aux setup_new_level "W?setup_new_level$n(xus)v"
Thing *find_waiting_surgeon(RoomInfo *room, u8 state);
#pragma aux find_waiting_surgeon "W?find_waiting_surgeon$n(pn$RoomInfo$$)pn$Thing$$"
u8 forget_about_my_slave(Thing *w, u16 action);
#pragma aux forget_about_my_slave "W?forget_about_my_slave$n(us)uc"
void swap_criteria_packets(u8 fail, u8 active);
#pragma aux swap_criteria_packets "W?swap_criteria_packets$n(uc)v"
u16 find_next_emergency_person(u16 id, u32 flags);
#pragma aux find_next_emergency_person "W?find_next_emergency_person$n(us)us"
void set_award_value(const u8 award, const s8 dir, const u8 tier, const s16 value, u8 type);
#pragma aux set_award_value "W?set_award_value$n(xucxucxucxsuc)v"
}

/*
 * Globals the retail code uses with a different type or layout from the
 * demo global that config/symbols.txt places at their address.
 */
extern "C" {

struct PlayerSlot {
    s32 active;
    u8 unknown4[0x20];
};

extern PlayerSlot D_001609c4[];
#pragma aux D_001609c4 "_D_001609c4"
}

#endif
