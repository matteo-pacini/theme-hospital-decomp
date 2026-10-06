# Writes OUT, a header defining TH_SNAPSHOT_SOURCE_HASH: a hash of SOURCES (a
# |-separated list) with their // comments and blank space taken out, so that
# a snapshot (src/core/world/snapshot.h) is refused by a build whose
# snapshotted structures or snapshot code differ, but not for an edited
# comment. OUT is only rewritten when the hash changes.
#
# cmake -DOUT=<header> -DSOURCES=<files> -P snapshot_id.cmake

string(REPLACE "|" ";" SOURCES "${SOURCES}")
set(text "")
foreach(source IN LISTS SOURCES)
  file(READ "${source}" content)
  string(REGEX REPLACE "//[^\n]*" "" content "${content}")
  string(REGEX REPLACE "[ \t\r\n]+" " " content "${content}")
  string(APPEND text "${content}")
endforeach()
string(SHA256 hash "${text}")
string(SUBSTRING "${hash}" 0 16 hash)
set(header "#pragma once\n#define TH_SNAPSHOT_SOURCE_HASH 0x${hash}ull\n")
if(EXISTS "${OUT}")
  file(READ "${OUT}" old)
endif()
if(NOT old STREQUAL header)
  file(WRITE "${OUT}" "${header}")
endif()
