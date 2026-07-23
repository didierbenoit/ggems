# ============================================================================
# @file      GNUOptions.cmake @brief     Compiler configuration for GNU C++.
# ============================================================================

include_guard(GLOBAL)

# ----------------------------------------------------------------------------
# User options
# ----------------------------------------------------------------------------

option(GGEMS_WARNINGS_AS_ERRORS "Treat GGEMS compiler warnings as errors" OFF)

# ----------------------------------------------------------------------------
# GNU C++ diagnostics
# ----------------------------------------------------------------------------

message(STATUS "Configuring GNU C++ compiler")

add_compile_options(
  "$<$<COMPILE_LANGUAGE:CXX>:-Wall>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wextra>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wshadow>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wconversion>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wsign-conversion>"
  "$<$<COMPILE_LANGUAGE:CXX>:-Wfloat-conversion>"
  "$<$<COMPILE_LANGUAGE:CXX>:-finput-charset=UTF-8>"
  "$<$<COMPILE_LANGUAGE:CXX>:-fexec-charset=UTF-8>"
  "$<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>"
  "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>"
  "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>"
  "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:-fno-omit-frame-pointer>")

if(GGEMS_WARNINGS_AS_ERRORS)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Werror>")
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------

message(STATUS "GNU compiler            : ${CMAKE_CXX_COMPILER}")
message(STATUS "GNU compiler version    : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Warnings as errors      : ${GGEMS_WARNINGS_AS_ERRORS}")
message(
  STATUS "Optimisation policy     : managed by CMake Debug/Release profiles")
