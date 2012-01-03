## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 2.8)
project(LibQi)
include("qibuild.cmake")

if (EFFECTIVE_CPP)
  add_definitions(" -Weffc++ ")
endif()

add_definitions("-DBOOST_FILESYSTEM_VERSION=3")
if(MSVC)
  add_definitions(" -DWIN32_LEAN_AND_MEAN ")
endif()

qi_add_optional_package(QT_QTCORE "Enable QT")

enable_testing()
include(CMakeDependentOption)

set(H
  qi/config.hpp
  qi/error.hpp
  qi/exception.hpp
  qi/log/consoleloghandler.hpp
  qi/log/fileloghandler.hpp
  qi/log/headfileloghandler.hpp
  qi/log/tailfileloghandler.hpp
  qi/log.hpp
  qi/macro.hpp
  qi/os.hpp
  qi/path.hpp
  qi/qi.hpp
  )

set(C
  src/dlfcn.cpp
  src/path.cpp
  src/application.cpp
  src/sdklayout.hpp
  src/filesystem.hpp
  src/filesystem.cpp
  src/log.cpp
  src/consoleloghandler.cpp
  src/fileloghandler.cpp
  src/headfileloghandler.cpp
  src/tailfileloghandler.cpp
  src/locale-light.cpp
  )

if (WIN32)
  list(APPEND C src/os_launch_win32.cpp src/os_win32.cpp)
else()
  list(APPEND C src/os_launch_posix.cpp src/os_posix.cpp)
endif()

if (WITH_QT_QTCORE)
  list(APPEND C src/sdklayout-qt.cpp)
else()
  list(APPEND C src/sdklayout-boost.cpp)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("src/")

if("${BUILDCONFIG}" STREQUAL "python-sdk")
  qi_create_lib(qi STATIC ${H} ${C})
else()
  qi_create_lib(qi SHARED ${H} ${C})
endif()


if (WITH_QT_QTCORE)
  qi_use_lib(qi QT_QTCORE)
endif()

qi_use_lib(qi BOOST_FILESYSTEM BOOST_THREAD)

if(MSVC)
  # Temporary ugly hack: we can't find out
  # where the boost_date_time dependency comes from:
  qi_use_lib(qi BOOST_DATE_TIME)
endif()

if(UNIX AND NOT APPLE)
  qi_use_lib(qi DL)
endif()

qi_install_header(${H} KEEP_RELATIVE_PATHS)
qi_stage_lib(qi QI)

add_subdirectory(examples)
add_subdirectory(tests)
