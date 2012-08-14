##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011, 2012 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(libqimessaging)

enable_testing()

qi_add_optional_package(PYTHON)
set(WITH_PYTHON OFF)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PERF     "Performances test" ON)

qi_add_optional_package(QISSH)

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
                   qimessaging/details/functorresult.hxx
                   qimessaging/details/boostfunctor.hxx
                   qimessaging/details/future.hxx
                   qimessaging/details/functor.hxx
                   qimessaging/details/type_signature.hpp
                   qimessaging/details/function_signature.hpp
                   qimessaging/details/qt_signature.hpp
                   qimessaging/details/callfunctor.hpp
                   qimessaging/details/makefunctor.hpp
                   qimessaging/details/stl_signature.hpp
                   qimessaging/value.hpp
                   qimessaging/url.hpp
                   qimessaging/file.hpp
                   qimessaging/object.hpp
                   qimessaging/session.hpp
                   qimessaging/metaevent.hpp
                   qimessaging/metamethod.hpp
                   qimessaging/object.hxx
                   qimessaging/value.hxx
                   qimessaging/service_directory.hpp
                   qimessaging/functor.hpp
                   qimessaging/signature.hpp
                   qimessaging/api.hpp
                   qimessaging/message.hpp
                   qimessaging/transport_server.hpp
                   qimessaging/transport_socket.hpp
                   qimessaging/future.hpp
                   qimessaging/datastream.hpp
                   qimessaging/bufferreader.hpp
                   qimessaging/buffer.hpp
                   qimessaging/gateway.hpp
                   qimessaging/service_info.hpp
)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)
qi_submodule_create("qi-messaging" ${CONFIG_H}
                                   ${PUBLIC_HEADERS}

                                   ${CMAKE_CURRENT_BINARY_DIR}/tp_qimessaging.h
                                   src/buffer.cpp
                                   src/bufferreader.cpp
                                   src/datastream.cpp
                                   src/gateway.cpp
                                   src/message.cpp
                                   src/network_thread.cpp
                                   src/network_thread.hpp
                                   src/metamethod.cpp
                                   src/metamethod_p.cpp
                                   src/metamethod_p.hpp
                                   src/metaevent.cpp
                                   src/metaevent_p.cpp
                                   src/metaevent_p.hpp
                                   src/object_p.hpp
                                   src/object.cpp
                                   src/object_p.cpp
                                   src/remoteobject.cpp
                                   src/remoteobject_p.hpp
                                   src/service_directory.cpp
                                   src/service_info.cpp
                                   src/session.cpp
                                   src/session_p.hpp
                                   src/signature_convertor.cpp
                                   src/signature_convertor.hpp
                                   src/signature_iterator.cpp
                                   src/transport_server.cpp
                                   src/transport_server_p.hpp
                                   src/transport_server_libevent_p.cpp
                                   src/transport_server_libevent_p.hpp
                                   src/transport_server_dummy_p.cpp
                                   src/transport_server_dummy_p.hpp
                                   src/transport_socket.cpp
                                   src/transport_socket_p.cpp
                                   src/transport_socket_p.hpp
                                   src/transport_socket_libevent_p.cpp
                                   src/transport_socket_libevent_p.hpp
                                   src/transport_socket_dummy_p.cpp
                                   src/transport_socket_dummy_p.hpp
                                   src/value.cpp
                                   src/file.cpp
                                   src/url.cpp
                                   src/future.cpp
                                   src/server_functor_result_future_p.hpp
                                   src/functor.cpp)


add_subdirectory(c)

qi_create_lib(qimessaging SUBMODULE qi-messaging qi-messaging-c
                 #qi-cbinding
                 DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD LIBEVENT)
if(WIN32)
  qi_use_lib(qimessaging WSA)
endif(WIN32)

qi_stage_lib(qimessaging)

#add_subdirectory(test)
qi_add_subdirectory(examples WITH_EXAMPLES)
qi_add_subdirectory(perfs     WITH_PERF)
#add_subdirectory(bin)
#qi_gen_doc(DOXYFILES "Doxyfile")




qi_add_optional_package(QT_QTCORE)
if(WITH_QT_QTCORE)
  add_subdirectory("qt")
endif()
add_subdirectory("bin")
if(WITH_PYTHON)
  add_subdirectory("python")
endif()
add_subdirectory("tests")

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_EXAMPLES  : ${WITH_EXAMPLES}")
message(STATUS "WITH_PERF      : ${WITH_PERF}")
message(STATUS "WITH_PYTHON    : ${WITH_PYTHON}")
message(STATUS "WITH_QT_QTCORE : ${WITH_QT_QTCORE}")
