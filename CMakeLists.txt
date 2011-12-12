##
## Copyright (C) 2011 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(LibQi)
include("bootstrap.cmake")

if (EFFECTIVE_CPP)
  add_definitions(" -Weffc++ ")
endif()

enable_testing()
#find_package(BOOST_LOCALE QUIET)
include(CMakeDependentOption)

set(H qi/config.hpp
      qi/os.hpp
      qi/macro.hpp
      qi/path.hpp
      qi/exception.hpp
      qi/qi.hpp
      qi/log.hpp
      qi/log/consoleloghandler.hpp
      qi/log/fileloghandler.hpp
      qi/log/headfileloghandler.hpp
      qi/log/tailfileloghandler.hpp
      )

set(C src/dlfcn.cpp
      src/path.cpp
      src/application.cpp
      src/sdklayout.hpp
      src/sdklayout.cpp
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

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("src/")

if("${BUILDCONFIG}" STREQUAL "python-sdk")
  create_lib(qi STATIC ${H} ${C})
else()
  create_lib(qi SHARED ${H} ${C})
endif()

use_lib(qi BOOST_FILESYSTEM)
use_lib(qi BOOST_THREAD)

if(MSVC)
# Temporary ugly hack: we can't find out
# where the boost_date_time dependency comes from:
  use_lib(qi BOOST_DATE_TIME)
endif()

if(UNIX AND NOT APPLE)
  use_lib(qi DL)
endif()

if (QI_SDK_DIR)
  qi_install_header(${H} KEEP_RELATIVE_PATHS)
endif()
stage_lib(qi QI)

add_subdirectory(examples)
