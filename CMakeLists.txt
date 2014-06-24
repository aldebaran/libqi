##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011, 2012, 2013 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(libqimessaging)
find_package(qibuild)

enable_testing()

qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

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

set(PUBLIC_HEADERS
                   qi/messaging/api.hpp
                   qi/messaging/autoservice.hpp
                   qi/messaging/details/autoservice.hxx
                   qi/messaging/gateway.hpp
                   qi/messaging/serviceinfo.hpp
                   qi/applicationsession.hpp
                   qi/session.hpp
                   qi/url.hpp
)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)
set(SRCS ${CONFIG_H}
         ${PUBLIC_HEADERS}
         src/messaging/applicationsession.cpp
         src/messaging/boundobject.cpp
         src/messaging/boundobject.hpp
         src/messaging/gateway.cpp
         src/messaging/message.hpp
         src/messaging/message.cpp
         src/messaging/messagedispatcher.hpp
         src/messaging/messagedispatcher.cpp
         src/messaging/objecthost.hpp
         src/messaging/objecthost.cpp
         src/messaging/objectregistrar.hpp
         src/messaging/objectregistrar.cpp
         src/messaging/remoteobject.cpp
         src/messaging/remoteobject_p.hpp
         src/messaging/servicedirectory.cpp
         src/messaging/servicedirectory.hpp
         src/messaging/servicedirectoryclient.hpp
         src/messaging/servicedirectoryclient.cpp
         src/messaging/serviceinfo.cpp
         src/messaging/session.cpp
         src/messaging/session_p.hpp
         src/messaging/sessionservice.hpp
         src/messaging/sessionservice.cpp
         src/messaging/sessionservices.hpp
         src/messaging/sessionservices.cpp
         src/messaging/server.hpp
         src/messaging/server.cpp
         src/messaging/transportserver.hpp
         src/messaging/transportserver.cpp
         src/messaging/transportserverasio_p.cpp
         src/messaging/transportserverasio_p.hpp
         src/messaging/transportsocket.hpp
         src/messaging/transportsocket.cpp
         src/messaging/transportsocketcache.cpp
         src/messaging/transportsocketcache.hpp
         src/messaging/tcptransportsocket.cpp
         src/messaging/tcptransportsocket.hpp
         src/messaging/url.cpp
         src/messaging/serverresult.hpp
         )

#normalize BUILD_SHARED_LIBS, default ON

if (NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()

# Convert ON/OFF to SHARED/STATIC for create_lib argument
set(_libtype)
if (BUILD_SHARED_LIBS OR ANDROID)
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

qi_stage_lib(qimessaging INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/compat")

qi_add_subdirectory("tests")
qi_add_subdirectory("bin")

add_subdirectory("compat")

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_EXAMPLES  : ${WITH_EXAMPLES}")
message(STATUS "WITH_DOTNET    : ${WITH_DOTNET}")
message(STATUS "WITH_SSL       : ${WITH_SSL}")
