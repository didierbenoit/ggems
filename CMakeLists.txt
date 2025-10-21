# ************************************************************************
# * This file is part of GGEMS.                                          *
# *                                                                      *
# * GGEMS is free software: you can redistribute it and/or modify        *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation, either version 3 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# * GGEMS is distributed in the hope that it will be useful,             *
# * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
# * GNU General Public License for more details.                         *
# *                                                                      *
# * You should have received a copy of the GNU General Public License    *
# * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
# *                                                                      *
# ************************************************************************

#-------------------------------------------------------------------------------
# CMakeLists.txt
#
# CMakeLists.txt - Compile and build the GGEMS library
#
# Authors :
#   - Julien Bert <julien.bert@univ-brest.fr>
#   - Didier Benoit <didier.benoit@inserm.fr>
#
# Generated on : 06/10/2025
#-------------------------------------------------------------------------------

# Check CMake version
cmake_minimum_required(VERSION 3.15...4.1)
project(${SKBUILD_PROJECT_NAME} VERSION ${SKBUILD_PROJECT_VERSION} HOMEPAGE_URL ggems.fr LANGUAGES CXX)

# Find Packages
find_package(pybind11 REQUIRED)

# Generate compile commands, useful for some environment editor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler detection and set flag for each compiler
include(cmake/CompilerOptions.cmake)

#-------------------------------------------------------------------------------
# Find the OpenCL v3.0
find_package(CUDAToolkit 12.0 REQUIRED)
message(STATUS "CUDA Toolkit: ${CUDAToolkit_FOUND}")
message(STATUS "CUDA Toolkit VERSION: ${CUDAToolkit_VERSION}")
message(STATUS "CUDA Toolkit INCLUDE DIRS: ${CUDAToolkit_INCLUDE_DIRS}")
message(STATUS "CUDA Toolkit LIBRARY DIR: ${CUDAToolkit_LIBRARY_DIR}")
add_definitions(
  "-DCL_HPP_TARGET_OPENCL_VERSION=300"
  "-DCL_TARGET_OPENCL_VERSION=300"
)

# Locate C++ files
FILE(GLOB GGEMS_SOURCE_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/frameworks/*.cc
)

# GGEMS C++ library building
add_library(ggems STATIC)
target_sources(ggems PRIVATE ${GGEMS_SOURCE_FILES})
target_link_libraries(ggems PRIVATE CUDA::OpenCL)
target_include_directories(ggems PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# C++/Python bindings
add_subdirectory(python/ggems)

# Google test
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
