# ============================================================================
#  @file      CMakeLists.txt
#  @brief     Main build configuration for the GGEMS project.
#  @details   GGEMS (GPU Geant4-based Monte Carlo Simulations) is a scientific
#             simulation framework implemented in C++23, leveraging OpenCL 3.0
#             for heterogeneous computing and pybind11 for Python integration.
#             This configuration defines compiler dispatch, build-type
#             management, and optional subsystems such as documentation,
#             unit testing, and GUI integration (ImGui/Vulkan).
#
#             Requirements:
#               - CMake ≥ 3.27  (scikit-build-core and pybind11 CONFIG mode)
#               - OpenCL 3.0 SDK
#               - pybind11 ≥ 3.0
#               - Ninja (recommended for single-config builds)
#
#             Author:   Didier Benoit
#             Created:  2025-11-05
#             License:  GNU GPL v3.0 or later
# ============================================================================

# ----------------------------------------------------------------------------
# Minimum required CMake version
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# ----------------------------------------------------------------------------
# Project declaration
# ----------------------------------------------------------------------------
project(GGEMS
  VERSION 2.0.0
  LANGUAGES C CXX
  DESCRIPTION "OpenCL-based simulation engine with Python bindings"
)

# ----------------------------------------------------------------------------
# C++ language standard
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# Launcher configuration
# ----------------------------------------------------------------------------
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerLauncher)

# ----------------------------------------------------------------------------
# Display build context
# ----------------------------------------------------------------------------
message(STATUS "CMake version in use: ${CMAKE_VERSION}")
message(STATUS
  "Configuring project ${PROJECT_NAME}
  version ${PROJECT_VERSION}")

# ----------------------------------------------------------------------------
# Ensure position-independent code for shared libraries and Python modules
# ----------------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----------------------------------------------------------------------------
# Compiler configuration dispatch (Clang / MSVC)
# ----------------------------------------------------------------------------
include(CompilerOptions)

# ----------------------------------------------------------------------------
# Optional build features
# ----------------------------------------------------------------------------
option(GGEMS_BUILD_TESTS "Build GoogleTest unit tests" OFF)
option(GGEMS_BUILD_DOCS  "Build documentation with Doxygen and Sphinx" OFF)
option(GGEMS_WITH_IMGUI  "Enable ImGui / Vulkan interface" OFF)
option(GGEMS_BUILD_VALIDATION "Build GGEMS scientific validation tools" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(GGEMS_DEBUG_MODE)
else()
  add_compile_definitions(GGEMS_RELEASE_MODE)
endif()

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------
find_package(OpenCL REQUIRED)
set(OPENCL_HPP_INCLUDE_DIR
  "${PROJECT_SOURCE_DIR}/external/OpenCL-CLHPP/include")
message(STATUS "OpenCL library located at: ${OpenCL_LIBRARY}")
message(STATUS "OpenCL include path: ${OPENCL_HPP_INCLUDE_DIR}")

find_package(pybind11 REQUIRED CONFIG)

# ----------------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

# ----------------------------------------------------------------------------
# Core library
# ----------------------------------------------------------------------------
file(GLOB_RECURSE GGEMS_CORE_SOURCES
  ${PROJECT_SOURCE_DIR}/src/core/*.cc
  ${PROJECT_SOURCE_DIR}/src/frameworks/*.cc
  ${PROJECT_SOURCE_DIR}/src/render/*.cc
  ${PROJECT_SOURCE_DIR}/src/utf/*.cc
)

add_library(ggems_core STATIC ${GGEMS_CORE_SOURCES})

target_include_directories(ggems_core PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${OPENCL_HPP_INCLUDE_DIR})

target_link_libraries(ggems_core PUBLIC OpenCL::OpenCL)

target_compile_definitions(ggems_core
  PRIVATE
  GGEMS_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
)

if(UNIX AND NOT APPLE)
  target_link_libraries(ggems_core PRIVATE atomic)
endif()

# ----------------------------------------------------------------------------
# Optional ImGui / Vulkan interface
# ----------------------------------------------------------------------------
if (GGEMS_WITH_IMGUI)
  message(STATUS "GGEMS GuiMode enabled: Vulkan / Dear ImGui layer requested.")
  add_subdirectory(src/ui)
endif()

# ----------------------------------------------------------------------------
# Python module (pybind11)
# ----------------------------------------------------------------------------
add_subdirectory(python)

# ----------------------------------------------------------------------------
# Unit testing
# ----------------------------------------------------------------------------
if (GGEMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ----------------------------------------------------------------------------
# Scientific validation
# ----------------------------------------------------------------------------
if (GGEMS_BUILD_VALIDATION)
  add_subdirectory(validation)
endif()

# ----------------------------------------------------------------------------
# Documentation
# ----------------------------------------------------------------------------
if (GGEMS_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# ----------------------------------------------------------------------------
# Build summary
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "================= GGEMS Build Summary =================")
message(STATUS "Compiler ID            : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler path          : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard           : ${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests            : ${GGEMS_BUILD_TESTS}")
message(STATUS "Build documentation    : ${GGEMS_BUILD_DOCS}")
message(STATUS "ImGui UI enabled       : ${GGEMS_WITH_IMGUI}")
message(STATUS "Build validation       : ${GGEMS_BUILD_VALIDATION}")
message(STATUS "========================================================")
message(STATUS "")
