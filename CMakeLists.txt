# ----------------------------------------------------------------------------
# Minimum required CMake version
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# ----------------------------------------------------------------------------
# MSVC-compatible debug information
#
# /Z7 stores debug information in object files. This works better with compiler
# caches such as sccache than a shared compiler PDB.
# ----------------------------------------------------------------------------
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:Embedded>")

# ----------------------------------------------------------------------------
# Project declaration
# ----------------------------------------------------------------------------
project(
  GGEMS
  VERSION 2.0.0
  LANGUAGES CXX
  DESCRIPTION "GPU Geant4-based Monte Carlo Simulations")

# ----------------------------------------------------------------------------
# General CMake configuration
# ----------------------------------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----------------------------------------------------------------------------
# C++ language standard
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# Supported build configurations
# ----------------------------------------------------------------------------
set(GGEMS_SUPPORTED_BUILD_TYPES Debug Release)

if(CMAKE_CONFIGURATION_TYPES)
  # Multi-configuration generators such as Visual Studio or Ninja Multi-Config
  set(CMAKE_CONFIGURATION_TYPES
      "${GGEMS_SUPPORTED_BUILD_TYPES}"
      CACHE STRING "GGEMS build configurations" FORCE)
else()
  # Single-configuration generators such as Ninja and Unix Makefiles
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        Debug
        CACHE STRING "GGEMS build configuration" FORCE)
  endif()

  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

  if(NOT CMAKE_BUILD_TYPE IN_LIST GGEMS_SUPPORTED_BUILD_TYPES)
    message(FATAL_ERROR "Unsupported GGEMS build type '${CMAKE_BUILD_TYPE}'."
                        "Expected Debug or Release.")
  endif()
endif()

# ----------------------------------------------------------------------------
# Launcher configuration
# ----------------------------------------------------------------------------
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerLauncher)

# ----------------------------------------------------------------------------
# Display build context
# ----------------------------------------------------------------------------
message(STATUS "CMake version in use: ${CMAKE_VERSION}")
message(STATUS "Configuring project ${PROJECT_NAME} version ${PROJECT_VERSION}")

# ----------------------------------------------------------------------------
# Compiler configuration dispatch (Clang / MSVC)
# ----------------------------------------------------------------------------
include(CompilerOptions)
include(OptimizationOptions)

# ----------------------------------------------------------------------------
# Optional build features
# ----------------------------------------------------------------------------
option(GGEMS_BUILD_TESTS "Build GoogleTest unit tests" OFF)
option(GGEMS_BUILD_DOCS "Build documentation with Doxygen and Sphinx" OFF)
option(GGEMS_WITH_IMGUI "Enable ImGui / Vulkan interface" OFF)
option(GGEMS_BUILD_VALIDATION "Build GGEMS scientific validation tools" OFF)

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
# Core library
# ----------------------------------------------------------------------------
file(
  GLOB_RECURSE
  GGEMS_CORE_SOURCES
  CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/core/*.cc"
  "${PROJECT_SOURCE_DIR}/src/frameworks/*.cc"
  "${PROJECT_SOURCE_DIR}/src/render/*.cc"
  "${PROJECT_SOURCE_DIR}/src/utf/*.cc")

list(SORT GGEMS_CORE_SOURCES)

add_library(ggems_core STATIC ${GGEMS_CORE_SOURCES})
add_library(GGEMS::Core ALIAS ggems_core)

target_compile_features(ggems_core PUBLIC cxx_std_23)

target_include_directories(ggems_core PUBLIC ${PROJECT_SOURCE_DIR}/include
                                             ${OPENCL_HPP_INCLUDE_DIR})

target_compile_definitions(
  ggems_core
  PUBLIC "$<$<CONFIG:Debug>:GGEMS_DEBUG_MODE>"
         "$<$<CONFIG:Release>:GGEMS_RELEASE_MODE>"
  PRIVATE "GGEMS_KERNEL_ROOT=\"${PROJECT_SOURCE_DIR}/kernels\"")

target_link_libraries(ggems_core PUBLIC OpenCL::OpenCL)

if(UNIX AND NOT APPLE)
  target_link_libraries(ggems_core PRIVATE atomic)
endif()

# ----------------------------------------------------------------------------
# Release optimization policy
# ----------------------------------------------------------------------------
ggems_apply_release_optimizations(ggems_core)

# ----------------------------------------------------------------------------
# Optional ImGui / Vulkan interface
# ----------------------------------------------------------------------------
if(GGEMS_WITH_IMGUI)
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
if(GGEMS_BUILD_TESTS)
  target_compile_definitions(ggems_core
                             PRIVATE GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ----------------------------------------------------------------------------
# Scientific validation
# ----------------------------------------------------------------------------
if(GGEMS_BUILD_VALIDATION)
  add_subdirectory(validation)
endif()

# ----------------------------------------------------------------------------
# Documentation
# ----------------------------------------------------------------------------
if(GGEMS_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# ----------------------------------------------------------------------------
# Build summary
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "================= GGEMS Build Summary =================")
message(STATUS "GGEMS version          : ${PROJECT_VERSION}")
message(STATUS "CMake version          : ${CMAKE_VERSION}")
message(STATUS "Generator              : ${CMAKE_GENERATOR}")
message(STATUS "Compiler ID            : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler version       : ${CMAKE_CXX_COMPILER_VERSION}")
message(
  STATUS "Compiler frontend      : ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
message(STATUS "Compiler path          : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard           : C++${CMAKE_CXX_STANDARD}")

if(CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Build configurations   : ${CMAKE_CONFIGURATION_TYPES}")
else()
  message(STATUS "Build type             : ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "Compiler cache         : ${GGEMS_USE_COMPILER_CACHE}")
message(STATUS "Build tests            : ${GGEMS_BUILD_TESTS}")
message(STATUS "Build documentation    : ${GGEMS_BUILD_DOCS}")
message(STATUS "ImGui UI enabled       : ${GGEMS_WITH_IMGUI}")
message(STATUS "Build validation       : ${GGEMS_BUILD_VALIDATION}")
message(STATUS "========================================================")
message(STATUS "")
