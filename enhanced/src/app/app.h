#pragma once

// The application: the window, the game in play and the interface round it.
// Its member functions are defined in app_*.cpp, one file by area.

#include "app/audio/mixer.h"
#include "app/drive.h"
#include "app/install_job.h"
#include "app/paths.h"
#include "app/player_details.h"
#include "app/screen_fade.h"
#include "app/settings.h"
#include "app/ui/tools.h"
#include "app/upscale_texture.h"
#include "core/audio/music.h"
#include "core/audio/sounds.h"
#include "core/data/gamedata.h"
#include "core/render/surface.h"
#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/fax.h"
#include "core/ui/helpmsg.h"
#include "core/ui/info_windows.h"
#include "core/ui/jukebox.h"
#include "core/ui/loading_screen.h"
#include "core/ui/menubar.h"
#include "core/ui/popups.h"
#include "core/ui/queue_window.h"
#include "core/ui/staff_demand.h"
#include "core/ui/startup.h"
#include "core/ui/winlose.h"
#include "core/ui/yearend.h"
#include "core/world/game.h"
#include "core/world/savegame.h"

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The folder or file chosen in a native dialog, handed over from its
// callback, which SDL may call on another thread.
struct FolderPick {
    std::mutex mutex;
    std::optional<std::string> chosen;
};

// A level as it starts: its map and objects, the shadowed blocks, and the
// scratch surface the view is drawn with.
struct LevelView {
    Game game;
    std::unique_ptr<ShadowSprites> shadows;
    Surface scratch;
    // What the interface left on the map for the next draw.
    ViewOverlays overlays;
};

// A debug report's contents, held in memory between the capture (at F10 or
// the F12 button) and the write (when the note box is answered).
struct DebugCapture {
    SDL_Surface *shot = nullptr;
    std::string shot_error;
    std::string state;
    std::vector<uint8_t> snapshot;
    bool has_snapshot = false;
    std::string snapshot_error;
    std::string commands;
    std::string system;
};

// The F10 quick report: the capture held while its note box is up.
struct QuickReport {
    bool armed = false;     // F10 pressed: capture at the frame's start
    bool open = false;      // the note box is up
    bool immediate = false; // Shift+F10: write at once, with no box
    bool focus = false;     // focus the note box's text field
    bool have_pause = false;
    bool restore_paused = false;
    DebugCapture capture;
    std::array<char, 4096> note{};
    std::string last_report; // the folder of the last report written
    std::string toast;
    uint32_t toast_until = 0;
};

// What write_debug_report did.
struct DebugReportResult {
    std::string message;
    std::filesystem::path dir;
};

class App : public DriveHost
{
  public:
    bool init(int argc, char **argv);
    // Starts the frame clock; call once after init, before the first iterate.
    void begin();
    // One pass of the main loop: the driver's step, update and render. False
    // when the app is done; exit_code() then tells how.
    bool iterate();
    void on_event(const SDL_Event &e) { handle_event(e); }
    // A running install is cancelled and waited for when the app is destroyed.
    void shutdown();
    // 1 when --install-from failed or a drive script's expectations did, else 0.
    int exit_code() const { return install_failed_ || (driver_ && driver_->failures() > 0) ? 1 : 0; }

  private:
    // --drive SCRIPT, --control, --out DIR, --size WxH, --visible,
    // --default-speed, --real-time, --fades.
    void parse_drive_args(int argc, char **argv);
    // --user-data (skip the development folder, so a development build looks
    // only in the game folder as a normal start does), --drop PATH (handle PATH as a file dropped
    // on the window at start, for tests) and --install-from PATH, returned.
    std::optional<std::filesystem::path> parse_setup_args(int argc, char **argv);
    // Installs `from` (an installer or a game folder) before the window
    // opens, reporting on stderr (stdout carries --control's answers);
    // false when it failed.
    bool install_from_command_line(const std::filesystem::path &from);
    // The setup screen's jobs: starts one on the worker thread, takes its
    // result when done, and loads the game from what an install made.
    void start_setup_job(InstallKind kind, const std::filesystem::path &source);
    void poll_setup();
    void poll_setup_job();
    // The event of --drop, queued at start.
    void queue_drop_arg();
    void setup_step();
    void refresh_game_dir_state();
    SetupView setup_view();
    // The job's upscaling: what the setup screen chose, saved as the setting.
    UpscaleBuild commit_upscale_choice();
    // One of TH_DRIVE_SLOTS (default 6) slots shared by every driven run on
    // the machine, under the repository's build/slots/drive.
    void hold_drive_slot();
    bool create_window(bool driving);
    void start_driver();
    // Milliseconds since start: the real clock, or the driven one, which
    // moves a fixed step per frame so driven runs repeat exactly.
    uint32_t ticks_ms() const;
    // Window points of map pixel `p` in the hospital view, false off the
    // window.
    bool drive_warp_world(Point p);
    bool drive_move_screen(int x, int y) override;
    bool drive_move_cell(int col, int row) override;
    bool drive_move_thing(uint16_t id) override;
    void drive_button(int button, bool down) override;
    bool drive_key(std::string_view name, bool down, uint16_t mods = 0) override;
    bool drive_text(std::string_view text) override;
    void drive_screenshot(const std::filesystem::path &path) override;
    void drive_screenshot_ui(const std::filesystem::path &path) override;
    std::string drive_state() const override;
    std::string drive_report_dir() const override;
    int64_t drive_turn() const override;
    bool drive_staff_demand(uint16_t id) override;
    bool drive_win_level() override;
    bool drive_lose_level() override;
    bool drive_watch(bool epidemic) override;
    bool drive_wheel(int notches) override;

    bool try_load_data(const std::filesystem::path &dir);
    // Sets upscale_ from --upscale, else the settings file.
    void choose_upscale();
    // Gives `data` its upscaled map sheets from the cache or by building
    // them; upscaling is off for the run if they cannot be made.
    void attach_upscaled(GameData &data, const std::filesystem::path &dir);
    // Where the upscaled sprites are kept: the user's folder, or the output
    // folder of a driven run.
    std::filesystem::path upscale_cache_dir() const;
    // Preselects the setup screen's upscaling choice from --upscale or the
    // settings, once.
    void init_upscale_choice();
    void load_level(int number);
    // Takes over `view` as the level in play and resets the interface round
    // it. `fresh` starts the advisor, the hospital settings and the insurers
    // as a level's start does; a loaded save has them already.
    void start_view(std::unique_ptr<LevelView> view, const Level &level, bool fresh);
    void reset_interface_for_level();
    // The save slot path for the player's name and the current save folder.
    std::filesystem::path save_path(int slot) const;
    // What a save of the game in play continues with: the balance id, the
    // tutorial and the view's camera (the level is the game's).
    SaveInfo current_save() const;
    // Marks which File > Save/Load slots hold a save (check_save_games).
    void refresh_save_slots();
    // Whether the game may be saved now (check_okay_to_save): in the normal
    // mouse mode with nothing being built, placed or edited.
    bool can_save(const Game &game) const;
    // Saves the game to `slot` (save_game) or loads `slot` over a level
    // (load_game); both show the panel message the original shows (0x18
    // saved, 0x19 cannot save, 0x1c load failed). perform_load returns
    // whether it loaded.
    void perform_save(int slot);
    bool perform_load(int slot);
    // display_panel_special_info: the misc_text message `id` on the panel's
    // information bar for `frames` frames (the timer is one a frame).
    void show_panel_message(uint8_t id, int frames);
    void draw_panel_special_info();
    void draw_panel_bar(const Game &game);
    // Saves the Continue slot when the game returns to the main menu
    // (quit_game 1: save_game(0x63)).
    void save_continue_slot();
    // The player's name for file names: the name screen's, or the default
    // name of a run that skipped it (--level).
    std::string player_name() const;
    // load_player_details: reads save/<name>.hdt (the defaults without one)
    // and applies it; save_player_details writes it, after each save.
    void load_player_details();
    void save_player_details();
    // apply_player_details: the audio, shadows and speed options.
    void apply_player_details();
    // Takes the live options and the advisor's flags into details_.
    void collect_player_details();
    // Gives a level's advisor the player's flags (they outlive the level).
    void give_advisor_flags(Game &game) const;
    // check_autosave (src/io.cpp): slot 8 on day 28.
    void check_autosave(const Game &game);
    void handle_event(const SDL_Event &e);
    // What comes during a fade is kept (hold_during_fade) and handled once it
    // is over (deliver_held_events).
    bool hold_during_fade(const SDL_Event &e);
    bool chart_escape(Game &game);
    void decline_win(Game &game, bool win);
    void mouse_motion(const SDL_MouseMotionEvent &motion);
    void mouse_wheel(const SDL_MouseWheelEvent &wheel);
    void deliver_held_events();
    void key_down(const SDL_KeyboardEvent &key);
    void key_up(const SDL_KeyboardEvent &key);
    void note_held_keys(const SDL_KeyboardEvent &key, bool down);
    void game_key(const SDL_KeyboardEvent &key);
    // Alt+S quick saves and Alt+L quick loads debug_save_slot; true when
    // `key` was one of them (keyboard.cpp's KC_S and KC_L).
    bool quick_save_load_key(const SDL_KeyboardEvent &key);
    // The speed keys 1-5: true when `key` is one and the speed was set.
    bool game_speed_key(const SDL_KeyboardEvent &key);
    // P pauses, T the town map, Shift+A the advisor, J the jukebox.
    void game_letter_key(const SDL_KeyboardEvent &key);
    // The cheat keys; true when `key` was one.
    bool cheat_key(const SDL_KeyboardEvent &key);
    // The quit keys; true when `key` is one.
    bool quit_key(const SDL_KeyboardEvent &key);
    // Alt+F1-F8 stores and Ctrl+F1-F8 recalls a view position
    // (record_mappos, recall_mappos); true when `key` was one.
    bool view_bookmark_key(const SDL_KeyboardEvent &key);
    void open_chart_key(int chart);
    void mouse_down(const SDL_MouseButtonEvent &button);
    void inspect_click(uint8_t button);
    void mouse_up(const SDL_MouseButtonEvent &button);
    // The mouse in a layer drawn at (ox, oy) at `scale`.
    UiMouse ui_mouse(int ox, int oy, int scale) const;
    void menu_step(int out_w, int scale);
    void perform(const MenuEvent &event);
    // File > Save and File > Load (pulldown_save and pulldown_load),
    // perform's, apart for their length.
    void perform_save_load(const MenuEvent &event);
    void play_menu_sound(MenuSound sound);
    void open_menu_screen(int chart);
    void update(float dt);
    void run_turns(float dt);
    // move_windows (src/cakewin.cpp): moves the view by the scroll direction
    // `dir` (the arrow keys' bits, 1 up, 2 down, 4 left, 8 right, plus the
    // window edge), accelerating while held and stopping at the scroll limit.
    void scroll_step(uint8_t dir);
    // The scroll direction this frame: the held keys, the pointer's window
    // edge replacing them (process_mouse's flags), an open pulldown ignoring
    // both (io.cpp), and opposite keys cancelling (process_game_keys).
    uint8_t scroll_direction();
    void render();
    // The palette fades and the loading screen (ScreenFader): queues the fade
    // of a change of screen and presents a fade's frame. True when the frame
    // was presented, so the screen is not drawn.
    bool fade_frame(int out_w, int out_h);
    // The screen shown, as the fades tell them apart.
    ScreenId screen_id() const;
    void draw_loading_picture(int out_w, int out_h);
    // Alt+C: the screen shot (screen_shot, src/utils.cpp:172) of the frame
    // being drawn, saved as scrshots/hospNNNN.png (downloaded in the browser).
    void take_screen_shot();
    // True when `key` is Alt+C.
    bool screen_shot_key(const SDL_KeyboardEvent &key);
    // Reads the render target back and saves it as a PNG (a driven run's
    // screenshot).
    void save_drive_screenshot(const std::filesystem::path &path);
    // The frame of what takes the whole window, if anything: the start-up
    // screens, the fax, a chart or the town map. False when it is the
    // hospital's.
    bool screen_step(int out_w, int out_h);
    void hospital_step(int out_w, int out_h);
    void resize_view(int vw, int vh);
    void view_sounds(int vw, int vh);
    void draw_view_overlays(int vw, int vh);
    void present_view(int vw, int vh, int z);
    bool pause_shaded() const;
    void shade_view();
    void shade_ui(UiLayer &layer);
    int zoom() const;
    int ui_scale() const;
    void make_ui_textures();
    // Frees the textures, cursors and surfaces the app owns.
    void destroy_textures();
    void interface_step(int out_w, int out_h, int scale);
    bool interface_escape(Game *game);
    InfoWindowResult interface_requesters(Game &game, const InterfaceEvents &events, const UiMouse &mouse);
    // The jukebox, hire staff and staff demand requesters' clicks, split from
    // interface_requesters.
    void interface_jukebox_input(Game &game, const InterfaceEvents &events, const UiMouse &mouse);
    void interface_hire_input(Game &game, const InterfaceEvents &events);
    void interface_staff_demand_input(Game &game, const InterfaceEvents &events);
    void interface_yes_no(Game *game, const InterfaceEvents &events);
    void interface_panel_action(Game &game, PanelAction action, bool placing);
    // The panel's edit room button (panel_l_button): toggles the edit-room
    // mouse mode. Returns whether it refused.
    bool panel_edit_room(Game &game);
    void interface_map_mouse(Game &game, const UiMouse &mouse, bool on_map, bool placing);
    // The map's right button this frame (process_right_button): the edit-room
    // modes, picking up or cancelling. Returns the command, EV_NONE for none.
    Event interface_map_right(Game &game, const UiMouse &mouse, bool placing);
    // The right click on the emergency or epidemic watch: opens the next
    // such patient's window. EV_NONE when the pointer is not on the watch.
    Event interface_watch_right(Game &game, const UiMouse &mouse);
    // The held left button on the emergency watch: keeps centring on its patient.
    bool held_emergency_watch(Game &game, const UiMouse &mouse);
    // Centres the view on the watch's picked patient `id` (none: nothing),
    // with a click sound when `click`.
    void centre_on_watch_patient(const Game &game, uint16_t id, bool click);
    // Centres the view on thing `id` unless it is beyond the scroll limit.
    void centre_view_on(const World &world, uint16_t id);
    void interface_map_hover(Game &game, const UiMouse &mouse, bool on_map, bool placing);
    // process_no_button's MM_NORMAL case: the pointer on a room of the player's
    // or on its reception (desk or receptionist) flags the queue.
    void interface_flag_queue(Game &game, const UiMouse &mouse);
    // The watch button's highlight sound (draw_emergency's othervar.watch_hovered):
    // played once on entering it, when the button is drawn (epidemic, or the
    // opening countdown past the tutorial flag).
    void interface_watch_hover(Game &game, const UiMouse &mouse);
    // The watch's buttons over the panel (process_left_button): the epidemic's
    // vaccination button and the opening countdown's open-now button. True
    // when one took the left click.
    bool interface_watch_buttons(Game &game, const UiMouse &mouse);
    // The pointer and left click of interface_map_hover for what it is over.
    void interface_map_hover_click(Game &game, const UiMouse &mouse, const MapHover &hover);
    // interface_map_hover's near rat: the near pointer and a miss on a click;
    // true if it handled it.
    bool interface_near_rat(Game &game, const UiMouse &mouse, const MapHover &hover);
    // After interface_map_hover while the queue window is open: its panel
    // information, the drag pointer and the door hotspot's scroll (req7_overdraw).
    void queue_window_after_frame(Game &game);
    void interface_advisor(Game &game, const UiMouse &mouse, bool input, bool paused);
    // The panel's icons for this frame, and where its top is.
    PanelAction panel_step(const Game *game, const UiMouse &mouse, bool paused, bool input);
    int panel_top_of(const Game &game) const;
    void interface_command(Game &game, const InfoWindowResult &info);
    // The rat bonus level has ended: on to the level it interrupted.
    void finish_rat_level();
    int level_after_win(const Game &game, int next);
    void interface_draw(Game *game, int ox, int oy, int scale);
    void draw_panel(const Game *game, const PanelFigures &figures);
    void flash_tutorial(const Game *game);
    // The help tooltip of the screen open this frame (src/helpmsg.cpp): the
    // group depends on the front requester.
    void interface_help();
    // The text font (cache slots 0 and 0x2d), the requester frame art and the
    // blend table the help system draws with.
    void load_help_art(const std::filesystem::path &dir);
    // compute_help_text for help group `group` at (x, y), with a context of
    // the in-game interfaces.
    void help_screen(int group, int x, int y);
    // Fills help_ctx_ for the current frame (the game, the data and the mouse
    // the last interface frame saw) and sets the help clock.
    void help_context();
    // Points the full-screen screens at help_ and help_ctx_ (once, after the
    // data and help art are loaded).
    void wire_screen_help();
    // build.cpp's show_help_page(0, type): a room type's page the first time
    // one is built.
    void interface_room_help(Game &game);
    // Draws the prepared help box onto the interface layer.
    void help_draw();
    // Whether a framed help page is open.
    bool help_page_open() const { return help_.box().style == 2 && !help_.box().msg.empty(); }
    void draw_info_view(Game &game);
    void draw_map_view(Surface &dst, const Game &game, Rect box, Point camera);
    void ask_quit();
    void quit_game();
    void ask_restart();
    void draw_pointer(int scale, uint8_t sprite = kPointerDefault);
    void startup_step(int out_w, int out_h);
    // Carries out what the start-up screens chose (startup_step's result).
    void startup_result(const StartupResult &r);
    // The debug report tool's save: the window as drawn so far, the state
    // (debug_state) and the note, in a new folder under debug-reports/.
    void save_debug_report();
    // The F10 quick report: pause at the press, capture the frame and state
    // into memory, and either write at once (Shift+F10) or show the note box.
    void quick_report_key(SDL_Keymod mods);
    void quick_report_step();
    // The note box and the save confirmation of the F10 quick report.
    void draw_quick_report();
    void capture_debug_report(DebugCapture &capture);
    // Writes a held capture to a new debug-reports/<time>/ folder.
    DebugReportResult write_debug_report(const std::string &note, DebugCapture &capture);
    void finish_quick_report(const std::string &note);
    void discard_quick_report();
    void restore_quick_pause();
    // Where a report goes: TH_DEBUG_REPORT_DIR when set (the drive tests),
    // else debug-reports/ under the folder the game was started from when the
    // data came from the development folder, else the user's reports folder.
    std::filesystem::path report_base() const;
    std::string debug_state() const;
    // The report's system.txt: build, machine, display, renderer, settings.
    std::string system_text() const;
    // What fills the window this frame, for debug_state.
    std::string screen_name() const;
    // debug_state's hospital lines: the level, saves, camera, panel, date,
    // money, the last command and the sounds played.
    std::string debug_state_hospital(const Game &g) const;
    std::string debug_state_startup() const;
    // debug_state's fax icons and open fax.
    std::string debug_state_calls(const Game &g) const;
    void open_town_map();
    void town_step(int out_w, int out_h);
    void charts_step(int out_w, int out_h);
    void fax_step(int out_w, int out_h);
    // The year-end screens (the hall of fame, the league tables and awards)
    // and the win/lose sequence.
    void check_year_end();
    void open_year_end();
    void year_end_step(int out_w, int out_h);
    void open_winlose();
    bool end_game();
    int lose_line_index(int anim);
    void winlose_step(int out_w, int out_h);
    void start_next_level(int next);
    // Saves the Continue slot and unloads the game for the main menu, keeping
    // its score for the next level.
    void return_to_main_menu();
    // Plays the win/lose movie (index -1 when none): its frames and audio, as
    // startup_step plays a start-up movie.
    void open_winlose_movie(int index);
    void advance_winlose_movie();
    // Puts the movie's current frame and palette on winlose_movie_surface_;
    // a losing movie's line is drawn over the frames from its text frame on
    // (lose_call).
    void show_winlose_movie_frame();
    void load_ui_art(const std::filesystem::path &dir);
    // The sounds and announcements of what the game did since the last
    // frame: new faxes, the emergency's helicopter sent, its clock.
    void game_sounds(Game &game);
    // Carries out the frame's command (Game::event): the interface's part,
    // then the game's (process_turn).
    void run_command(Game &game);
    void screen_event(const Event &event);
    // The yes/no requester of an edited room's cancel (the EV_OPEN_YES_NO
    // object_escape sets) and its yes answer, EV_DELETE_ROOM.
    void edited_room_question(Game &game);
    // The mouse in `src` drawn at `at` (from fit_rect).
    UiMouse fitted_mouse(const SDL_FRect &at, const Surface &src) const;
    // Draws `src` in `palette` into `dst` of the window through `tex`, or
    // through the upscaler when it is on and `upscalable` (a picture, not a
    // movie's frame).
    void draw_fitted(const Surface &src, const Palette &palette, SDL_Texture *&tex, const SDL_FRect &dst,
                     bool upscalable = true);
    // Draws the `w`-wide ARGB layer `pixels` over `dst`: scaled by the
    // upscaler when it is on, else through `plain` (a texture of its size).
    void draw_layer(ScaledTexture &scaled, SDL_Texture *plain, const std::vector<uint32_t> &pixels, int w,
                    const SDL_FRect &dst);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *screen_ = nullptr;
    int screen_w_ = 0, screen_h_ = 0;
    // The panel and the pointer, drawn at the UI scale over the view.
    // The game's interface (panel and requesters), drawn in its 640 x 480
    // canvas.
    UiLayer ui_;
    SDL_Texture *ui_tex_ = nullptr;
    // The upscaling of this run (--upscale, else settings.ini), and the 1x
    // layers and full-screen pictures shown through it.
    UpscaleConfig upscale_;
    // --upscale, as given; empty when the setting decides.
    std::string upscale_arg_;
    // The setup screen's upscaling options, preselected from the above on
    // its first frame.
    UpscaleChoice upscale_choice_;
    bool upscale_choice_ready_ = false;
    ScaledTexture ui_scaled_;
    ScaledTexture menu_scaled_;
    ScaledTexture fit_scaled_;
    PanelState panel_state_;
    Interface interface_;
    HospScores hosp_scores_;
    RequesterArt requester_art_;
    // Furnishing and hiring (milestone 4 steps 2 and 3).
    // Sound effects: the bank, the game's rules for playing them and the
    // device they are mixed to.
    SoundBank bank_;
    SoundPlayer sounds_;
    MusicPlayer music_; // the songs of SOUND/MIDI, played on an emulated OPL3
    Mixer mixer_;
    std::vector<FrameSound> frame_sounds_;
    ObjectUi object_ui_;
    HireStaff hire_;
    HireStaffArt hire_art_;
    StaffDemand demand_;
    // The panel information bar's kind and the thing under the pointer
    // (othervar.panel_info and panel_thing).
    uint8_t panel_info_ = 0;
    uint16_t panel_thing_ = 0;
    // flag_all_queue_members' last_flag_turn and last_flag_room: the frame and
    // room (World::rooms index) it last ran for, which decide whether a
    // newly highlighted queue plays its sound.
    uint64_t last_flag_frame_ = 0;
    uint16_t last_flag_room_ = 0;
    // The bar's kind as the last frame drew it, 0 when the raised panel
    // covered it (draw_panel_info's return).
    uint8_t panel_info_drawn_ = 0;
    // othervar.panel_info_timer: frames the bar stays once the pointer leaves.
    int panel_info_timer_ = 0;
    // othervar.watch_hovered: the pointer is over the watch button, so the
    // highlight sound plays once on entering it.
    bool watch_hovered_ = false;
    // The left button is held on the epidemic watch's button: its pressed
    // sprite is drawn (process_mleft_button).
    bool watch_pressed_ = false;
    // othervar.panel_message and panel_message_time: the special message on
    // the panel's information bar (display_panel_special_info).
    std::string panel_message_;
    int panel_message_time_ = 0;
    BuildUi build_ui_;
    Jukebox jukebox_;
    // The help system (tooltips and room help pages), its text font (cache
    // slot 0) and the mouse the last interface frame saw.
    HelpSystem help_;
    Font help_font_;
    Font help_menu_font_;
    UiMouse help_mouse_;
    // The help context the full-screen screens share, filled by help_context.
    HelpContext help_ctx_;
    // The player's details (myplayer): the room types whose help page was
    // shown, the advisor's one-off advice, the audio, speed and autosave
    // options. save/<name>.hdt holds them between runs.
    PlayerDetails details_;
    // check_autosave's last_saved (month + day of the last autosave) and
    // need_a_save (saving was not possible on day 28).
    uint8_t last_autosave_ = 0;
    bool need_autosave_ = false;
    // The last room built, as seen by interface_room_help.
    uint8_t last_built_room_ = 0;
    // The town map: while open it takes the window from the view, the
    // interface and the menu bar; the game keeps running behind it.
    TownMap town_;
    SDL_Texture *town_tex_ = nullptr;
    // The full-screen charts (F1-F8, the Charts menu, the panel), shown in
    // place of everything else while open, the game running behind them.
    Charts charts_;
    SDL_Texture *charts_tex_ = nullptr;
    // The fax (the choice screen), shown in place of everything else while
    // open, the game running behind it; and the fax icons above the panel.
    Fax fax_;
    SDL_Texture *fax_tex_ = nullptr;
    SDL_Texture *year_end_tex_ = nullptr;
    SDL_Texture *winlose_tex_ = nullptr;
    PopupIcons popups_;
    // The year-end screens (the hall of fame, the league tables and awards)
    // and the win/lose sequence: shown in place of everything else, the game
    // paused behind them.
    YearEndScreen year_end_;
    WinLoseScreen winlose_;
    // The emergency's helicopter at the last frame, for its announcement.
    uint16_t heli_seen_ = 0;
    // The thing under the pointer as the view was drawn this frame.
    PointedThing pointed_;
    // othervar.near_rat: the pointer is in a rat's near box but not its hit
    // box. Reset before each draw, set by draw_world.
    bool near_rat_ = false;
    // The start-up screens (intro, menus, the level's area movie), shown
    // until they start a level; skipped with --level.
    Startup startup_;
    std::vector<StartupKey> startup_keys_; // keys pressed since the last frame
    // Wheel notches since the last start-up frame, positive away from the user.
    int startup_wheel_ = 0;
    SDL_Texture *startup_tex_ = nullptr;
    SDL_AudioStream *movie_stream_ = nullptr;
    // The win/lose movie: its frames (into winlose_movie_surface_) and audio,
    // played before the letter as startup_step plays a start-up movie.
    SmackerVideo winlose_movie_;
    int winlose_movie_index_ = -1;
    int winlose_movie_frame_ = 0; // frames shown so far (current_frame)
    unsigned winlose_movie_flags_ = 0;
    double winlose_movie_time_ = 0;
    Surface winlose_movie_surface_;
    Palette winlose_movie_palette_{};
    std::vector<int16_t> winlose_movie_audio_;
    std::string balance_id_{kDefaultBalanceId};
    Event last_event_; // the command carried out the frame before (the tutorial reads it)
    // The commands the yes/no requester sends for yes and for no when it
    // asks before one (othervar.yes_type and no_type); EV_NONE otherwise.
    Event yes_event_;
    Event no_event_;
    // The slot File > Save is overwriting, waiting for the yes/no answer
    // (pulldown_save's EV_CLOSE_SAVE_YES_NO); 0 for none.
    int pending_save_slot_ = 0;
    // The slot Alt+S and Alt+L use (debug.save_slot).
    int debug_save_slot_ = 0;
    // The pointers of kPointersDrawn, by number.
    struct PointerTexture {
        SDL_Texture *texture = nullptr;
        int w = 0, h = 0;
    };
    std::array<PointerTexture, kPointerQueue + 1> pointers_{};
    // The system crosshair, shown over the game while the inspector picks.
    SDL_Cursor *crosshair_ = nullptr;
    bool crosshair_shown_ = false;
    // The patient, staff and handyman windows, and the view of their person
    // drawn into them (its surface and draw_world's scratch).
    InfoWindows info_;
    Surface info_view_;
    Surface info_scratch_;
    // The queue window (src/req7.cpp).
    QueueWindow queue_;

    Settings settings_;
    UserPaths user_paths_;
    // --debug (implied by --drive and --control): the F12 developer tools.
    bool debug_ = false;
    // --data, and whether the development folder is skipped: with --user-data
    // or --install-from, which make a run look as a normal start.
    std::optional<std::filesystem::path> data_arg_;
    bool user_data_ = false;
    DataSearch search_;
    std::string data_error_;
    // The setup screen: the install or verification on its worker thread, the
    // last verification's result, the state of the game folder, and a path
    // to drop on the window at start (--drop).
    InstallJob job_;
    std::string setup_notice_;
    bool game_dir_exists_ = false;
    bool game_dir_incomplete_ = false;
    std::string drop_arg_;
    bool install_failed_ = false;
    std::filesystem::path hiscore_path_;
    std::filesystem::path data_dir_;
    // The folder the saved games live in, and which File > Save/Load slots
    // hold one ([0] is slot 1).
    std::filesystem::path save_dir_;
    std::array<bool, kMenuSlots + 1> save_slots_{};
    FolderPick pick_;
    FolderPick installer_pick_;
    std::unique_ptr<GameData> data_;

    std::optional<int> language_; // --language: LANG-n.DAT instead of HOSPITAL.CFG's
    std::unique_ptr<LevelView> map_;

    Surface surface_;
    Point camera_;                                         // map pixel position of the view's top-left
    uint16_t scroll_min_ = 0, scroll_max_ = kMapCells - 1; // the level's scroll area, as cells
    // The scroll keys held (move_windows' scroll.*), and the view's velocity
    // in 1/65536 pixel a frame with the fraction carried between frames.
    uint8_t scroll_held_ = 0;
    // W and Tab held (othervar.turbo_key, lbKeyOn[KC_TAB]) and the modifiers of
    // the latest key event.
    bool turbo_held_ = false;
    bool tab_held_ = false;
    uint16_t key_mod_ = 0;
    // The numpad 8/2/4/6 keys held and pressed since the casebook or staff
    // list last read them, as ChartKeys::kPad* bits.
    uint8_t pad_held_ = 0, pad_pressed_ = 0;
    // key_mappos: the cell (column + row * 128) Alt+F1-F8 stored for each of
    // the 12 bookmark keys, 0 for none; cleared when a level starts.
    std::array<uint16_t, 12> key_mappos_{};
    int32_t scroll_vel_x_ = 0, scroll_vel_y_ = 0;
    int32_t scroll_frac_x_ = 0, scroll_frac_y_ = 0;
    double scroll_frame_time_ = 0;      // real time accrued towards the next move_windows frame
    float scroll_x_ = 0, scroll_y_ = 0; // the mouse drag's sub-pixel pixels, carried between frames
    double turn_clock_ = 0;             // turns due but not yet run
    ToolsState tools_;
    QuickReport quick_;
    // The game's menu bar, drawn into its own layer across the top of the
    // window at the UI scale.
    MenuBar menu_;
    UiLayer menu_layer_;
    SDL_Texture *menu_tex_ = nullptr;

    int level_ = 1;
    int year_end_phase_ = 0; // 0 the hall, 1 the charts
    int last_year_ = 0;      // the game's year the last frame saw
    int winlose_pick_ = 0;   // the letter variant drawn
    // The map cell under the pointer this frame (column + row * 128), -1
    // when off the map.
    int map_cell_ = -1;
    int movie_playing_ = -1;  // the movie movie_stream_ plays
    float frame_seconds_ = 0; // the last frame's length
    int zoom_ = 0;            // the view's scale: 0 automatic, else the integer scale
    Speed speed_ = Speed::Normal;
    // The pointer shown over the hospital (othervar.pointer_sprite).
    uint8_t pointer_sprite_ = kPointerDefault;
    // The epidemic's vaccination tool chosen (mouse mode MM_VACCINATE): a
    // click on the watch's vaccination button toggles it, a click on a
    // highlighted patient queues or unqueues them.
    bool vaccinate_ = false;
    bool open_town_ = false; // the T key asked for the town map this frame
    bool year_end_showing_ = false;
    bool winlose_showing_ = false;
    bool restart_level_ = false;
    bool quit_now_ = false; // Shift+Ctrl+Escape: EV_QUIT_GAME without the question
    // The score the last level ended with, for the next level (init_player).
    int32_t carried_score_ = 0;
    // The rivals' monthly banks, which no level clears (comp_buckets).
    RivalBuckets carried_buckets_{};
    bool have_carried_buckets_ = false;
    // debug.force_bonus_level, which no level clears.
    bool carried_force_bonus_ = false;
    void remember_level_end();
    bool winlose_to_menu_ = false; // the losing letter asked for the main menu
    bool chart_space_ = false;     // Space pressed for the charts this frame
    bool any_key_ = false;         // a key pressed for the win/lose screens (lbInkey)
    bool in_startup_ = false;      // the start-up screens are shown
    bool level_from_cli_ = false;
    // The level to go back to when a bonus level ends (othervar.level_after_bonus,
    // which a new level keeps).
    int level_after_bonus_ = 0;
    bool tutorial_ = false;    // the advisor's tutorial, chosen in the start-up screens
    bool escape_ = false;      // Escape pressed this frame
    bool asking_quit_ = false; // the yes/no requester asks whether to quit
    bool dragging_ = false;
    bool shadows_ = true; // the game's shadows option
    // The mouse buttons as the game's interface sees them; the pressed and
    // released flags last one frame.
    struct {
        bool left = false, right = false;
        bool left_pressed = false, right_pressed = false;
        bool left_released = false, right_released = false;
    } buttons_;
    bool running_ = true;
    bool imgui_ready_ = false;

    // Driven runs (drive.h): the script or control input, its options, the
    // driven clock and the screenshot asked for this frame.
    std::unique_ptr<Driver> driver_;
    std::filesystem::path drive_script_;
    std::filesystem::path drive_out_ = "drive-out";
    bool drive_control_ = false;
    bool drive_visible_ = false;
    // --default-speed: a driven run starts at the player's default speed
    // (Max) like a normal start, not at Normal.
    bool drive_default_speed_ = false;
    // --real-time: a driven run takes its frame length and clock from the
    // real time, as a normal start does, not a fixed 1/60 s.
    bool drive_real_time_ = false;
    // --fades: a driven run fades between screens as a normal start does; by
    // default it changes them at once, so that scripts need not wait out the
    // fades (the game stands still while one runs).
    bool drive_fades_ = false;
    int drive_w_ = 1280, drive_h_ = 720;
    uint64_t drive_ns_ = 0;
    uint64_t last_ns_ = 0;
    std::filesystem::path drive_shot_;
    std::filesystem::path drive_shot_ui_;
    // The text of the `type` commands waiting to be polled: SDL keeps the
    // pointer, so the strings outlive SDL_PushEvent until the next poll.
    std::deque<std::string> drive_texts_;
    // The full-screen screen drawn last (draw_fitted): where, its size and
    // the frame, for driven clicks in its 640 x 480 coordinates.
    struct {
        SDL_FRect at{};
        int w = 0, h = 0;
        uint64_t frame = 0;
    } fit_;
    uint64_t frame_ = 0;
    // The fades between screens and the loading screen, with the loading
    // picture's texture; whether the program's start has shown it yet.
    ScreenFader fader_;
    LoadingScreen loading_;
    SDL_Texture *loading_tex_ = nullptr;
    bool boot_faded_ = false;
    // Events (keys, buttons, the wheel) that came while a fade held the game,
    // delivered when it ends, as the keyboard buffer and mouse driver keep
    // them for the game.
    std::vector<SDL_Event> held_events_;
    // Alt+C asked for a screen shot of this frame; the number of the last one
    // taken (debug.screenshot_number, -1 before the first: the folder is
    // scanned for the first free name) and how many were taken.
    bool screen_shot_requested_ = false;
    int screen_shot_number_ = -1;
    int screen_shots_ = 0;
};

} // namespace th
