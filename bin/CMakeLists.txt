## Copyright (c) 2012, 2013, 2014, 2015 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 2.8)
project(LibQiBin)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

option(WITH_PERF     "Performances test" ON)

add_subdirectory("qicli")
add_subdirectory("qiclient")
add_subdirectory("qiservice")
add_subdirectory("qimaster")
add_subdirectory("qigateway")
add_subdirectory("perfs")
