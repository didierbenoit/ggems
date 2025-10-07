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

# Checking CMake version
cmake_minimum_required(VERSION 3.15...4.1)
project(${SKBUILD_PROJECT_NAME} VERSION ${SKBUILD_PROJECT_VERSION} HOMEPAGE_URL ggems.fr LANGUAGES CXX)

# Find pybind11 and Python
find_package(Python REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED)

# Generate compile commands, useful for some environment editor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create a python module for each directory
#set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
#file(GLOB SUBDIRS LIST_DIRECTORIES true "${INCLUDE_DIR}/GGEMS/*")
#foreach(DIR ${SUBDIRS})
#  if(IS_DIRECTORY ${DIR})
#    message(STATUS "Creating module for: ${DIR}")
#  endif()
#endforeach()

# Locate source files
file(
  GLOB tools_src_files
  CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/tools/*.cc"
  "${CMAKE_SOURCE_DIR}/python/ggems/tools/GGEMSToolsBindings.cc"
)

# Tools module
python_add_library(_tools MODULE ${tools_src_files} WITH_SOABI)
target_link_libraries(_tools PRIVATE pybind11::headers)
target_include_directories(_tools PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(_tools PRIVATE VERSION_INFO=${PROJECT_VERSION})

# Sources modules
#python_add_library(_sources MODULE ${sources_src_files} WITH_SOABI)
#target_link_libraries(_sources PRIVATE pybind11::headers)
#target_include_directories(_sources PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
#target_compile_definitions(_sources PRIVATE VERSION_INFO=${PROJECT_VERSION})

# The install directory is the output directory
install(TARGETS _tools DESTINATION ggems)
#install(TARGETS _sources DESTINATION ggems)

