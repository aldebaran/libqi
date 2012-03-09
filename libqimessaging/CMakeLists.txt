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

if (EFFECTIVE_CPP)
  add_definitions(" -Weffc++ ")
endif()

# Because CMake automatically qi_EXPORTS, we can do:
if (UNIX OR MINGW)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

qi_create_config_h(CONFIG_H "qimessaging/config.hpp.in" "qimessaging/config.hpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/c)

qi_submodule_create("qi-messaging" ${CONFIG_H}
                                   qimessaging/api.hpp
                                   qimessaging/buffer.hpp
                                   qimessaging/config.hpp.in
                                   qimessaging/datastream.hpp
                                   qimessaging/functor.hpp
                                   qimessaging/gateway.hpp
                                   qimessaging/message.hpp
                                   qimessaging/object.hpp
                                   qimessaging/object.hxx
                                   qimessaging/server.hpp
                                   qimessaging/service_directory.hpp
                                   qimessaging/service_info.hpp
                                   qimessaging/session.hpp
                                   qimessaging/signature.hpp
                                   qimessaging/transport_server.hpp
                                   qimessaging/transport_socket.hpp
                                   qimessaging/value.hpp
                                   qimessaging/url.hpp
                                   qimessaging/future.hpp
                                   qimessaging/details/callfunctor.hpp
                                   qimessaging/details/function_signature.hpp
                                   qimessaging/details/functor.hxx
                                   qimessaging/details/makefunctor.hpp
                                   qimessaging/details/memberfunctor.hxx
                                   qimessaging/details/qt_signature.hpp
                                   qimessaging/details/stl_signature.hpp
                                   qimessaging/details/type_signature.hpp
                                   qimessaging/details/voidfunctor.hxx
                                   qimessaging/details/voidmemberfunctor.hxx

                                   src/buffer.cpp
                                   src/datastream.cpp
                                   src/gateway.cpp
                                   src/message.cpp
                                   src/network_thread.cpp
                                   src/network_thread.hpp
                                   src/object.cpp
                                   src/remoteobject.cpp
                                   src/remoteobject_p.hpp
                                   src/server.cpp
                                   src/service_directory.cpp
                                   src/service_info.cpp
                                   src/session.cpp
                                   src/session_p.hpp
                                   src/signature_convertor.cpp
                                   src/signature_convertor.hpp
                                   src/signature_iterator.cpp
                                   src/transport_server.cpp
                                   src/transport_server_p.hpp
                                   src/transport_socket.cpp
                                   src/value.cpp
                                   src/url.cpp
                                   src/future.cpp
                                   src/server_functor_result_future_p.hpp)


add_subdirectory(c)

qi_create_lib(qimessaging SUBMODULE qi-messaging qi-messaging-c
                 #qi-cbinding
                 DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD TINYXML LIBEVENT UUID)
if(WIN32)
  qi_use_lib(qimessaging WSA)
endif(WIN32)

qi_stage_lib(qimessaging)

#add_subdirectory(test)
qi_add_subdirectory(examples WITH_EXAMPLES)
qi_add_subdirectory(perfs     WITH_PERF)
#add_subdirectory(bin)
#qi_gen_doc(DOXYFILES "Doxyfile")


message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_EXAMPLES: ${WITH_EXAMPLES}")
message(STATUS "WITH_PERF    : ${WITH_PERF}")
message(STATUS "WITH_PYTHON  : ${WITH_PYTHON}")


add_subdirectory("qt")
add_subdirectory("bin")
if(WITH_PYTHON)
  add_subdirectory("python")
endif()
add_subdirectory("tests")

