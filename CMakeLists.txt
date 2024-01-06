#
# FPTOOL is a tool for generating fixed-point VHDL code using a
# simple domain-specific hardware description language.
#
# Copyright (C) 2017-2024 Niels A. Moseley
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

cmake_minimum_required (VERSION 3.16)
project (fptool VERSION 0.1)

message("Using: ${CMAKE_CXX_COMPILER}")

# enable exception unwinding semantics for
# Visual Studio
# enable exception unwinding semantics for
# Visual Studio
if (CMAKE_CXX_COMPILER MATCHES ".*Microsoft")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") 
endif()

# enable C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##################################################
# INCLUDE CPM PACKAGE MANAGER
##################################################

option(UseCPM "Use CPM package manager" ON)

if(UseCPM)

    set(CPM_DOWNLOAD_VERSION 0.38.2)

    if(CPM_SOURCE_CACHE)
        set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    elseif(DEFINED ENV{CPM_SOURCE_CACHE})
        set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    else()
        set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    endif()

    if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
        message(STATUS "Downloading CPM.cmake ${CPM_DOWNLOAD_VERSION} to ${CPM_DOWNLOAD_LOCATION}")
        file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION} )
    endif()

    include(${CPM_DOWNLOAD_LOCATION})

endif(UseCPM)

##################################################
## Add dependencies
##################################################

# TinyTestPP test framework
# linking name: tinytestpp
option(UseTinyTestPP "Use TinyTestPP test framework" OFF)

if(UseTinyTestPP)
    CPMAddPackage("bb:trcwm/tinytestpp#main")
endif(UseTinyTestPP)

# Cxxopts
# linking name: cxxopts
option(UseCXXOPTS "Use CXXOPTS commandline argument parser" OFF)

if(UseCXXOPTS)
    CPMAddPackage(
        GITHUB_REPOSITORY jarro2783/cxxopts
        VERSION 3.1.1
        OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL NO"
    )
endif(UseCXXOPTS)

##################################################
## CPPCHECK static analysis support
##################################################

option(UseCPPCHECK "UseCPPCHECK" ON)

if (UseCPPCHECK)
    set(CMAKE_CXX_CPPCHECK "cppcheck")
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    message("Using CPPCHECK")
    list(
        APPEND CMAKE_CXX_CPPCHECK 
            "--enable=warning"
            "--inconclusive"
            "--force" 
            "--inline-suppr"
            "--suppressions-list=${CMAKE_CURRENT_SOURCE_DIR}/cppcheck/CppCheckSuppressions.txt"
            "--library=qt"
    )
endif()

##################################################
# SET VERSION
##################################################

set(VERSIONSTRING "Version ${CMAKE_PROJECT_VERSION}")
set(COMPILERSTRING "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
configure_file(${PROJECT_SOURCE_DIR}/src/version.cpp.in ${PROJECT_SOURCE_DIR}/src/version.cpp @ONLY)

# compile fplib library
add_subdirectory(${CMAKE_SOURCE_DIR}/externals/fplib)

# compile fptool
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/externals/fplib/src")
file(GLOB_RECURSE sources "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable (fptool ${sources})
target_link_libraries (fptool LINK_PUBLIC fplib)
