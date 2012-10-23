## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
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

enable_testing()
include(CMakeDependentOption)

if(ANDROID)
  set(_lock_free_default FALSE)
else()
  set(_lock_free_default TRUE)
endif()
option(USE_BOOST_LOCK_FREE "Use boost/lockfree for asynchronous qi::log"
  ${_lock_free_default}
)

if(USE_BOOST_LOCK_FREE)
  add_definitions("-D QI_USE_BOOST_LOCK_FREE")
endif()

set(H
  qi/details/future.hxx
  qi/application.hpp
  qi/atomic.hpp
  qi/buffer.hpp
  qi/bufferreader.hpp
  qi/config.hpp
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
  qi/shared_ptr.hpp
  qi/qi.hpp
  qi/threadpool.hpp
  qi/eventloop.hpp
  )

set(C
  src/dlfcn.cpp
  src/path.cpp
  src/application.cpp
  src/buffer.cpp
  src/buffer_p.hpp
  src/bufferreader.cpp
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
  src/utils.hpp
  src/utils.cpp
  src/threadpool.cpp
  src/threadpool_p.hpp
  src/eventloop.cpp
  src/eventloop_p.hpp
  )

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

if (WITH_QT_QTCORE)
  list(APPEND C src/sdklayout-qt.cpp)
else()
  list(APPEND C src/sdklayout-boost.cpp)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
if(USE_BOOST_LOCK_FREE)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external")
endif()

if(NOT DEFINED BUILD_SHARED_LIBS OR "${BUILD_SHARED_LIBS}")
  qi_create_lib(qi SHARED ${H} ${C})
else()
  qi_create_lib(qi STATIC ${H} ${C})
endif()


if (WITH_QT_QTCORE)
  qi_use_lib(qi QT_QTCORE)
endif()

qi_use_lib(qi BOOST_FILESYSTEM BOOST_THREAD BOOST_PROGRAM_OPTIONS LIBEVENT)

if (UNIX)
  qi_use_lib(qi PTHREAD)
endif()

if(MSVC)
  # Temporary ugly hack: we can't find out
  # where the boost_date_time dependency comes from:
  qi_use_lib(qi BOOST_DATE_TIME)
endif()

if(UNIX AND NOT APPLE)
  qi_use_lib(qi DL)
endif()

if(WIN32)
  qi_use_lib(qi WSOCK IPHLPAPI)
endif()

qi_install_header(${H} KEEP_RELATIVE_PATHS)
qi_stage_lib(qi QI)

add_subdirectory(examples)
add_subdirectory(tests)
