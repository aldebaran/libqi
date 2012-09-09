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

enable_testing()

qi_add_optional_package(PYTHON)
set(WITH_PYTHON ON)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PERF     "Performances test" ON)

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
                   qimessaging/details/future.hxx
                   qimessaging/details/value.hpp
                   qimessaging/details/metatype.hxx
                   qimessaging/details/metafunction.hxx
                   qimessaging/details/metavalue.hxx
                   qimessaging/details/value.hxx
                   qimessaging/details/object.hxx
                   qimessaging/url.hpp
                   qimessaging/event_loop.hpp
                   qimessaging/file.hpp
                   qimessaging/object.hpp
                   qimessaging/object_factory.hpp
                   qimessaging/session.hpp
                   qimessaging/metaevent.hpp
                   qimessaging/metamethod.hpp
                   qimessaging/metaobject.hpp
                   qimessaging/metaobjectbuilder.hpp
                   qimessaging/service_directory.hpp
                   qimessaging/signature.hpp
                   qimessaging/api.hpp
                   qimessaging/message.hpp
                   qimessaging/metafunction.hpp
                   qimessaging/metatype.hpp
                   qimessaging/metavalue.hpp
                   qimessaging/signal.hpp
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
                                   src/buffer_p.hpp
                                   src/bufferreader.cpp
                                   src/datastream.cpp
                                   src/event_loop.cpp
                                   src/event_loop_p.hpp
                                   src/gateway.cpp
                                   src/message.cpp
                                   src/message_p.hpp
                                   src/metamethod.cpp
                                   src/metamethod_p.cpp
                                   src/metamethod_p.hpp
                                   src/metaevent.cpp
                                   src/metaevent_p.cpp
                                   src/metaevent_p.hpp
                                   src/metaobject.cpp
                                   src/metaobject_p.hpp
                                   src/metafunction.cpp
                                   src/object_p.hpp
                                   src/object.cpp
                                   src/object_p.cpp
                                   src/metaobjectbuilder.cpp
                                   src/remoteobject.cpp
                                   src/remoteobject_p.hpp
                                   src/service_directory.cpp
                                   src/service_directory_p.hpp
                                   src/service_directory_client.hpp
                                   src/service_directory_client.cpp
                                   src/service_info.cpp
                                   src/session.cpp
                                   src/session_p.hpp
                                   src/server_client.hpp
                                   src/server_client.cpp
                                   src/session_service.hpp
                                   src/session_service.cpp
                                   src/session_services.hpp
                                   src/session_services.cpp
                                   src/session_server.hpp
                                   src/session_server.cpp
                                   src/service_watcher.hpp
                                   src/service_watcher.cpp
                                   src/signal.cpp
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
                                   src/signature.cpp)


add_subdirectory(c)

qi_create_lib(qimessaging SUBMODULE qi-messaging qi-messaging-c
                 SHARED DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD LIBEVENT)
if(WIN32)
  qi_use_lib(qimessaging WSA)
endif(WIN32)

qi_stage_lib(qimessaging)

qi_add_subdirectory(examples WITH_EXAMPLES)
#qi_gen_doc(DOXYFILES "Doxyfile")

#target_link_libraries(qimessaging "-Wl,--no-undefined")


qi_add_optional_package(QT_QTCORE)
if(WITH_QT_QTCORE)
  add_subdirectory("qt")
endif()
add_subdirectory("bin")
if(WITH_PYTHON)
  add_subdirectory("python")
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
