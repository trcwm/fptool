#
# FPTOOL is a tool for generating fixed-point VHDL code using a
# simple domain-specific hardware description language.
#
# Copyright (C) 2017 Niels A. Moseley
#
# This file is part of FPTOOL.
#
# Foobar is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Foobar is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar. If not, see <http://www.gnu.org/licenses/>.

#
# FPTool CMake make file
#

cmake_minimum_required (VERSION 3.0)
project (fptool)

# enable C++11 standard
set(CMAKE_CXX_STANDARD 11)

# compile fplib library
add_subdirectory(${CMAKE_SOURCE_DIR}/externals/fplib)

# compile fptool
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/externals/fplib/src")
file(GLOB_RECURSE sources "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable (fptool ${sources})
target_link_libraries (fptool LINK_PUBLIC fplib)
