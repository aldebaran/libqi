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
cmake_dependent_option(WITH_BOOST_LOCALE "Use boost.locale to have an utf-8 locale" ON "BOOST_LOCALE_PACKAGE_FOUND" OFF)

if (WITH_BOOST_LOCALE)
  message(STATUS "UTF8: Using boost.locale")
else()
  message(STATUS "UTF8: Using embedded fallback implementation")
endif()

set(H qi/config.hpp
      qi/os.hpp
      qi/api.hpp
      qi/path.hpp
      qi/path/sdklayout.hpp
      qi/application.hpp
      qi/log.hpp
      qi/locale.hpp
      qi/noncopyable.hpp
      qi/log/consoleloghandler.hpp
      qi/log/fileloghandler.hpp
      qi/log/headfileloghandler.hpp
      )

set(C src/dlfcn.cpp
      src/path.cpp
      src/application.cpp
      src/sdklayout.cpp
      src/filesystem.hpp
      src/filesystem.cpp
      src/log.cpp
      src/consoleloghandler.cpp
      src/fileloghandler.cpp
      src/headfileloghandler.cpp
      )

if (WIN32)
  list(APPEND C src/os_debugger_win32.cpp src/os_launch_win32.cpp src/os_win32.cpp)
else()
  list(APPEND C src/os_debugger_posix.cpp src/os_launch_posix.cpp src/os_posix.cpp)
endif()

if (WITH_BOOST_LOCALE)
  set(C ${C} src/locale-boost-locale.cpp)
else()
  set(C ${C} src/locale-light.cpp)
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

if (WITH_BOOST_LOCALE)
  use_lib(qi BOOST_LOCALE)
endif()

if(MSVC)
# Temporary ugly hack: we can't find out
# where the boost_date_time dependency comes from:
  use_lib(qi BOOST_DATE_TIME)
endif()

if(UNIX AND NOT APPLE)
  use_lib(qi DL)
endif()

if (QI_SDK_DIR)
qi_install_header(KEEP_RELATIVE_PATHS ${H})
endif()
stage_lib(qi QI)

add_subdirectory(examples)
