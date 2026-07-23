# ============================================================================
# @file      CompilerOptions.cmake @brief     Dispatch GGEMS compiler-specific
# configuration. @details   Validate the operating system, compiler family, and
# compiler frontend before loading the corresponding option module.
# ============================================================================

include_guard(GLOBAL)

# ----------------------------------------------------------------------------
# Compiler identification
# ----------------------------------------------------------------------------

set(GGEMS_COMPILER_NAME "")
set(GGEMS_COMPILER_OPTIONS_MODULE "")

# ----------------------------------------------------------------------------
# Windows
# ----------------------------------------------------------------------------

if(WIN32)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    set(GGEMS_COMPILER_NAME "Microsoft Visual C++")
    set(GGEMS_COMPILER_OPTIONS_MODULE "MSVCOptions")

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")

      set(GGEMS_COMPILER_NAME "LLVM Clang with GNU-like frontend")

    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")

      set(GGEMS_COMPILER_NAME "LLVM Clang with MSVC-like frontend (clang-cl)")

    else()

      message(FATAL_ERROR "Unsupported Clang frontend on Windows: "
                          "'${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}'.")

    endif()

    set(GGEMS_COMPILER_OPTIONS_MODULE "ClangOptions")

  else()

    message(
      FATAL_ERROR
        "Unsupported compiler on Windows: '${CMAKE_CXX_COMPILER_ID}'.\n"
        "Supported Windows compilers:\n" "  - LLVM Clang\n" "  - clang-cl\n"
        "  - Microsoft Visual C++")

  endif()

  # ----------------------------------------------------------------------------
  # macOS
  # ----------------------------------------------------------------------------

elseif(APPLE)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")

    set(GGEMS_COMPILER_NAME "Apple Clang")
    set(GGEMS_COMPILER_OPTIONS_MODULE "ClangOptions")

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    set(GGEMS_COMPILER_NAME "LLVM Clang")
    set(GGEMS_COMPILER_OPTIONS_MODULE "ClangOptions")

  else()

    message(
      FATAL_ERROR
        "Unsupported compiler on macOS: '${CMAKE_CXX_COMPILER_ID}'.\n"
        "Supported macOS compilers:\n" "  - Apple Clang\n" "  - LLVM Clang")

  endif()

  # ----------------------------------------------------------------------------
  # Linux
  # ----------------------------------------------------------------------------

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    set(GGEMS_COMPILER_NAME "LLVM Clang")
    set(GGEMS_COMPILER_OPTIONS_MODULE "ClangOptions")

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    set(GGEMS_COMPILER_NAME "GNU C++")
    set(GGEMS_COMPILER_OPTIONS_MODULE "GNUOptions")

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")

    set(GGEMS_COMPILER_NAME "Intel oneAPI DPC++/C++")
    set(GGEMS_COMPILER_OPTIONS_MODULE "IntelOptions")

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")

    message(
      FATAL_ERROR "The Intel classic C++ compiler is not supported.\n"
                  "Use the Intel oneAPI LLVM-based compiler 'icpx' instead.")

  else()

    message(
      FATAL_ERROR
        "Unsupported compiler on Linux: '${CMAKE_CXX_COMPILER_ID}'.\n"
        "Supported Linux compilers:\n" "  - LLVM Clang\n" "  - GNU C++\n"
        "  - Intel oneAPI icpx")

  endif()

  # ----------------------------------------------------------------------------
  # Unsupported operating systems
  # ----------------------------------------------------------------------------

else()

  message(
    FATAL_ERROR
      "Unsupported operating system: '${CMAKE_SYSTEM_NAME}'.\n"
      "GGEMS currently supports Windows and Linux. "
      "macOS support is prepared but not yet validated.")

endif()

# ----------------------------------------------------------------------------
# Diagnostics
# ----------------------------------------------------------------------------

message(STATUS "Operating system      : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler              : ${GGEMS_COMPILER_NAME}")
message(STATUS "Compiler ID           : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler frontend     : ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
message(STATUS "Compiler options file : ${GGEMS_COMPILER_OPTIONS_MODULE}.cmake")

# ----------------------------------------------------------------------------
# Load compiler-specific options
# ----------------------------------------------------------------------------

include("${GGEMS_COMPILER_OPTIONS_MODULE}")

# ----------------------------------------------------------------------------
# Common Windows definitions
#
# These remain global temporarily. They will move into the target-based compiler
# policy during the next compiler-options cleanup.
# ----------------------------------------------------------------------------

if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()
