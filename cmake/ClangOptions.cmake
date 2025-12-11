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
# Detect clang frontend
# ----------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  message(STATUS "Detected LLVM Clang (pure GNU-style frontend)")
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  message(STATUS "Detected clang-cl (MSVC-like frontend)")
else()
  message(STATUS
    "Clang detected with unknown frontend variant:
    ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}"
  )
endif()

# ----------------------------------------------------------------------------
# Common compilation options
# ----------------------------------------------------------------------------
add_compile_options(
  -std=c++23 -Wall -Wextra -Wpedantic
  -Wshadow -Wnon-virtual-dtor -finput-charset=UTF-8 -fexec-charset=UTF-8
  -Wconversion -Wsign-conversion
)

# Colour diagnostics and visibility control
add_compile_options(
  -fdiagnostics-color=always
  -fcolor-diagnostics
  -fvisibility=hidden
  -fvisibility-inlines-hidden
)
add_compile_definitions(NOMINMAX)

# ----------------------------------------------------------------------------
# clang-cl specific behaviour (only if using MSVC frontend)
# ----------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
  message(STATUS "Configuring clang-cl behaviour")

  # Use MSVC-compatible flags (clang-cl mode)
  add_compile_options(
    -Wno-language-extension-token   # Avoid warnings on MSVC-specific pragmas
    -Wno-microsoft-enum-value       # MSVC ABI quirks
  )

  # MSVC-style definitions
  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    _SCL_SECURE_NO_WARNINGS
    NOMINMAX
  )

endif()

# ----------------------------------------------------------------------------
# Debug configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Configuring Clang Debug build")

  add_compile_options(-O0 -g3 -Wfloat-conversion -fno-omit-frame-pointer)

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

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  # Enable LINK-TIME optimisation if supported
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
  if (ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    message(STATUS "Clang IPO/LTO enabled")
  else()
    message(WARNING "Clang IPO/LTO not supported: ${ipo_error}")
  endif()
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------
message(STATUS "Clang build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler path    : ${CMAKE_CXX_COMPILER}")
