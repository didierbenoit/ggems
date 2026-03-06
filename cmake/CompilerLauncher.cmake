# ============================================================================
# @file CompilerLauncher.cmake
# @brief Configure optional compiler launchers for the GGEMS build.
# @details
#   This module configures a compiler launcher for C and C++ compilation.
#   It supports:
#     - an explicit user-provided launcher,
#     - automatic detection of sccache,
#     - fallback when no launcher is available.
# ============================================================================

# ----------------------------------------------------------------------------
# User options
# ----------------------------------------------------------------------------

option(GGEMS_USE_COMPILER_CACHE
  "Enable compiler cache launcher when available"
  OFF
)

set(GGEMS_COMPILER_LAUNCHER
  ""
  CACHE STRING "Explicit compiler launcher executable"
)

# ----------------------------------------------------------------------------
# Internal helper
# ----------------------------------------------------------------------------

set(GGEMS_COMPILER_LAUNCHER_PROGRAM "")

# ----------------------------------------------------------------------------
# Resolve compiler launcher
# ----------------------------------------------------------------------------

if(GGEMS_COMPILER_LAUNCHER)
  set(GGEMS_COMPILER_LAUNCHER_PROGRAM "${GGEMS_COMPILER_LAUNCHER}")
  message(STATUS "Using explicit compiler launcher: ${GGEMS_COMPILER_LAUNCHER_PROGRAM}")

elseif(GGEMS_USE_COMPILER_CACHE)
  find_program(GGEMS_SCCACHE_PROGRAM sccache)

  if(GGEMS_SCCACHE_PROGRAM)
    set(GGEMS_COMPILER_LAUNCHER_PROGRAM "${GGEMS_SCCACHE_PROGRAM}")
    message(STATUS "Using sccache compiler launcher: ${GGEMS_COMPILER_LAUNCHER_PROGRAM}")
  else()
    message(STATUS "No compiler cache launcher detected")
  endif()

else()
  message(STATUS "Compiler cache launcher disabled by user")
endif()

# ----------------------------------------------------------------------------
# Apply compiler launcher
# ----------------------------------------------------------------------------

if(GGEMS_COMPILER_LAUNCHER_PROGRAM)
  # Compiler launchers are best supported with Ninja and Makefile generators.
  if(CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
    set(CMAKE_C_COMPILER_LAUNCHER
      "${GGEMS_COMPILER_LAUNCHER_PROGRAM}"
      CACHE STRING "C compiler launcher"
      FORCE
    )

    set(CMAKE_CXX_COMPILER_LAUNCHER
      "${GGEMS_COMPILER_LAUNCHER_PROGRAM}"
      CACHE STRING "C++ compiler launcher"
      FORCE
    )

    message(STATUS "Compiler launcher enabled for C and C++ compilation")
  else()
    message(STATUS
      "Compiler launcher '${GGEMS_COMPILER_LAUNCHER_PROGRAM}' was found, "
      "but generator '${CMAKE_GENERATOR}' is not the recommended choice. "
      "Prefer Ninja or Makefiles for reliable launcher support."
    )
  endif()
endif()
