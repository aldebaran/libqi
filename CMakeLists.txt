##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011, 2012 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(libqimessaging)
find_package(qibuild)

enable_testing()

qi_sanitize_compile_flags(HIDDEN_SYMBOLS)


set(WITH_DOTNET OFF)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PYTHON   "Python bindings"   ON)

# Desactivate Python and .NET bindings for Android
if (ANDROID)
   set(WITH_PYTHON OFF)
   set(WITH_DOTNET OFF)
endif()

qi_add_optional_package(QISSH)

add_definitions(" -DBOOST_FILESYSTEM_VERSION=3 ")

if (WIN32)
  # _SCL_SECURE_NO_WARNINGS, _CRT_SECURE_NO_WARNINGS
  # prevents MS from being too pedantic, and allows more unix style code
  #
  # WIN32_LEAN_AND_MEAN
  # prevents windows.h from including winsock.h and removes extras
  #
  # /wd4503
  # Avoids 'decorated name length' warnings that come from boost variant's
  # heavy use of templates
  add_definitions(" -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN /wd4503")
endif()

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif()

if (WITH_SSL)
 add_definitions(" -DWITH_SSL ")
 set(_ssldeps "OPENSSL")
else()
 set(_ssldeps "")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/c)

set(PUBLIC_HEADERS
                   qimessaging/api.hpp
                   qimessaging/gateway.hpp
                   qimessaging/servicedirectory.hpp
                   qimessaging/serviceinfo.hpp
                   qimessaging/session.hpp
                   qimessaging/binarycodec.hpp
                   qimessaging/url.hpp
)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)
set(SRCS ${CONFIG_H}
         ${PUBLIC_HEADERS}

         src/boundobject.cpp
         src/boundobject.hpp
         src/binarydecoder.hpp
         src/binaryencoder.hpp
         src/binarycoder_p.hpp
         src/binarycoder.cpp
         src/gateway.cpp
         src/message.hpp
         src/message.cpp
         src/messagedispatcher.hpp
         src/messagedispatcher.cpp
         src/objecthost.hpp
         src/objecthost.cpp
         src/objectregistrar.hpp
         src/objectregistrar.cpp
         src/remoteobject.cpp
         src/remoteobject_p.hpp
         src/servicedirectory.cpp
         src/servicedirectory_p.hpp
         src/servicedirectoryclient.hpp
         src/servicedirectoryclient.cpp
         src/serviceinfo.cpp
         src/serviceinfo_p.hpp
         src/session.cpp
         src/session_p.hpp
         src/sessionservice.hpp
         src/sessionservice.cpp
         src/sessionservices.hpp
         src/sessionservices.cpp
         src/server.hpp
         src/server.cpp
         src/transportserver.hpp
         src/transportserver.cpp
         src/transportserver_p.hpp
         src/transportserverasio_p.cpp
         src/transportserverasio_p.hpp
         src/transportsocket.hpp
         src/transportsocket.cpp
         src/transportsocketcache.cpp
         src/transportsocketcache.hpp
         src/tcptransportsocket.cpp
         src/tcptransportsocket.hpp
         src/url.cpp
         src/url_p.hpp
         src/serverresult.hpp
         )

#normalize BUILD_SHARED_LIBS, default ON

if (NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()

# Convert ON/OFF to SHARED/STATIC for create_lib argument
set(_libtype)
if (BUILD_SHARED_LIBS)
  set(_libtype SHARED)
else()
  set(_libtype STATIC)
endif()

qi_create_lib(qimessaging
  ${_libtype}
  ${SRCS}
  DEPENDS
    QI BOOST BOOST_DATE_TIME BOOST_THREAD QITYPE ${_ssldeps}
)

if(WIN32)
  qi_use_lib(qimessaging WSA)
endif(WIN32)

qi_stage_lib(qimessaging)

qi_add_subdirectory(examples WITH_EXAMPLES)
#qi_gen_doc(DOXYFILES "Doxyfile")

#target_link_libraries(qimessaging "-Wl,--no-undefined")

qi_add_subdirectory(c)

qi_add_subdirectory("bin")

qi_add_subdirectory("python" WITH_PYTHON)
qi_add_subdirectory("dotnet" WITH_DOTNET)
qi_add_subdirectory("c/test" BUILD_TESTS)
qi_add_subdirectory("c/bin"  BUILD_TESTS)
qi_add_subdirectory("tests"  BUILD_TESTS)

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_EXAMPLES  : ${WITH_EXAMPLES}")
message(STATUS "WITH_PYTHON    : ${WITH_PYTHON}")
message(STATUS "WITH_DOTNET    : ${WITH_DOTNET}")
message(STATUS "WITH_SSL       : ${WITH_SSL}")
