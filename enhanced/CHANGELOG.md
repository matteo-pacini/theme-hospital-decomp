# Changelog

## v1.0.0

The first release: the whole single-player game, ported function by function
from the decompilation.

- The full single-player game: the 12 campaign levels and the two rat bonus
  levels, the win and lose letters, and the credits.
- Matches the original turn by turn: every random draw happens in the
  original's order, checked by replaying the same player actions through the
  instrumented original and through this port.
- Runs at any resolution and aspect ratio, from the game's own data. No game
  files are included.
- Installs the game data from the GOG installer or from a folder, on first
  start.
- xBRZ upscaling of the graphics, applied once at install.
- Start-up screens as the game has them: intro movie, title, main menu, load
  game, difficulty and tutorial, and the level's area movie.
- Palette fades between screens and the loading screen, as in the game.
- The pull-down menu bar, requesters and panel, with the original art.
- Sound effects, announcements and the music (XMI songs on an emulated OPL3),
  with the jukebox.
- Staff, patients, rooms, research, the town map, the charts and the year end,
  the rival hospitals and the emergencies, all as the original does them.
- Saves in the original's slots, autosave on day 28, the Continue slot, and the
  player's options in the original's file layout.
- The game's cheats, with the keypad code, and the briefing's level pick.
- PNG screen shots (Alt+C).
- Six languages: English, French, German, Italian, Spanish and Swedish.
- A browser build that runs in the page and installs from the files you give it.
- The original's bugs are kept, and listed in BUGS.md in the project's docs folder.
- Debug reports (F10, F12): a screenshot, the state, a snapshot and the
  commands since the level began.
