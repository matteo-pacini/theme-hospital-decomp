#pragma once

#include <cstdint>

namespace th
{

// What choosing a pull-down menu item asks for. The menu bar performs none
// of these; the caller does (see MenuEvent::value for each).
enum class MenuAction : uint8_t {
    None,
    // File > Load > Game n: pulldown_load, EV_LOAD_GAME. value: slot 1-8.
    Load,
    // File > Save > Game n: pulldown_save; asks before overwriting a used
    // slot (yesno_text[3]), then EV_SAVE_GAME. value: slot 1-8.
    Save,
    // File > Restart: ask_restart_level, a yes/no requester (yesno_text[6])
    // then EV_RESTART_LEVEL.
    Restart,
    // File > Quit: the key Shift+Escape, which asks (yesno_text[0]) and then
    // queues EV_QUIT_GAME.
    Quit,
    // Options > Sound: pulldown_toggle_sound, myplayer.sfx_on (stops all
    // samples when off; stays off without a sound driver).
    ToggleSound,
    // Options > Announcements: pulldown_toggle_ambient, myplayer.ambients_on.
    ToggleAnnouncements,
    // Options > Music: pulldown_toggle_music, myplayer.music_on (starts or
    // stops the MIDI song; stays off without a music driver).
    ToggleMusic,
    // Options > Sound volume > n%: pulldown_svol. value: item 0 (100%) to
    // 9 (10%); myplayer.sfx_volume = (10 - value) * 127 / 10.
    SoundVolume,
    // Options > Announcements volume: pulldown_sannounce, as SoundVolume for
    // myplayer.ambient_volume.
    AnnouncementVolume,
    // Options > Music volume: pulldown_smusic, as SoundVolume for
    // myplayer.music_volume, fading the music to it over 1000 ms.
    MusicVolume,
    // Options > Autosave: no callback; the item toggles myplayer.autosave.
    ToggleAutosave,
    // Options > Game speed > ...: pulldown_vary_speed, EV_VARY_SPEED.
    // value: vary_speed's argument, 10 slowest, 20 slower, 30 normal,
    // 40 max speed, 50 "and then some more" (myplayer.speed_choice is
    // value / 10 - 1).
    Speed,
    // Options > Jukebox: the key Alt+J, which opens the jukebox
    // (EV_OPEN_JUKEBOX) in high resolution when not paused.
    Jukebox,
    // Display > High res / MCGA lo-res: pulldown_toggle_screen_mode. value:
    // 0 for 640x480, 1 for 320x200; nothing if already in that mode or the
    // edit-room or buy-objects requester is open.
    ScreenMode,
    // Display > Shadows: pulldown_toggle_shadows, myplayer.shadows_on.
    ToggleShadows,
    // Charts > ...: the keys F1-F8, EV_OPEN_SCREEN with value 0 statement
    // (bank), 1 casebook (drugs), 2 policy, 3 research (refused without a
    // research room), 4 graphs, 5 staff listing, 6 bank manager, 7 status
    // (overview).
    Screen,
    // Charts > Briefing: pulldown_briefing, show_help_page(1, level); refused
    // in mouse mode MM_EDIT_ROOM_WAIT.
    Briefing,
};

// The setting an item's check mark shows (its check or check2 byte).
enum class MenuTick : uint8_t {
    None,
    Sound,         // myplayer.sfx_on
    Announcements, // myplayer.ambients_on
    Music,         // myplayer.music_on
    Autosave,      // myplayer.autosave
    SaveSlot,      // othervar.save_slot_used[item + 1]
    HighRes,       // othervar.hires_checked
    LowRes,        // othervar.lores_checked
    Shadows,       // myplayer.shadows_on
    SoundVolume,   // othervar.sfx_volume_checked[item]
    AnnouncementVolume,
    MusicVolume,
    Speed, // othervar.speed_checked[item]
};

// One pull-down item, from the game's PulldownItem (include/hosp.h).
struct MenuItemDef {
    // Works while the game is paused / on the rat level; otherwise choosing
    // it plays SFX_WRONG2.
    bool usable_paused;
    bool usable_rat_level;
    // pulldown_text index.
    uint8_t text;
    MenuTick tick;
    // Opens a submenu (numbered by setup_menus in item order).
    bool submenu;
    MenuAction action;
    // MenuEvent::value: the item's arg (Speed) or a fixed value; Load, Save
    // and the volumes take theirs from the item's position instead.
    uint8_t value;
    // flags bit 0: disabled (greyed) in a network game. Network games are not
    // supported, so it has no effect.
    bool net_disabled;
};

struct PulldownDef {
    // Shown on the bar (top-level menus only).
    bool active;
    // pulldown_text index of the title (top-level menus only).
    uint8_t text;
    // kMenuItems[first .. first + count).
    uint8_t first;
    uint8_t count;
};

// The game's pull-down menus (pmenu's initial data, src/array.cpp), less
// pulldowns[0], which nothing reads. kPulldownDefs[i] is the game's
// pulldowns[i + 1]: 0-4 are the bar's menus (Debug inactive), 5-10 the
// submenus in the order setup_menus numbers them.
inline constexpr MenuItemDef kMenuItems[] = {
    // 0: File
    {false, false, 7, MenuTick::None, true, MenuAction::None, 0, true},
    {false, false, 8, MenuTick::None, true, MenuAction::None, 0, true},
    {true, true, 9, MenuTick::None, false, MenuAction::Restart, 0, true},
    {true, true, 10, MenuTick::None, false, MenuAction::Quit, 0, false},
    // 4: Options
    {true, true, 12, MenuTick::Sound, false, MenuAction::ToggleSound, 0, false},
    {true, true, 13, MenuTick::Announcements, false, MenuAction::ToggleAnnouncements, 0, false},
    {true, true, 14, MenuTick::Music, false, MenuAction::ToggleMusic, 0, false},
    {true, true, 15, MenuTick::None, true, MenuAction::None, 0, false},
    {true, true, 16, MenuTick::None, true, MenuAction::None, 0, false},
    {true, true, 17, MenuTick::None, true, MenuAction::None, 0, false},
    {true, false, 18, MenuTick::Autosave, false, MenuAction::ToggleAutosave, 0, true},
    {true, true, 19, MenuTick::None, true, MenuAction::None, 0, false},
    {true, true, 20, MenuTick::None, false, MenuAction::Jukebox, 0, false},
    // 13: Display
    {true, false, 22, MenuTick::HighRes, false, MenuAction::ScreenMode, 0, true},
    {true, false, 23, MenuTick::LowRes, false, MenuAction::ScreenMode, 1, true},
    {true, true, 24, MenuTick::Shadows, false, MenuAction::ToggleShadows, 0, false},
    // 16: Charts
    {false, false, 26, MenuTick::None, false, MenuAction::Screen, 0, false},
    {false, false, 27, MenuTick::None, false, MenuAction::Screen, 1, false},
    {false, false, 28, MenuTick::None, false, MenuAction::Screen, 2, false},
    {false, false, 29, MenuTick::None, false, MenuAction::Screen, 3, false},
    {false, false, 30, MenuTick::None, false, MenuAction::Screen, 4, false},
    {false, false, 31, MenuTick::None, false, MenuAction::Screen, 5, false},
    {false, false, 32, MenuTick::None, false, MenuAction::Screen, 6, false},
    {false, false, 33, MenuTick::None, false, MenuAction::Screen, 7, false},
    {false, false, 34, MenuTick::None, false, MenuAction::Briefing, 0, false},
    // 25: Debug (never on the bar: its toggles of debug flags are not ported)
    {true, true, 36, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 37, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 38, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 39, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 40, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 41, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 42, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 43, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 44, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 45, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 46, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 47, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 48, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 49, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 50, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 51, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 52, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 53, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 54, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 55, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 56, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 57, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 58, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 59, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 60, MenuTick::None, false, MenuAction::None, 0, false},
    {true, true, 61, MenuTick::None, false, MenuAction::None, 0, false},
    // 51: File > Load
    {true, false, 63, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 64, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 65, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 66, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 67, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 68, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 69, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    {true, false, 70, MenuTick::SaveSlot, false, MenuAction::Load, 0, false},
    // 59: File > Save
    {true, false, 72, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 73, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 74, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 75, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 76, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 77, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 78, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    {true, false, 79, MenuTick::SaveSlot, false, MenuAction::Save, 0, false},
    // 67: Options > Sound volume
    {true, true, 81, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 82, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 83, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 84, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 85, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 86, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 87, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 88, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 89, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    {true, true, 90, MenuTick::SoundVolume, false, MenuAction::SoundVolume, 0, false},
    // 77: Options > Announcements volume
    {true, true, 92, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 93, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 94, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 95, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 96, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 97, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 98, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 99, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 100, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    {true, true, 101, MenuTick::AnnouncementVolume, false, MenuAction::AnnouncementVolume, 0, false},
    // 87: Options > Music volume
    {true, true, 103, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 104, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 105, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 106, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 107, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 108, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 109, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 110, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 111, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    {true, true, 112, MenuTick::MusicVolume, false, MenuAction::MusicVolume, 0, false},
    // 97: Options > Game speed
    {false, false, 114, MenuTick::Speed, false, MenuAction::Speed, 10, false},
    {false, false, 115, MenuTick::Speed, false, MenuAction::Speed, 20, false},
    {false, false, 116, MenuTick::Speed, false, MenuAction::Speed, 30, false},
    {false, false, 117, MenuTick::Speed, false, MenuAction::Speed, 40, false},
    {false, false, 118, MenuTick::Speed, false, MenuAction::Speed, 50, false},
};

inline constexpr PulldownDef kPulldownDefs[] = {
    {true, 1, 0, 4}, // File
    {true, 2, 4, 9}, // Options
    // Display: the game's High res and MCGA lo-res items (13, 14) are left
    // off; only Shadows is offered.
    {true, 3, 15, 1},
    {true, 4, 16, 9},   // Charts
    {false, 5, 25, 26}, // Debug
    {true, 0, 51, 8},   // File > Load
    {true, 0, 59, 8},   // File > Save
    {true, 0, 67, 10},  // Options > Sound volume
    {true, 0, 77, 10},  // Options > Announcements volume
    {true, 0, 87, 10},  // Options > Music volume
    {true, 0, 97, 5},   // Options > Game speed
};

} // namespace th
