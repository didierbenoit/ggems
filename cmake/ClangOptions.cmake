# ============================================================================
#  @file      ClangOptions.cmake
#  @brief     Compiler configuration for Clang and clang-cl.
#  @details   Defines warnings, optimisation, and build-type specific flags
#             for both Unix-like (clang++) and Windows (clang-cl) environments.
#             Designed for C++23, pybind11 and OpenCL builds in GGEMS.
#  @author    Didier Benoit
#  @date      2025-11-05
# ============================================================================

include_guard(GLOBAL)

# ----------------------------------------------------------------------------
# Detect clang-cl (Windows) or clang++ (Unix/WSL)
# ----------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    message(STATUS "Detected compiler: clang-cl (Windows/MSVC emulation)")
    set(IS_CLANG_CL TRUE)
else()
    message(STATUS "Detected compiler: clang++ (Unix-like or WSL environment)")
    set(IS_CLANG_CL FALSE)
endif()

# ----------------------------------------------------------------------------
# Common compilation options
# ----------------------------------------------------------------------------
add_compile_options(
  -std=c++23 -Wall -Wextra -Wpedantic
  -Wshadow -Wnon-virtual-dtor
  -Wconversion -Wsign-conversion
)

# Colour diagnostics and visibility control
add_compile_options(
  -fdiagnostics-color=always
  -fcolor-diagnostics
  -fvisibility=hidden
  -fvisibility-inlines-hidden
)

# ----------------------------------------------------------------------------
# Debug configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Configuring Clang Debug build")

  add_compile_options(-O0 -g3 -Wfloat-conversion -fstandalone-debug)

  # Optional: detailed compilation timing
  add_compile_options(-ftime-trace)

  # Windows specific debugging flags
  if (WIN32)
    add_compile_options(-gcodeview)
  endif()
endif()

# ----------------------------------------------------------------------------
# Release configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Configuring Clang Release build")

  add_compile_options(
    -O3 -march=native
    -ffast-math
    -finline-functions
    -fstrict-aliasing
  )

  add_compile_definitions(NDEBUG)
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------
message(STATUS "Clang build type     : ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler path        : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard         : ${CMAKE_CXX_STANDARD}")
