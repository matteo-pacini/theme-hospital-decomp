cmake_minimum_required(VERSION 3.24)
project(theme_hospital_enhanced LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Where a development build looks for the game data before anything else.
# Release builds leave it empty and rely on the runtime search (see
# src/app/datadir.cpp).
set(TH_DEV_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../orig" CACHE PATH
                    "Game data folder a development build reads first (empty to disable)")

# WebAssembly (`emcmake cmake`, in the `web` dev shell). Every object, the
# fetched dependencies included, is built with threads (th_enhanced installs
# on a std::thread), as wasm objects must all agree. th_tests and th_replay
# run under node on the host's files; the browser target takes its own link
# options.
if (EMSCRIPTEN)
  add_compile_options(-pthread)
  # Memory growth with threads is the design's choice (the data and the install
  # need more than a fixed size is kind to), not something to warn of.
  add_link_options(-pthread -Wno-pthreads-mem-growth)
  set(TH_NODE_LINK_OPTIONS -sSTACK_SIZE=8MB -sNODERAWFS=1 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1 -sENVIRONMENT=node)
endif()

include(cmake/deps.cmake)

if (MSVC)
  set(TH_WARNINGS /W4)
else()
  set(TH_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# Line coverage of th_core and th_replay (tools/coverage.sh): gcov data with
# GCC, LLVM source-based coverage with Clang.
option(TH_COVERAGE "Instrument th_core and th_replay for coverage" OFF)
if (TH_COVERAGE)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TH_COVERAGE_FLAGS --coverage)
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(TH_COVERAGE_FLAGS -fprofile-instr-generate -fcoverage-mapping)
  else()
    message(FATAL_ERROR "TH_COVERAGE needs GCC or Clang")
  endif()
endif()

# The engine: data loading and drawing, with no SDL dependency.
add_library(th_core STATIC
  src/core/audio/music.cpp
  src/core/audio/sounds.cpp
  src/core/data/anims.cpp
  src/core/data/balance.cpp
  src/core/data/fs.cpp
  src/core/data/rnc.cpp
  src/core/data/smacker.cpp
  src/core/data/sprites.cpp
  src/core/data/text.cpp
  src/core/data/palette.cpp
  src/core/data/level.cpp
  src/core/data/gamedata.cpp
  src/core/data/gog_install.cpp
  src/core/data/sha256.cpp
  src/core/render/surface.cpp
  src/core/render/text.cpp
  src/core/render/blit.cpp
  src/core/render/frame.cpp
  src/core/render/wibble.cpp
  src/core/render/iso.cpp
  src/core/render/panel.cpp
  src/core/render/shadows.cpp
  src/core/render/view.cpp
  src/core/ui/build_room.cpp
  src/core/ui/buy_objects.cpp
  src/core/ui/fax.cpp
  src/core/ui/charts.cpp
  src/core/ui/pie.cpp
  src/core/ui/helpmsg.cpp
  src/core/ui/helpmsg_callbacks.cpp
  src/core/ui/helpmsg_table.cpp
  src/core/ui/hire_staff.cpp
  src/core/ui/info_windows.cpp
  src/core/ui/jukebox.cpp
  src/core/ui/layer.cpp
  src/core/ui/menubar.cpp
  src/core/ui/panel_info.cpp
  src/core/ui/popups.cpp
  src/core/ui/queue_window.cpp
  src/core/ui/startup.cpp
  src/core/ui/staff_demand.cpp
  src/core/ui/town_map.cpp
  src/core/ui/windows.cpp
  src/core/ui/yearend.cpp
  src/core/ui/winlose.cpp
  src/core/world/advisor.cpp
  src/core/world/arrivals.cpp
  src/core/world/build.cpp
  src/core/world/complex.cpp
  src/core/world/computer.cpp
  src/core/world/computer_month.cpp
  src/core/world/computer_staff.cpp
  src/core/world/computer_peep.cpp
  src/core/world/computer_ai.cpp
  src/core/world/counts.cpp
  src/core/world/debug_dump.cpp
  src/core/world/disaster.cpp
  src/core/world/emergency.cpp
  src/core/world/epidemic.cpp
  src/core/world/fax_screen.cpp
  src/core/world/finance.cpp
  src/core/world/game.cpp
  src/core/world/icons.cpp
  src/core/world/litter.cpp
  src/core/world/mayor.cpp
  src/core/world/nav.cpp
  src/core/world/objects.cpp
  src/core/world/popups.cpp
  src/core/world/quake.cpp
  src/core/world/queue.cpp
  src/core/world/rats.cpp
  src/core/world/reaper.cpp
  src/core/world/reception.cpp
  src/core/world/reputation.cpp
  src/core/world/research.cpp
  src/core/world/rooms.cpp
  src/core/world/savegame.cpp
  src/core/world/snapshot.cpp
  src/core/world/staff.cpp
  src/core/world/staff_complex.cpp
  src/core/world/trace.cpp
  src/core/world/training.cpp
  src/core/world/vaccine.cpp
  src/core/world/world.cpp
  src/core/world/year_end.cpp)
# The snapshot build id (src/core/world/snapshot.h): a hash of the headers of
# what a snapshot holds and of its code, regenerated when they change.
set(TH_SNAPSHOT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/data/balance.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/data/level.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ui/charts.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ui/town_map.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/advisor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/arrivals.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/build.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/computer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/emergency.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/events.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/fax_screen.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/finance.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/game.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/object_tables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/objects.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/rats.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/research.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/snapshot.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/staff.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/trace.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/world.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/year_end.h
)
string(REPLACE ";" "|" TH_SNAPSHOT_SOURCE_LIST "${TH_SNAPSHOT_SOURCES}")
set(TH_SNAPSHOT_ID ${CMAKE_CURRENT_BINARY_DIR}/generated/snapshot_id.h)
add_custom_command(OUTPUT ${TH_SNAPSHOT_ID}
  COMMAND ${CMAKE_COMMAND} -DOUT=${TH_SNAPSHOT_ID} "-DSOURCES=${TH_SNAPSHOT_SOURCE_LIST}"
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/snapshot_id.cmake
  DEPENDS ${TH_SNAPSHOT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/snapshot_id.cmake
  VERBATIM)
target_sources(th_core PRIVATE ${TH_SNAPSHOT_ID})
target_include_directories(th_core PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_include_directories(th_core PUBLIC src)
target_compile_options(th_core PRIVATE ${TH_WARNINGS})
target_link_libraries(th_core PRIVATE th_ymfm th_miniz)
target_compile_options(th_core PRIVATE ${TH_COVERAGE_FLAGS})
target_link_options(th_core INTERFACE ${TH_COVERAGE_FLAGS})

add_executable(th_enhanced
  src/app/main.cpp
  src/app/audio/mixer.cpp
  src/app/datadir.cpp
  src/app/drive.cpp
  src/app/paths.cpp
  src/app/player_details.cpp
  src/app/install_job.cpp
  src/app/settings.cpp
  src/app/ui/nav_tool.cpp
  src/app/ui/tools.cpp)
target_link_libraries(th_enhanced PRIVATE th_core th_imgui SDL3::SDL3)
if (WIN32)
  target_link_libraries(th_enhanced PRIVATE ole32 shell32)
endif()
target_compile_options(th_enhanced PRIVATE ${TH_WARNINGS})
if (TH_DEV_DATA_DIR AND NOT EMSCRIPTEN)
  target_compile_definitions(th_enhanced PRIVATE TH_DEV_DATA_DIR="${TH_DEV_DATA_DIR}")
endif()

# The browser build: build/web/dist/index.html, index.js and index.wasm, with
# the shell page and the JS bridge of web/ (storage, picker, reports). The
# thread pool holds the install thread and the audio threads SDL may start.
if (EMSCRIPTEN)
  set(TH_WEB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/web)
  target_sources(th_enhanced PRIVATE src/app/web.cpp)
  set_target_properties(th_enhanced PROPERTIES
    OUTPUT_NAME index
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist
    LINK_DEPENDS "${TH_WEB_DIR}/shell.html;${TH_WEB_DIR}/bridge.js")
  target_link_options(th_enhanced PRIVATE
    --shell-file ${TH_WEB_DIR}/shell.html
    --pre-js ${TH_WEB_DIR}/bridge.js
    -lidbfs.js
    -sEXPORTED_RUNTIME_METHODS=FS,ccall
    -sALLOW_MEMORY_GROWTH=1
    -sPTHREAD_POOL_SIZE=4
    # The simulation overflows the default 64 KB stack (as th_replay does).
    -sSTACK_SIZE=8MB
    -sDEFAULT_PTHREAD_STACK_SIZE=2MB)
endif()

# The replay check: runs a level headless and writes its trace (no SDL).
add_executable(th_replay tools/replay/main.cpp)
target_link_libraries(th_replay PRIVATE th_core)
target_link_options(th_replay PRIVATE ${TH_NODE_LINK_OPTIONS})
target_compile_options(th_replay PRIVATE ${TH_WARNINGS})
target_compile_options(th_replay PRIVATE ${TH_COVERAGE_FLAGS})
if (TH_DEV_DATA_DIR)
  target_compile_definitions(th_replay PRIVATE TH_DEV_DATA_DIR="${TH_DEV_DATA_DIR}")
endif()

include(CTest)
if (BUILD_TESTING)
  add_executable(th_tests
    tests/main.cpp
    tests/paths.cpp
    src/app/paths.cpp
    tests/player_details.cpp
    src/app/player_details.cpp
    tests/common.cpp
    tests/audio.cpp
    tests/computer.cpp
    tests/computer_month.cpp
    tests/computer_staff.cpp
    tests/computer_peep.cpp
    tests/computer_ai.cpp
    tests/data.cpp
    tests/drive.cpp
    src/app/drive.cpp
    tests/epidemic.cpp
    tests/game.cpp
    tests/help.cpp
    tests/help_callbacks.cpp
    tests/help_screens.cpp
    tests/icons.cpp
    tests/install.cpp
    tests/nav.cpp
    tests/panel_info.cpp
    tests/patients.cpp
    tests/rats.cpp
    tests/render.cpp
    tests/replay.cpp
    tests/research.cpp
    tests/rooms.cpp
    tests/sim_yearend.cpp
    tests/staff.cpp
    tests/staff_demand.cpp
    tests/ui.cpp
    tests/yearend.cpp)
  target_link_libraries(th_tests PRIVATE th_core)
  target_link_options(th_tests PRIVATE ${TH_NODE_LINK_OPTIONS})
  if (WIN32)
    target_link_libraries(th_tests PRIVATE ole32 shell32)
  endif()
  target_compile_options(th_tests PRIVATE ${TH_WARNINGS})
  # Tests that need the game data are skipped when TH_TEST_DATA_DIR is unset
  # or missing.
  add_test(NAME th_tests COMMAND th_tests)
  set_tests_properties(th_tests PROPERTIES ENVIRONMENT "TH_TEST_DATA_DIR=${TH_DEV_DATA_DIR}" SKIP_RETURN_CODE 77)
endif()
