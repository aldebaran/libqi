## Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

cmake_minimum_required(VERSION 2.8)
project(LibQi)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include(CheckSymbolExists)
check_symbol_exists(_SC_HOST_NAME_MAX "unistd.h" HAVE_SC_HOST_NAME_MAX)
set_source_files_properties(src/os_posix.cpp
  PROPERTIES
    COMPILE_DEFINITIONS HAVE_SC_HOST_NAME_MAX)

# We always want boost filesystem v3
add_definitions("-DBOOST_FILESYSTEM_VERSION=3")
if(MSVC)
  add_definitions(" -DWIN32_LEAN_AND_MEAN ")
endif()

# Remove this line to use QT if usable
set(WITH_QT_QTCORE OFF)

qi_add_optional_package(QT_QTCORE "Enable QT")
qi_add_optional_package(BOOST_LOCALE "Enable qi::translate")

enable_testing()
include(CMakeDependentOption)

find_package(qiprobes)
qiprobes_create_probe(tp_qi
  src/tp_qi.in.h
  PROVIDER_NAME qi_qi)
qiprobes_instrument_files(tp_qi
  src/eventloop.cpp)

set(H
  qi/details/eventloop.hxx
  qi/details/future.hxx
  qi/details/log.hxx
  qi/details/trackable.hxx
  qi/details/warn_push_ignore_deprecated.hpp
  qi/details/warn_pop_ignore_deprecated.hpp
  qi/api.hpp
  qi/application.hpp
  qi/atomic.hpp
  qi/buffer.hpp
  qi/bufferreader.hpp
  qi/clock.hpp
  qi/clock.hxx
  qi/error.hpp
  qi/future.hpp
  qi/log/consoleloghandler.hpp
  qi/log/fileloghandler.hpp
  qi/log/headfileloghandler.hpp
  qi/log/tailfileloghandler.hpp
  qi/log.hpp
  qi/macro.hpp
  qi/os.hpp
  qi/preproc.hpp
  qi/types.hpp
  qi/path.hpp
  qi/periodictask.hpp
  qi/shared_ptr.hpp
  qi/stats.hpp
  qi/qi.hpp
  qi/threadpool.hpp
  qi/traits.hpp
  qi/trackable.hpp
  qi/eventloop.hpp
  qi/version.hpp
  qi/iocolor.hpp
  )

set(C
  src/dlfcn.cpp
  src/path.cpp
  src/application.cpp
  src/buffer.cpp
  src/buffer_p.hpp
  src/bufferreader.cpp
  src/clock.cpp
  src/sdklayout.hpp
  src/filesystem.hpp
  src/filesystem.cpp
  src/future.cpp
  src/log.cpp
  src/log_p.hpp
  src/consoleloghandler.cpp
  src/fileloghandler.cpp
  src/headfileloghandler.cpp
  src/tailfileloghandler.cpp
  src/locale-light.cpp
  src/os.cpp
  src/path_conf.cpp
  src/path_conf.hpp
  src/periodictask.cpp
  src/utils.hpp
  src/utils.cpp
  src/threadpool.cpp
  src/threadpool_p.hpp
  src/eventloop.cpp
  src/eventloop_p.hpp
  src/sdklayout-boost.cpp
  src/version.cpp
  src/iocolor.cpp
  )

if(WITH_BOOST_LOCALE)
  list(APPEND H qi/translator.hpp)
  list(APPEND C src/translator.cpp)
endif()

if (WIN32)
  list(APPEND C
    src/os_launch_win32.cpp
    src/os_win32.cpp
    src/os_debugger_win32.cpp
  )
else()
  list(APPEND C
    src/os_posix.cpp
    src/os_debugger_posix.cpp
  )
  if(ANDROID)
    list(APPEND C src/os_launch_android.cpp)
  else()
    list(APPEND C src/os_launch_posix.cpp)
  endif()
endif()

#normalize BUILD_SHARED_LIBS, default ON
if (NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()

# Set qi_STATIC_BUILD  used in config.hpp.in
set(qi_STATIC_BUILD OFF)
if (NOT BUILD_SHARED_LIBS) #lol
  set(qi_STATIC_BUILD ON)
endif()

# Convert ON/OFF to SHARED/STATIC for create_lib argument
set(_libtype)
if (BUILD_SHARED_LIBS OR ANDROID)
  set(_libtype SHARED)
else()
  set(_libtype STATIC)
endif()

qi_create_config_h(_out qi/config.hpp.in qi/config.hpp)

# Help IDEs see the generated config.hpp
list(APPEND C ${_out})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external")

qi_create_lib(qi ${_libtype} ${H} ${C} SUBMODULE tp_qi)

if (WITH_QT_QTCORE)
  qi_use_lib(qi QT_QTCORE)
endif()

qi_use_lib(qi BOOST_CHRONO BOOST_THREAD BOOST_FILESYSTEM BOOST_THREAD BOOST_PROGRAM_OPTIONS BOOST_REGEX)

if(WITH_BOOST_LOCALE)
  qi_use_lib(qi BOOST_LOCALE)
endif()

if (UNIX)
  qi_use_lib(qi PTHREAD)
endif()

if(MSVC)
  # Temporary ugly hack: we can't find out
  # where the boost_date_time dependency comes from:
  qi_use_lib(qi BOOST_DATE_TIME)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
  qi_use_lib(qi DL RT)
endif()

if(WIN32)
  qi_use_lib(qi WSOCK IPHLPAPI PSAPI)
endif()

qi_install_header(${H} KEEP_RELATIVE_PATHS)
qi_stage_lib(qi QI)

add_subdirectory(examples)
add_subdirectory(tests)
