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
                   qimessaging/details/dynamicvalue.hpp
                   qimessaging/details/type.hxx
                   qimessaging/details/metafunction.hxx
                   qimessaging/details/genericvalue.hxx
                   qimessaging/details/dynamicvalue.hxx
                   qimessaging/details/genericobject.hxx
                   qimessaging/details/genericobjectbuilder.hxx
                   qimessaging/details/objecttypebuilder.hxx
                   qimessaging/details/signal.hxx
                   qimessaging/url.hpp
                   qimessaging/eventloop.hpp
                   qimessaging/file.hpp
                   qimessaging/dynamicobject.hpp
                   qimessaging/genericobject.hpp
                   qimessaging/objecttypebuilder.hpp
                   qimessaging/genericobjectbuilder.hpp
                   qimessaging/objectfactory.hpp
                   qimessaging/session.hpp
                   qimessaging/metasignal.hpp
                   qimessaging/metamethod.hpp
                   qimessaging/metaobject.hpp
                   qimessaging/servicedirectory.hpp
                   qimessaging/signature.hpp
                   qimessaging/api.hpp
                   qimessaging/message.hpp
                   qimessaging/metafunction.hpp
                   qimessaging/type.hpp
                   qimessaging/genericvalue.hpp
                   qimessaging/functiontype.hpp
                   qimessaging/details/functiontype.hxx
                   qimessaging/methodtype.hpp
                   qimessaging/details/methodtype.hxx
                   qimessaging/signal.hpp
                   qimessaging/transportserver.hpp
                   qimessaging/transportsocket.hpp
                   qimessaging/typeint.hpp
                   qimessaging/future.hpp
                   qimessaging/datastream.hpp
                   qimessaging/bufferreader.hpp
                   qimessaging/buffer.hpp
                   qimessaging/gateway.hpp
                   qimessaging/serviceinfo.hpp
)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)
qi_submodule_create("qi-messaging" ${CONFIG_H}
                                   ${PUBLIC_HEADERS}

                                   ${CMAKE_CURRENT_BINARY_DIR}/tp_qimessaging.h
                                   src/buffer.cpp
                                   src/buffer_p.hpp
                                   src/bufferreader.cpp
                                   src/datastream.cpp
                                   src/dynamicobject.cpp
                                   src/eventloop.cpp
                                   src/eventloop_p.hpp
                                   src/functiontype.cpp
                                   src/gateway.cpp
                                   src/genericobjectbuilder.cpp
                                   src/genericvalue.cpp
                                   src/message.cpp
                                   src/message_p.hpp
                                   src/messagedispatcher.hpp
                                   src/messagedispatcher.cpp
                                   src/metamethod.cpp
                                   src/metasignal.cpp
                                   src/metasignal_p.cpp
                                   src/metasignal_p.hpp
                                   src/metaobject.cpp
                                   src/metaobject_p.hpp
                                   src/metafunction.cpp
                                   src/object_p.hpp
                                   src/object.cpp
                                   src/objectfactory.cpp
                                   src/objecttypebuilder.cpp
                                   src/remoteobject.cpp
                                   src/remoteobject_p.hpp
                                   src/servicedirectory.cpp
                                   src/servicedirectory_p.hpp
                                   src/servicedirectoryclient.hpp
                                   src/servicedirectoryclient.cpp
                                   src/serviceinfo.cpp
                                   src/session.cpp
                                   src/session_p.hpp
                                   src/serverclient.hpp
                                   src/serverclient.cpp
                                   src/sessionservice.hpp
                                   src/sessionservice.cpp
                                   src/sessionservices.hpp
                                   src/sessionservices.cpp
                                   src/sessionserver.hpp
                                   src/sessionserver.cpp
                                   src/servicewatcher.hpp
                                   src/servicewatcher.cpp
                                   src/signal.cpp
                                   src/signal_p.hpp
                                   src/signatureconvertor.cpp
                                   src/signatureconvertor.hpp
                                   src/signatureiterator.cpp
                                   src/staticobjecttype.cpp
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
                                   src/type.cpp
				   src/typeint.cpp
                                   src/value.cpp
                                   src/file.cpp
                                   src/url.cpp
                                   src/future.cpp
                                   src/serverresult.hpp
                                   src/signature.cpp
                                   )


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
