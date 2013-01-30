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
option(WITH_QT       "Qt support"       OFF)
option(WITH_PYTHON   "Python bindings"   ON)

# Desactivate Python and .NET bindings for Android
if (ANDROID)
   set(WITH_PYTHON OFF)
   set(WITH_DOTNET OFF)
endif()

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
                   qimessaging/binarydecoder.hpp
                   qimessaging/binaryencoder.hpp
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
                                   src/binarycoder_p.hpp
                                   src/binarycoder.cpp
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
                                   src/transportserver.cpp
                                   src/transportserver_p.hpp
                                   src/transportserverasio_p.cpp
                                   src/transportserverasio_p.hpp
                                   src/transportserverdummy_p.cpp
                                   src/transportserverdummy_p.hpp
                                   src/transportsocket.cpp
                                   src/transportsocket_p.hpp
                                   src/transportsocketcache.cpp
                                   src/transportsocketcache.hpp
                                   src/tcptransportsocket.cpp
                                   src/tcptransportsocket.hpp
                                   src/url.cpp
                                   src/url_p.hpp
                                   src/serverresult.hpp
                                   )

qi_add_subdirectory(c)
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
  SUBMODULE
    qi-messaging qi-messaging-c
  DEPENDS
    QI BOOST BOOST_DATE_TIME BOOST_THREAD QITYPE
)

if(WIN32)
  qi_use_lib(qimessaging WSA)
endif(WIN32)

qi_stage_lib(qimessaging)

qi_add_subdirectory(examples WITH_EXAMPLES)
#qi_gen_doc(DOXYFILES "Doxyfile")

#target_link_libraries(qimessaging "-Wl,--no-undefined")


qi_add_optional_package(QT_QTCORE)

qi_add_subdirectory("qt" WITH_QT_CORE WITH_QT)
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
message(STATUS "WITH_QT_QTCORE : ${WITH_QT_QTCORE}")
message(STATUS "WITH_DOTNET    : ${WITH_DOTNET}")
