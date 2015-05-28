## Copyright (c) 2012, 2013, 2014, 2015 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 2.8)
project(LibQiPamAuth)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(PAMAUTH_SRC pamclientauthenticator.hpp
          pamclientauthenticator.cpp
          pamauthapi.hpp)

qi_add_optional_package(PAM)

if (NOT WIN32 AND WITH_PAM)
  set(PAMAUTH_SRC ${PAMAUTH_SRC}
          pamauthprovider.hpp
          pamauthprovider.cpp)
endif()

qi_create_lib(qipamauth SHARED
          ${PAMAUTH_SRC}
          DEPENDS QI)

if (NOT WIN32 AND WITH_PAM)
  qi_use_lib(qipamauth PAM)
endif()
