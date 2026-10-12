include(FetchContent)

# SDL3: the system's when there is one (Nix, distro packages), otherwise
# built from source.
find_package(SDL3 CONFIG QUIET)
if(NOT SDL3_FOUND)
  message(STATUS "SDL3 not found, fetching it")
  FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.16
    GIT_SHALLOW TRUE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(SDL3)
endif()

# Dear ImGui has no CMake build of its own: compile the core and the SDL3
# platform and SDL_Renderer backends.
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.9b
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)

add_library(th_imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(th_imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(th_imgui PUBLIC SDL3::SDL3)

# ymfm (BSD-3-Clause): the YMF262 (OPL3) emulation the music plays on. No
# CMake build of its own; built with its own flags, outside the project's
# warnings.
FetchContent_Declare(ymfm
  URL https://github.com/aaronsgiles/ymfm/archive/81aec25ccbb98f4873a255f7551ac4dadac59b4a.tar.gz
  URL_HASH SHA256=9bcf001b30f53754c49cf2fb864792a64702b8aeebb105baa908ad5dd33f443f)
FetchContent_MakeAvailable(ymfm)

add_library(th_ymfm STATIC
  ${ymfm_SOURCE_DIR}/src/ymfm_adpcm.cpp
  ${ymfm_SOURCE_DIR}/src/ymfm_misc.cpp
  ${ymfm_SOURCE_DIR}/src/ymfm_opl.cpp
  ${ymfm_SOURCE_DIR}/src/ymfm_pcm.cpp
)
target_include_directories(th_ymfm SYSTEM PUBLIC ${ymfm_SOURCE_DIR}/src)

# miniz (MIT): its inflate (miniz_tinfl.c) reads GOG's installer and the
# upscaling cache, its deflate (miniz_tdef.c) writes that cache (miniz.c
# holds the checksums they use).
# Its own CMake build is not used (SOURCE_SUBDIR names no folder)
# and its export header, which that build generates, is written here empty.
# Built with its own flags, outside the project's warnings.
FetchContent_Declare(miniz
  URL https://github.com/richgel999/miniz/archive/refs/tags/3.0.2.tar.gz
  URL_HASH SHA256=c4b4c25a4eb81883448ff8924e6dba95c800094a198dc9ce66a292ac2ef8e018
  SOURCE_SUBDIR none)
FetchContent_MakeAvailable(miniz)
enable_language(C)

file(WRITE ${CMAKE_BINARY_DIR}/miniz_generated/miniz_export.h "#define MINIZ_EXPORT\n")
add_library(th_miniz STATIC ${miniz_SOURCE_DIR}/miniz.c ${miniz_SOURCE_DIR}/miniz_tinfl.c ${miniz_SOURCE_DIR}/miniz_tdef.c)
target_include_directories(th_miniz SYSTEM PUBLIC ${miniz_SOURCE_DIR} ${CMAKE_BINARY_DIR}/miniz_generated)

# xBRZ (GPL-3.0-only), the install-time upscaler, vendored in third_party/xbrz
# (README there: version, archive hash, the one change). Built with warnings
# off, outside the project's.
add_library(th_xbrz STATIC third_party/xbrz/xbrz.cpp)
target_include_directories(th_xbrz SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/xbrz)
if (NOT MSVC)
  target_compile_options(th_xbrz PRIVATE -w)
endif()
