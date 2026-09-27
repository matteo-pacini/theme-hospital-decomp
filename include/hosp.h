#ifndef HOSP_H
#define HOSP_H

#include "bflib.h"

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

struct Map {
    s16 first;
    u8 overlay;
    u8 unknown3;
    u8 unknown4;
    u8 flags;
    u8 floor;
    u8 unknown7;
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
    s8 f1;
    s8 f2;
    s8 f3;
    u16 unknown4;
    u16 unknown6;
    u8 f8;
    u8 unknown9[3];
    u16 first_object;
    u16 first;
    u8 unknown16[4];
    u16 count;
    u32 unknown22;
    u8 unknown26;
    u8 f1b;
    u8 owner;
    u8 unknown29;
    u8 f1e;
    u8 unknown31;
    u8 unknown32;
    u8 unknown33;
    u8 f22;
};

struct Thing {
    s16 prev;
    s16 next;
    MapPos x;
    MapPos y;
    u16 sprite;
    s16 height;
    u8 renderer;
    u8 kind;
    u8 subtype;
    u8 anim_speed;
    u8 palette;
    u8 unknown17;
    u8 unknown18;
    u8 unknown19;
    u8 unknown20;
    s8 depth;
    u8 owner;
    u8 unknown23[32];
    union {
        s32 mode;
        s16 offset;
        u16 link;
        RoomInfo *room;
    };
    union {
        s32 value;
        struct {
            s16 timer;
            u16 unknown61;
        };
    };
    union {
        char text[4];
        u16 unknown63;
    };
    u16 icon;
    u16 unknown69;
    u16 unknown71;
    u16 unknown73;
    u8 unknown75[2];
    u16 room_prev;
    u16 room_next;
    u16 unknown81;
    u8 unknown83[2];
    u16 unknown85;
    u8 unknown87[2];
    u16 unknown89;
    u16 unknown91;
    u8 unknown93;
    union {
        u8 state_a;
        struct {
            u8 unknown94;
            u16 group;
        };
    };
    u8 unknown97[3];
    union {
        char label[0x44];
        struct {
            u16 unknown100;
            u8 state_b;
            u8 unknown103;
            u8 unknown104;
            u8 unknown105;
            u8 unknown106;
            u8 unknown107;
            s32 unknown108;
            u8 unknown112[4];
            union {
                u32 flags32;
                struct {
                    union {
                        u16 flags;
                        struct {
                            u8 f74;
                            u8 f75;
                        };
                    };
                    u8 unknown118;
                    u8 unknown119;
                };
            };
            u8 unknown120[2];
            s16 unknown122;
            u16 unknown124;
            u8 unknown126[2];
            u16 unknown128;
            s16 unknown130;
            u8 unknown132[2];
            s16 unknown134;
            s16 unknown136;
            u8 unknown138[2];
            u16 unknown140;
            u8 f8e;
            u8 f8f;
            u8 unknown144[2];
            union {
                s16 unknown146;
                struct {
                    u8 unknown146_lo;
                    u8 f93;
                };
            };
            u8 unknown148[10];
            u8 unknown158[10];
        };
    };
    u8 style;
    u8 unknown169;
    u8 unknown170;
    u8 unknown171;
    u8 unknown172[2];
    u8 unknown174;
};

struct HospitalRoom {
    s32 unknown0;
    u8 unknown4[8];
    s32 unknownC;
    s32 unknown10;
    s32 unknown14;
    u16 unknown18;
    u8 unknown1A;
    u8 unknown1B;
    u8 unknown1C;
    u8 unknown1D;
    u16 unknown1E;
    u16 unknown20;
    u8 unknown22[2];
    u16 unknown24;
};

struct Equipment {
    u8 unknown0;
    u8 unknown1[13];
};

struct ThingPager {
    u8 unknown0[1];
};

struct AllHosps {
    u8 unknown0[0x2a];
    u8 f2a;
    u8 f2b;
    u8 unknown2c[9];
    s32 f35;
    u8 unknown39[4];
    s32 f3d;
    u8 unknown41[0x18e];
    s16 unknown1cf[4];
    u8 unknown1d7[2];
    u16 f1d9;
    u8 unknown1db[7];
    u8 f1e2;
    u8 unknown1e3[2];
    u8 f1e5;
    u8 unknown1e6[0xb];
    u16 f1f1;
    u32 f1f3;
    u8 unknown1f7[3];
    u8 f1fa;
    u8 unknown1fb[8];
    u16 f203;
    u16 f205;
    u16 f207;
    u8 unknown209[2];
    u8 f20b;
    u8 f20c;
    u8 f20d;
    u8 f20e;
    u8 unknown20f[8];
    u16 f217;
    u16 f219;
    u16 f21b;
    u16 f21d;
    u8 f21f;
    HospitalRoom rooms[0x2e];
    u8 unknown8f4[0x32];
    union {
        Equipment equipment[0x3e];
        struct {
            u8 unknown926[0x358];
            s32 unknownc7e;
        };
    };
    u8 unknownc8a[0xa];
    u8 fc94;
    u8 fc95;
    u8 unknownc96[0x91];
    s16 fd27;
    u16 fd29;
    u8 unknownd2b[0x242];
    ThingPager ff6d;
    u8 unknownf6e[0x249];
    u16 unknown11b7;
    u8 unknown11b9[3];
    u8 f11bc;
    u8 unknown11bd[0x31];
    u16 unknown11ee;
    u16 unknown11f0;
    u8 unknown11f2[2];
    u16 f11f4;
    u16 f11f6;
    u8 unknown11f8[0x5e];
    u8 unknown1256;
    u16 unknown1257;
};

struct GameControl {
    u8 unknown0[0x64f];
    u8 unknown64f;
    u8 unknown650;
};

struct ListHead {
    u8 unknown0[0x7c];
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
    u8 pad[2];
    u8 owner;
};

struct Window {
    s16 x;
    s16 y;
    u8 unknown4[0x1e];
};

struct Expertise {
    u16 unknown0;
    u8 unknown2[2];
    u8 kind;
    u8 unknown5[3];
    u8 unknown8;
    u8 category;
    u8 unknown10[9];
};

struct ObjectControl {
    u8 unknown0[2];
    u8 shapes[4];
    u8 unknown6[0x16];
    u8 unknown1c;
    u8 unknown1d;
    u16 unknown1e;
    u8 unknown20[5];
};

struct RoomControl {
    u16 id;
    u8 unknown2[0xa];
    u8 fc;
    u8 unknownd[0x17];
    u8 unknown36;
    u8 unknown25[5];
};

struct Shapes {
    s8 count;
    u16 points[0x15];
    u8 unknown2b;
};

struct Blueprint {
    u16 anim;
    u8 unknown2[6];
};

struct VXY {
    s16 x;
    s16 y;
};

struct AnimStart {
    u16 idle;
    u8 unknown2[8];
    u16 unknownA;
    u8 unknownC[6];
    u16 unknown12;
    u8 unknown14[0x104];
};

struct AnimTable {
    char name[0x28];
    u16 f28;
    u8 unknown2a[8];
    u32 f32;
    u32 f36;
};

struct CompFrame {
    u8 unknown0[5];
    u8 length;
    u8 unknown6;
    u8 flags;
    u16 next;
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

struct PixBuffer {
    u8 unknown0[0x2c];
};

struct menu {
    u8 unknown0;
    u8 unknown1[6];
    u8 unknown7;
};

struct HospPlayerDetails {
    char unknown0[0x11];
    u8 unknown11;
    u8 unknown12[0xa];
};

struct Colors {
    u8 *unknown0;
    u8 unknown4[3];
    u8 unknown7;
    u8 unknown8;
    u8 unknown9;
    u8 unknownA;
    u8 unknownB;
    u8 unknownC;
    u8 unknownD;
    u8 unknownE;
    u8 unknownF;
    u8 unknown10;
    u8 unknown11;
    u8 unknown12;
    u8 unknown13;
    u8 unknown14;
};

struct Packet {
    u8 unknown0[4];
};

struct NetText {
    u8 unknown0;
    u8 unknown1[0x31];
};

struct Blob63 {
    u8 unknown0[0x3f];
};

struct Game {
    u8 unknown0[0x1e];
    u8 unknown1e;
    u8 unknown1f[2];
    u8 unknown21;
    u8 unknown22[3];
    u8 unknown25;
    u8 unknown26;
    u8 unknown27[8];
    u16 unknown2f;
    u8 unknown31[2];
    union {
        Blob63 unknown33;
        struct {
            u8 unknown33_[0x11];
            u8 unknown44;
        };
    };
    u8 unknown72[3];
    u16 unknown75;
    u16 unknown77;
    u16 unknown79;
};

struct Statics {
    s32 unknown0;
    u16 unknown4;
    u8 unknown6[0xdf];
    u8 unknowne5;
    u8 unknowne6;
    u8 unknowne7;
    u8 unknowne8[0x14];
    u8 unknownfc;
    u8 unknownfd;
};

struct Scroll {
    u8 unknown0;
    u8 unknown1;
    u8 unknown2;
    u8 unknown3;
};

struct Info {
    u8 unknown0[0x5c];
    s32 unknown5c;
    s32 unknown60;
};

struct HospWindow {
    u8 unknown0[0x30];
    s32 unknown30;
    u8 unknown34[8];
    s32 unknown3c;
    u8 unknown40[8];
    s32 unknown48;
    u8 unknown4c[8];
    s32 unknown54;
    u8 unknown58[0x38];
    s32 unknown90;
    u8 unknown94[8];
    s32 unknown9c;
    u8 unknowna0[0x220];
    u8 unknown2c0;
    u8 unknown2c1[0x2a];
    u8 unknown2eb;
    u8 unknown2ec;
};

struct Required {
    u8 unknown0[3];
    u8 unknown3;
    u8 unknown4;
    u8 unknown5;
    u8 unknown6;
    u8 unknown7;
    u8 unknown8;
    u8 unknown9;
};

struct Debug {
    u8 unknown0[8];
    u32 unknown8;
    u8 unknownc[0xc8];
    u8 unknownd4;
    u8 unknownd5[2];
    u8 unknownd7;
    u8 unknownd8[0xa];
    u8 unknowne2;
    u8 unknowne3[0x27];
    u8 unknown10a;
    u8 unknown10b[0xb];
    char unknown116[0xc8];
    u8 unknown1de;
    u8 unknown1df;
    s32 unknown1e0;
    u8 *unknown1e4;
    u8 unknown1e8[0x12];
    u8 unknown1fa;
};

struct GameCounts {
    u8 unknown0[0x14];
    u32 unknown14;
    u32 unknown18;
};

struct Menu {
    u16 unknown0;
    u8 unknown2;
    u8 unknown3;
};

struct Req {
    u16 unknown0;
    u8 unknown2;
    u8 unknown3;
    u16 unknown4;
    u16 unknown6;
    u8 unknown8;
    u8 unknown9;
    u8 unknownA;
    u8 unknownB;
};

struct Level {
    u8 unknown0[0x28024];
    u16 unknown28024[4];
};

struct Icons {
    u8 unknown0[0x62];
    s16 x;
    s16 y;
};

struct ScreenInfo {
    u8 unknown0[0xa];
};

struct NonResettingAdvisorCounts {
    u16 unknown0;
    u16 unknown2;
    u8 unknown4[2];
    s32 unknown6;
};

struct WinLoseNet {
    u16 unknown0;
};

struct Trophies {
    u8 unknown0;
};

struct Awards {
    u8 unknown0;
};

struct orthonav_path {
    u8 unknown0[0xc];
    u8 result;
    u8 unknownd[3];
};

/*
 * Layouts of globals that are still named after their address (D_XXXXXXXX)
 * but are used by more than one source file.
 */

struct MemRegion {
    u32 addr;
    s32 size;
};

struct Event {
    u8 unknown0;
    u8 type;
    u16 value;
    u8 flags;
    union {
        u16 arg;
        u8 argb[2];
    };
    u32 checksum;
    u8 unknownB[4];
};

struct View {
    s32 unknown0;
    s32 unknown4;
    s32 unknown8;
    s32 unknownC;
    u8 unknown10[0x2d0];
    s32 unknown2e0;
    s32 unknown2e4;
    s32 unknown2e8;
    s32 unknown2ec;
};

struct Point {
    s32 x;
    s32 y;
};

struct Button {
    s16 x;
    s16 y;
    u16 width;
    u16 height;
    void (*callback)(int index);
    u16 unknownC;
    u16 unknownE;
    u16 sprite;
};

extern Blocks blocks[];
extern const u8 blocks_backup[];
extern const u8 *pointer_second_frame_check;
extern VXY around_vxy[];
extern s16 around[];
extern s16 next_angle[];
extern Blueprint blueprint[];
extern ObjectControl objects[];
extern Shapes shapes[];
extern s8 change_person_who[][15];
extern RoomControl rooms[];
extern const u8 ingame_pointer_second_frame[];
extern const u8 screen_pointer_second_frame[];
extern const u8 ingame_pointer_hotspots[];
extern const u8 screen_pointer_hotspots[];
extern u8 abilities[][6];
extern Expertise expertise[];
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
extern Map *map;
extern Map *end_map;
extern TbSprite *game_panel;
extern TbSprite *pulldv;
extern ToDraw draw_cells[];
extern PixBuffer pixBuf;
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
extern Game gamevar;
extern Statics statvar;
extern Scroll scroll;
extern Info info;
extern HospWindow window;
extern Required required;
extern Debug debug;
extern GameCounts gamecounts;
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
extern Req crequester;
extern Level *level;
extern void *midi_buffer;
extern u16 popup_id[];
extern Icons *panel;
extern u16 available_buttons;
extern s16 choice_spare_data;
extern ScreenInfo choice_backup;
extern ScreenInfo year_end_backup;
extern NonResettingAdvisorCounts NRAdcounts;
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
extern Window *open_windows[];
extern s8 *button_sprite_data;
extern s8 *end_button_sprite_data;
extern TbSprite *button_sprite;
extern TbSprite *end_button_sprites;
extern u8 mouse_was_on_window;
extern volatile s32 mouse_dx;
extern volatile s32 mouse_dy;

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
u8 process_lost_focus_display(u8 flag);
void display_mission(void);
void display_drugs(void);
void process_drug_panel(void);
void price_up(void);
void price_down(void);
void process_drug_keys(void);
void process_drug_button(u8 action);
u8 get_next_illness(const u8 forward, const u8 start);
void setup_drug(u16 a, u16 b);
void drugs_help_concentrate(void);
void drugs_help_cure_type(void);
void drugs_help_handled(void);
void update_cure_costs(void);
void update_single_cure_cost(u8 room, u8 hospital);
void do_mapwho(const Map *const cell, const s32 x, const s32 y, const s32 left, const s32 top);
void mapwho_person(Thing *w, s32 x, s32 y, s32 left, s32 top);
void mapwho_staff(Thing *w, s32 x, s32 y, s32 left, s32 top);
void mapwho_object(Thing *w, const s32 x, const s32 y, const s32 left, const s32 top);
void mapwho_reaper(Thing *w, s32 x, s32 y);
void mapwho_rat(Thing *w, const s32 x, const s32 y, const s32 left, const s32 top);
void mapwho_bang(Thing *w, s32 x, s32 y);
void mapwho_helicopter(Thing *w, s32 x, s32 y);
void mapwho_numfx(Thing *w, s32 x, s32 y);
void mapwho_mayor(Thing *w, s32 x, s32 y, s32 left, s32 top);
void mapwho_shadow(Thing *w, s32 x, s32 y);
void insert_node(Thing *w, MapPos x, MapPos y);
u16 move_node(Thing *w, MapPos x, MapPos y);
void person_waits_for_busy(Thing *w);
void person_arrives_at_door(Thing *w);
void person_in_queue(Thing *w);
void person_entertains_themself(Thing *w);
u16 find_standing_place(Thing *w);
void person_go_home(Thing *w);
void remove_person(Thing *w);
void move_person_in_room_list(Thing *w, u16 id);
void insert_person_in_room_list(RoomInfo *room, Thing *w);
void nav_debug_state(Thing *w);
u8 transform_peep_who(Thing *w, u8 level);
u16 call_person_to_room(Thing *w, u16 id, u16 action, u16 arg, u8 follow);
u8 call_person_again(Thing *w, u16 action, u16 arg);
void send_person_to_bed(Thing *w);
void person_in_bed(Thing *w);
u16 find_peep(u16 room_id, u8 random, u8 renderer, u8 kind, u8 owner);
void start_person_going(Thing *w);
void person_nav_failure(Thing *w);
void person_fills(Thing *w);
void person_energises(Thing *w);
void person_in_bogs(Thing *w);
void leave_toilets(Thing *w);
void return_to_queue_area(Thing *w);
void play_person_showanim(Thing *w);
void page_staff_i_need(Thing *w, RoomInfo *room);
void send_person_somewhere_else(Thing *w);
void refer_patient(Thing *w, u8 owner);
void person_waits_for_decision(Thing *w);
u16 make_a_playercall(const u16 type, const u16 x, const s16 y, u8 owner);
void get_new_popup_head(u16 type, u8 owner);
void save_popup(u16 type);
void restore_popup(u16 type);
u16 get_playercall(const u16 type, u16 id, u8 owner);
u16 check_playercall(u16 type, u16 x, u8 owner);
void expire_playercalls(void);
u16 call_no_treatment(Thing *e, char **strs);
void process_no_treatment_buttons(u8 n);
void person_arrived_from_call(Thing *e);
u32 person_ok_to_call(Thing *e);
u16 announce_new_illness_fax(const u8 kind, char **strs);
void process_announce_new_illness_buttons(u8 n);
void force_peep_to_be_cured(Thing *e);
void interrupt_person(Thing *e);
u16 div_err(void);
void setup_colors(void);
u16 can_i_get_to(Thing *e, u16 pos);
void remove_thing(Thing *e);
void count_hospital_area(u16 player);
Thing *search_mapwho_for_room_door(Map *t, u16 id);
void screen_shot(void);
RoomInfo *get_room_info_rand(u8 kind, u8 mode, u8 owner);
RoomInfo *get_room_info(u8 kind, u8 mode, u8 owner);
RoomInfo *get_nearest_room_info(Thing *e, u8 kind);
void cheat(u32 flags);
void set_mouse_mode(const u8 mode);
void generate_bw_remap(u8 *palette, u8 *out);
u8 check_points_connected(u16 from, u16 to);
u8 direction_to_face(Thing *a, Thing *b);
void convert_MW_to_xy(u16 pos, s8 *x, s8 *y);
void measure_objects(void);
int descend(const void *a, const void *b);
int ascend(const void *a, const void *b);
void set_mouse_sprite(const u8 idx);
u16 can_i_go_near_point(Thing *e, u16 target);
u8 am_i_near_point(Thing *e, u16 pos);
void vary_speed(s16 speed);
void compute_whether_user_not_busy(void);
u32 calc_room_value(RoomInfo *r);
Thing *find_person_in_room(RoomInfo *room, u16 kind, u16 sub);
void setup_abort_all_requesters(void);
void copy_screen_to_wscreen(const u8 *src);
void copy_screen_from_wscreen(u8 *dst);
void cancel_top_level_requesters(s32 n);
Thing *allocate_thing(u32 n);
s32 over_hotspot(s16 x, s16 y, s16 rx, s16 ry, s16 w, s16 h);
void draw_frame(const s16 x, const s16 y, const s16 w, const s16 h, const u16 color);
u8 hosp_play_smk(const u8 idx);
void hosp_dos_printf(char *fmt, ...);
void recall_mappos(u8 i);
void setup_key_mappos(void);
void hosp_fade_palette(u8 *palette, u8 steps, TbPaletteFadeFlag flag);
void close_crequester(u8 i);
void process_turn(void);
void temp_place_door(u16 a, u8 b);
void place_bp_door(u8 a, u8 b);
RoomInfo *which_room(u16 tile);
RoomInfo *is_this_a_door(u16 tile);
void object_placed(void);
void init_place_object(u8 a, u8 b, u16 c, s16 d);
void init_edit_room(RoomInfo *room);
void setup_place_object_requester(void);
u8 next_object_to_build(void);
void open_room(u8 a);
void pick_up_object(u16 a, u8 b);
void clear_build_objects(u8 a);
void clear_object_drag_shape(void);
void sack_staff(Thing *e, u16 a);
s32 pay_staff_bonus(Thing *e, u16 a, u16 b, u16 c, u8 d);
void staff_picked_up(Thing *e);
void interrupt_staff(Thing *e);
void staff_join_new_employer(u16 a, u16 b);
void increase_staff_salaries(Thing *e, u16 a, u16 b, u8 c);
s16 setup_lang(void);
void draw_a_frame(const u16 sprite, const s16 x, const s16 y, const u8 flags, const u8 palette);
void draw_special_id_frame(Thing *w, const s16 x, const s16 y);
void draw_split_frame(Thing *w, const s16 x, const s16 y);
void draw_energise_frame(Thing *w, const s16 x, const s16 y);
void draw_a_frame_scaled(const u16 sprite, const s16 x, const s16 y, u8 flags, const u8 palette, s32 scale);
void draw_hilight_frame(Thing *w, const s16 x, const s16 y);
void draw_from_special_id_frame(const u16 icon, const s16 x, const s16 y, u8 *a, u8 *label, const u8 palette);
void draw_text_on_map(char *text, const u8 font, const s32 x, const s32 y);
void draw_money(s32 x, s32 y, s32 value);
void setup_panel_icons(const u8 mode);
void clear_panel_info_bar(void);
void delete_panel_popup(PlayerCall *p);
void add_panel_popup(PlayerCall *p);
u16 find_object(u16 tile, u16 a, u8 b, u8 c, u8 owner);
u16 find_object_anywhere(u16 a, u8 b, u16 c, u8 owner);
u16 find_nearest_free_object(u16 id, u8 a);
u16 find_nearest_manned_reception(Thing *w, const u8 a, const u8 b);
s16 place_small_thing(u16 a, u16 b, u8 c);
void replace_machine(Thing *e);
void setup_next_treatment(Thing *a, Thing *w);
void next_treatment_now_setup(Thing *e, u8 a);
void best_guess_treatment(Thing *e);
u16 find_possible_diagnosis(Thing *w);
void record_person_as_patient(Thing *w);
u8 process_person_complex(Thing *w);
u8 set_complex(Thing *w, u16 action, u16 arg);
void abandon_complex(Thing *w);
void interrupt_complex(Thing *e);
void display_policy(void);
void init_blueprint(u8 a, u8 b);
void modify_blueprint(u8 a, u8 b, u16 c);
u8 commit_blueprint(u8 a);
void uncommit_blueprint(u8 a);
void edit_blueprint(u8 a);
void process_blueprint_zones(void);
void commit_doors(u8 a);
u8 move_peep_from_blueprint(Thing *w);
void delete_blueprint_room(u8 a);
void allocate_roominfo(const u8 a, const u8 b);
u16 page_member_of_staff(u16 a, u16 room, u8 b, s8 c, u8 owner);
RoomInfo *get_expertise_dest(u16 type, Thing *w);
void front_queue(const u16 a, const u16 b);
void back_queue(const u16 a, const u16 b);
void leave_queue(Thing *w);
u16 which_queue(Thing *w);
void queue_person_go_home(const u16 a, const u16 b);
void queue_person_go_reception(const u16 a, const u16 b);
void queue_person_to_new_queue(const u16 a, const u16 b);
void DrawWindows(Window **windows);
u8 remove_window(Window **windows, Window *w);
u8 OpenWindow(Window **windows, Window *w, u16 a);
u16 CountWindows(Window **windows);
void change_balance(const u8 a, const s32 b, const u16 c, const u16 d);
void month_end(void);
void year_end(void);
void bank_statement(void);
u16 exit_quick_requester(void);
void daily_admin(void);
void twice_daily_admin(void);
void calculate_value(const u8 a);
void adjust_build_palette_for_shop(RoomInfo *room);
void clear_req_indices(void);
void set_buyable_items(u16 a);
void buy_compulsory_items(const u16 a);
void req1_reset_rooms(u16 a);
u16 room_available(u16 room);
void req1_buy_object(const u16 a, const s32 b);
void orthonav_init(orthonav_path *path, s8 a, s8 b, s8 c, s8 d);
void display_peep_overview(void);
void reputation_affected(const u8 a, const u16 b, const u16 c, const u8 d, const s32 e);
void check_game_over(void);
void check_quarterly_game_over(void);
s32 max_price_for_treatment(const u16 room, const u8 hospital);
void satisfied_net_criteria(const u8 a, const u8 b);
void process_mouse(void);
void process_things(Thing *a, Thing *b);
void display_advisor(void);
void announce_this(const u8 a, const u32 b);
void draw_main_menu(void);
void process_keys_main_menu(void);
void process_mouse_main_menu(void);
void process_mouse_town_select(void);
void buy_land(u16 a);
void plot_bid_money(u8 a);
void setup_town_bid(void);
u8 draw_area_select(void);
void process_keys_area_select(void);
void process_mouse_area_select(void);
void req2_reset(void);
u16 req2_alter_staff(u8 a, u16 b, u16 c);
void req2_mark_employing(const u16 a);
void req2_exit_employing(const u16 a);
void open_staff_resign_requester(Thing *e);
void req12_clicked_sack(void);
void open_yes_no_requester(u8 a, u16 b, u16 c, u8 d, u16 e, u16 f);
void close_yes_no_requester(u8 a);
void req13_cancel_check(void);
void modify_person(Thing *w);
void close_query(u8 a);
s32 SWIKeyboardOpen(void);
s32 SWIKeyboardClose(void);
void machine_disaster(Thing *e);
void create_explosion(u16 a, u16 b);
void what_is_required(u8 room, u8 hospital);
void draw_graphs(u16 a, u16 b, u16 c, u16 d);
s32 read_config_file(void);
void place_litter(u16 tile, s8 owner, u8 a);
u8 thingpage_request(ThingPager *p, u16 a, u8 b);
void display_manager(void);
void draw_network_select(void);
void process_keys_network_select(void);
void process_mouse_network_select(void);
void do_net_text(void);
void net_text_display(u8 a);
void net_text_keypress(u8 a, u8 b);
void net_text_delete(u8 a);
u8 network_num_players(void);
void network_lost_focus(s32 a);
void network_gained_focus(s32 a);
void draw_lost_focus_screen(void);
u8 has_any_player_lost_focus(void);
void close_irequesters(void);
void adjust_handyman_values(const u16 a, const u8 b);
u8 setup_sound(void);
void do_ambients(void);
void play_ambient(u8 sound);
void play_sound(u32 a, u16 b, s32 c, s32 d);
void manage_sound(void);
void manage_background_sounds(void);
void play_next_midi(void);
void setup_menus(void);
void check_save_games(void);
u16 check_frame(u16 anim, Thing *w);
void stop_midi(void);
void draw_nickname(void);
void process_keys_nickname(void);
void process_mouse_nickname(void);
void display_slist(void);
void request_vaccination(Thing *e);
void cancel_vaccination_request(Thing *e);
u8 remove_person_from_vaccination_queue(Thing *e);
void compute_help_text(s16 a, s16 b, u8 c);
void draw_help_text(void);
void cancel_helpmsg(void);
void show_help_page(u32 a, u32 b);
void finish_composite_help(void);
void check_cd_available(char *path);
void req11_sell_object(const u16 a, const s32 b);
void create_numfx(u8 a, s16 b, u16 c, u8 d, u8 e);
void draw_load_game(void);
void process_keys_load_game(void);
void process_mouse_load_game(void);
void play_winlose(void);
void set_edit_room_requester_mode(const u8 a);
void open_edit_room_requester(void);
void go_back_to_place_object_requester(u8 a);
u32 count_and_flag_contagious_peeps(const u8 a, const u8 b, const u8 c);
void close_sound_heap(void);
void process_sound_heap(void);
u8 earthquake(const s8 a, const u8 b);
void display_choice(void);
void display_choice_screen(const u8 a, const u8 b, const s16 c, const s16 d);
void packet_choice_result(u8 n);
void restore_display_screen(void);
void draw_it(void);
void special_refresh(void);
void set_all_refresh_zones_in_window(const s32 x, const s32 y, const s32 w, const s32 h, const u8 a);
void goto_cell_in_window(const u8 a, const s32 b);
void goto_column_row_in_window(const u8 a, const s32 x, const s32 y);
void find_cell_at_point(const u8 a, const s32 x, const s32 y);
void open_new_window(const u8 a, const u8 b, const u8 c, const s32 x0, const s32 y0, const s32 x1, const s32 y1);
void give_emergency_stats(u16 a, u8 b);
u32 old_newrand(void);
u16 saferand(void);
void set_icon_value(Thing *const w, const s8 a);
void reset_icon_value(Thing *const w, const s8 a);
void initialise_shadows(void);
void display_research(void);
void alter_research(u8 a, u8 b, u8 c);
void set_font_from_cache(const s32 font);
void return_from_screen(const u8 a, const u8 b, const u8 c);
void load_ingame_pic(const u8 a, const u8 b, const u8 c, const u8 d, TbScreenMode mode);
void fade_out_menu(void);
u8 display_loading_screen(void);
void clear_loading_screen(u8 *palette);
void set_default_player_details(void);

/*
 * The retail game defines or calls these with a different parameter list
 * from the demo function whose name config/functions.txt gives them, so they
 * keep their address names and are bound to that symbol explicitly.
 */
extern "C" {
u16 fn_00021818(s8 slot, s8 player);
#pragma aux fn_00021818 "W?save_game$n(xc)s"
s16 fn_00021c38(s8 slot, s8 player);
#pragma aux fn_00021c38 "W?load_game$n(xc)s"
u16 fn_00029c4c(int a, int b, int c, u8 owner);
#pragma aux fn_00029c4c "W?place_staff_on_map$n(xusxusuc)s"
u16 fn_00037098(int a, u16 b, int c, u8 owner);
#pragma aux fn_00037098 "W?place_object$n(ususuc)us"
void fn_0004195c(Thing *w, u16 a);
#pragma aux fn_0004195c "W?nav_init$n(pn$Thing$$)v"
void fn_00074220(int a, int b);
#pragma aux fn_00074220 "W?keybuffer_clear$n()v"
u8 fn_000c960c(u8 *palette, int r, int g, int b);
#pragma aux fn_000c960c "W?draw_bdem_unavailable_message$n()v"
void fn_0001bc94(u8 idx, u8 flag);
#pragma aux fn_0001bc94 "W?open_requester$n(uc)v"
}

/*
 * Globals the retail code uses with a different type or layout from the
 * demo global that config/symbols.txt places at their address.
 */
extern "C" {
extern s32 D_00188c34;
#pragma aux D_00188c34 "W?research_names$npnpna"

struct PlayerSlot {
    s32 active;
    u8 unknown4[0x20];
};

extern PlayerSlot D_001609c4[];
#pragma aux D_001609c4 "W?popup_store$n[][9]$PlayerCall$$"
}

#endif
