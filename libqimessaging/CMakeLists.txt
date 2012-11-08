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

qi_sanitize_compile_flags(HIDDEN_SYMBOLS)


qi_add_optional_package(PYTHON)
set(WITH_PYTHON ON)
set(WITH_DOTNET OFF)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PERF     "Performances test" ON)
option(WITH_QT       "Qt support"       OFF)

qi_add_optional_package(QISSH)

add_definitions(" -DBOOST_FILESYSTEM_VERSION=3 ")

if (WIN32)
  add_definitions(" -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN")
endif()

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif()

qi_create_config_h(CONFIG_H "qimessaging/config.hpp.in" "qimessaging/config.hpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/c)

find_package(qiprobes)
qi_add_probes(src/tp_qimessaging.in.h
              PROVIDER qi_probes_qimessaging
              INSTRUMENTED_FILES src/session.cpp)

set(PUBLIC_HEADERS
                   qimessaging/api.hpp
                   qimessaging/details/datastream.hxx
                   qimessaging/datastream.hpp
                   qimessaging/file.hpp
                   qimessaging/gateway.hpp
                   qimessaging/message.hpp
                   qimessaging/servicedirectory.hpp
                   qimessaging/serviceinfo.hpp
                   qimessaging/session.hpp
                   qimessaging/transportserver.hpp
                   qimessaging/transportsocket.hpp
                   qimessaging/url.hpp
)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)
qi_submodule_create("qi-messaging" ${CONFIG_H}
                                   ${PUBLIC_HEADERS}

                                   ${CMAKE_CURRENT_BINARY_DIR}/tp_qimessaging.h
                                   src/boundobject.cpp
                                   src/boundobject.hpp
                                   src/datastream_p.hpp
                                   src/datastream.cpp
                                   src/gateway.cpp
                                   src/message.cpp
                                   src/message_p.hpp
                                   src/messagedispatcher.hpp
                                   src/messagedispatcher.cpp
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
                                   src/servicewatcher.hpp
                                   src/servicewatcher.cpp
                                   src/transportserver.cpp
                                   src/transportserver_p.hpp
                                   src/transportserverlibevent_p.cpp
                                   src/transportserverlibevent_p.hpp
                                   src/transportserverdummy_p.cpp
                                   src/transportserverdummy_p.hpp
                                   src/transportsocket.cpp
                                   src/transportsocket_p.hpp
                                   src/transportsocketcache.cpp
                                   src/transportsocketcache.hpp
                                   src/tcptransportsocket.cpp
                                   src/tcptransportsocket.hpp
                                   src/tcptransportsocket_p.cpp
                                   src/tcptransportsocket_p.hpp
                                   src/file.cpp
                                   src/url.cpp
                                   src/url_p.hpp
                                   src/serverresult.hpp
                                   )

add_subdirectory(c)

qi_create_lib(qimessaging SUBMODULE qi-messaging qi-messaging-c
                 SHARED DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD LIBEVENT QITYPE)
if(WIN32)
  qi_use_lib(qimessaging WSA)
endif(WIN32)

qi_stage_lib(qimessaging)

qi_add_subdirectory(examples WITH_EXAMPLES)
#qi_gen_doc(DOXYFILES "Doxyfile")

#target_link_libraries(qimessaging "-Wl,--no-undefined")


qi_add_optional_package(QT_QTCORE)
if(WITH_QT_QTCORE AND WITH_QT)
  add_subdirectory("qt")
endif()
add_subdirectory("bin")
if(WITH_PYTHON)
  add_subdirectory("python")
endif()
if (WITH_DOTNET)
  add_subdirectory("dotnet")
endif()
add_subdirectory("c/test")
add_subdirectory("c/bin")
add_subdirectory("tests")

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_EXAMPLES  : ${WITH_EXAMPLES}")
message(STATUS "WITH_PERF      : ${WITH_PERF}")
message(STATUS "WITH_PYTHON    : ${WITH_PYTHON}")
message(STATUS "WITH_QT_QTCORE : ${WITH_QT_QTCORE}")
message(STATUS "WITH_DOTNET    : ${WITH_DOTNET}")
