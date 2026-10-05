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

include(cmake/deps.cmake)

if(MSVC)
  set(TH_WARNINGS /W4)
else()
  set(TH_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# The engine: data loading and drawing, with no SDL dependency.
add_library(th_core STATIC
  src/core/data/fs.cpp
  src/core/data/rnc.cpp
  src/core/data/sprites.cpp
  src/core/data/palette.cpp
  src/core/data/level.cpp
  src/core/data/gamedata.cpp
  src/core/render/surface.cpp
  src/core/render/blit.cpp
  src/core/render/iso.cpp
  src/core/render/panel.cpp
)
target_include_directories(th_core PUBLIC src)
target_compile_options(th_core PRIVATE ${TH_WARNINGS})

add_executable(th_enhanced
  src/app/main.cpp
  src/app/datadir.cpp
  src/app/settings.cpp
  src/app/ui/tools.cpp
)
target_link_libraries(th_enhanced PRIVATE th_core th_imgui SDL3::SDL3)
target_compile_options(th_enhanced PRIVATE ${TH_WARNINGS})
if(TH_DEV_DATA_DIR)
  target_compile_definitions(th_enhanced PRIVATE TH_DEV_DATA_DIR="${TH_DEV_DATA_DIR}")
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(th_tests tests/tests.cpp)
  target_link_libraries(th_tests PRIVATE th_core)
  target_compile_options(th_tests PRIVATE ${TH_WARNINGS})
  # Tests that need the game data are skipped when TH_TEST_DATA_DIR is unset
  # or missing.
  add_test(NAME th_tests COMMAND th_tests)
  set_tests_properties(th_tests PROPERTIES
    ENVIRONMENT "TH_TEST_DATA_DIR=${TH_DEV_DATA_DIR}"
    SKIP_RETURN_CODE 77)
endif()
