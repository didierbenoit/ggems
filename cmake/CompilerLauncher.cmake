# ============================================================================
# ============================================================================

include_guard(GLOBAL)

option(GGEMS_USE_COMPILER_CACHE "Use sccache as the compiler launcher" OFF)

if(NOT GGEMS_USE_COMPILER_CACHE)
  message(STATUS "Compiler cache         : disabled")
  return()
endif()

find_program(GGEMS_SCCACHE_EXECUTABLE NAMES sccache)

if(NOT GGEMS_SCCACHE_EXECUTABLE)
  message(
    FATAL_ERROR
      "GGEMS_USE_COMPILER_CACHE is ON, but sccache was not found.\n"
      "Install sccache or configure with " "-DGGEMS_USE_COMPILER_CACHE=OFF.")
endif()

# C++ is GGEMS' main language.
set(CMAKE_CXX_COMPILER_LAUNCHER
    "${GGEMS_SCCACHE_EXECUTABLE}"
    CACHE FILEPATH "C++ compiler launcher" FORCE)

# Some dependencies, notably GoogleTest, may enable and compile C sources.
set(CMAKE_C_COMPILER_LAUNCHER
    "${GGEMS_SCCACHE_EXECUTABLE}"
    CACHE FILEPATH "C compiler launcher" FORCE)

message(STATUS "Compiler cache         : sccache")
message(STATUS "sccache executable     : ${GGEMS_SCCACHE_EXECUTABLE}")
message(STATUS "C++ compiler launcher  : ${CMAKE_CXX_COMPILER_LAUNCHER}")
message(STATUS "C compiler launcher    : ${CMAKE_C_COMPILER_LAUNCHER}")
