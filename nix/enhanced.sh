# Build enhanced/ (CMake, Release) and run it. Run from the repository root
# (`nix run path:./nix#enhanced`). Arguments are passed to the program, e.g.
# --level 5 or --data FOLDER. Without --data it reads the game data from
# orig/, the development folder it is built with.
#
# The build goes to build/enhanced-run, not build/enhanced: a build tree
# configured in the dev shell records the shell's include paths as the
# compiler's own and would not build here.

if [ ! -f enhanced/CMakeLists.txt ]; then
  echo "run from the repository root" >&2
  exit 1
fi
cmake -S enhanced -B build/enhanced-run -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/enhanced-run
exec build/enhanced-run/th_enhanced "$@"
