Theme Hospital Enhanced @VERSION@
================================

A new engine for Theme Hospital, for Windows, macOS and Linux. It runs the
whole single-player game from the data of your own copy, at any resolution
and aspect ratio. It is free software (GPL v3): the source is at
https://github.com/matteo-pacini/theme-hospital-enhanced/tree/v@VERSION@

NO GAME FILES ARE INCLUDED. You need your own copy of Theme Hospital: either
GOG's installer (setup_theme_hospital_*.exe) or a folder with the installed
game (the one holding DATA, LEVELS and QDATA).


First start
-----------

Start the program. When it finds no game data it shows a setup screen:

  * choose GOG's installer, or the game folder, with the buttons, or drop
    the file or folder on the window;
  * optionally choose to upscale the graphics (xBRZ, 2x to 6x);
  * wait for the install: it copies the game's files to your user folder
    (below) and checks them.

The files in the installer are verified by size and checksum. Nothing is
changed in the folder or installer you pick. After this the game starts
straight away.


Where things are kept
---------------------

Settings, saves, the installed game data and the upscaled graphics live in
your user folders, never next to the program:

  Linux    settings   ~/.config/ThemeHospitalEnhanced
           saves      ~/.local/share/ThemeHospitalEnhanced/save
           game data  ~/.local/share/ThemeHospitalEnhanced/game
  macOS    everything ~/Library/Application Support/ThemeHospitalEnhanced
  Windows  settings   %APPDATA%\ThemeHospitalEnhanced
           saves      %USERPROFILE%\Saved Games\ThemeHospitalEnhanced
           game data  %LOCALAPPDATA%\ThemeHospitalEnhanced\game

(On Linux $XDG_CONFIG_HOME and $XDG_DATA_HOME are honoured.) Delete the
game data folder to run the setup again.


Running it
----------

Linux     The program is an AppImage. Make it executable and run it:
              chmod +x theme-hospital-enhanced-*.AppImage
              ./theme-hospital-enhanced-*.AppImage
          It needs no installation and no libfuse2. Where FUSE is not
          available, run it with --appimage-extract-and-run as its first
          argument (it unpacks itself to a temporary folder).

macOS     Unzip, then move "Theme Hospital Enhanced.app" to Applications.
          The app is not notarized (it is only ad-hoc signed), so on the
          first start macOS refuses to open it. Open System Settings >
          Privacy & Security, scroll down and click "Open Anyway" next to
          the app's name, then confirm. Or, in a terminal:
              xattr -dr com.apple.quarantine "Theme Hospital Enhanced.app"
          Requires macOS 13.3 or later on an Apple silicon Mac.

Windows   Unzip, then run ThemeHospitalEnhanced.exe. The program is not
          signed, so SmartScreen may say "Windows protected your PC": click
          "More info", then "Run anyway". Before unzipping you can
          right-click the zip, choose Properties and tick "Unblock".


What is in this package
-----------------------

  README.txt                  this file
  LICENSE, COPYING            the project's licence (GNU GPL version 3)
  CHANGELOG.md                what changed in each version
  THIRD_PARTY_NOTICES.txt     the libraries it contains, with their licences

Theme Hospital is a trademark of Electronic Arts. This project is not
affiliated with or endorsed by EA.
