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
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(GGEMS VERSION 2.0 HOMEPAGE_URL ggems.fr LANGUAGES CXX)

# Find pybind11 and Python
find_package(pybind11 CONFIG REQUIRED)

# Generate compile commands, useful for some environment editor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Locate source files for GGEMS
file(GLOB_RECURSE ggems_src_files CONFIGURE_DEPENDS src/*.cc)

# Add a library using FindPython's tooling
python_add_library(_ggems_lib MODULE src/tools/GGEMSPrint.cc WITH_SOABI)
target_link_libraries(_ggems_lib PRIVATE pybind11::headers)

# This is passing in the version as a define just as an example
target_compile_definitions(_ggems_lib PRIVATE VERSION_INFO=${PROJECT_VERSION})

# The install directory is the output directory
install(TARGETS _ggems_lib DESTINATION ggems)

