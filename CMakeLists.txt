## Copyright (c) 2011, Aldebaran Robotics
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Aldebaran Robotics nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
## WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
## LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
## ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.6.4)
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
  qi/log.hpp
  qi/log/tailfileloghandler.hpp
  qi/macro.hpp
  qi/os.hpp
  qi/path.hpp
  qi/qi.hpp
)

set(C src/dlfcn.cpp
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
  list(APPEND C src/os_debugger_win32.cpp src/os_launch_win32.cpp src/os_win32.cpp)
else()
  list(APPEND C src/os_debugger_posix.cpp src/os_launch_posix.cpp src/os_posix.cpp)
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
