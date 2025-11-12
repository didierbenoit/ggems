# ============================================================================
#  @file      MSVCOptions.cmake
#  @brief     Compiler configuration for Microsoft Visual C++.
#  @details   Sets runtime, warnings, and optimisation options for GGEMS when
#             built under MSVC. Ensures full compatibility with Python and
#             pybind11 modules by enforcing a dynamic runtime (/MD or /MDd).
#             Designed for C++23 and OpenCL integration on Windows systems.
#  @author    Didier Benoit
#  @date      2025-11-05
# ============================================================================

include_guard(GLOBAL)

message(STATUS "Detected compiler: Microsoft Visual C++")

# ----------------------------------------------------------------------------
# Common compile options
# ----------------------------------------------------------------------------
add_compile_options(
  /std:c++latest
  /W4
  /EHsc
  /permissive-
  /nologo
  /utf-8
)

# Disable security warnings for C runtime functions
add_compile_definitions(
  _CRT_SECURE_NO_WARNINGS
  _SCL_SECURE_NO_WARNINGS
  NOMINMAX
)

# ----------------------------------------------------------------------------
# Debug configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring MSVC Debug build")
    add_compile_options(/Zi /Od)
    add_link_options(/DEBUG)
endif()

# ----------------------------------------------------------------------------
# Release configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring MSVC Release build")
    add_compile_options(/O2 /GL)
    add_link_options(/LTCG)
    add_compile_definitions(NDEBUG)
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------
message(STATUS "MSVC runtime library : ${CMAKE_MSVC_RUNTIME_LIBRARY}")
message(STATUS "Build type           : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard         : ${CMAKE_CXX_STANDARD}")
