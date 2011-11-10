##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(libqimessaging)
include("qibuild.cmake")

enable_testing()

include(CMakeDependentOption)
find_package(ZEROMQ)
find_package(PYTHON)

cmake_dependent_option(WITH_PYTHON   "Build python bindings" OFF "PYTHON_FOUND" ON)
cmake_dependent_option(WITH_ZEROMQ   "Use zeromq"   ON "ZEROMQ_FOUND" OFF)
cmake_dependent_option(WITH_PROTOBUF "Use protobuf" OFF "PROTOBUF_FOUND" OFF)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PERF     "Performances test" ON)


project(libqimessaging)
cmake_minimum_required(VERSION 2.6.4)
include("qibuild.cmake")

if (WIN32)
  add_definitions(" -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN")
endif()

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif()

qi_create_config_h(CONFIG_H "qimessaging/config.hpp.in" "qimessaging/config.hpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

qi_submodule_create("qi-messaging"     "qimessaging/*.h[px][px]" "src/*.cpp")
qi_submodule_create("qi-messaging-transport"        "qimessaging/transport/*.h[px][px]" "src/transport/*.[ch][px][px]")

qi_submodule_create("qi-messaging-c"   "qimessaging/*.h" "src/c/*.[hc][px][px]")
qi_submodule_create("qi-preference"    qipreference/preference.hpp src/preference/preference.cpp)
qi_submodule_create("qi-messaging-src" "src/messaging/*.[ch][px][px]")
qi_submodule_add("qi-messaging-src"    "src/messaging/network/*.[ch][px][px]")
qi_submodule_add("qi-messaging-src"
                 src/transport/zmq/zmq_client_backend.cpp
                 src/transport/zmq/zmq_client_backend.hpp
                 src/transport/zmq/zmq_connection_handler.cpp
                 src/transport/zmq/zmq_connection_handler.hpp
                 src/transport/zmq/zmq_forwarder_backend.cpp
                 src/transport/zmq/zmq_forwarder_backend.hpp
                 src/transport/zmq/zmq_poll_client.cpp
                 src/transport/zmq/zmq_poll_client.hpp
                 src/transport/zmq/zmq_publisher.cpp
                 src/transport/zmq/zmq_publisher.hpp
                 src/transport/zmq/zmq_simple_server_backend.cpp
                 src/transport/zmq/zmq_simple_server_backend.hpp
                 src/transport/zmq/zmq_subscriber.cpp
                 src/transport/zmq/zmq_subscriber.hpp
                 IF WITH_ZEROMQ
                 DEPENDS ZEROMQ)

qi_submodule_add("qi-messaging-src"
                 src/transport/ssh/ssh_client_backend.cpp
                 src/transport/ssh/ssh_client_backend.hpp
                 #IF WITH_QISSH
                 DEPENDS QISSH)

# Needs the Binder Kernel driver
qi_submodule_add("qi-messaging-src"
                 src/transport/binder/linux/binder.h
                 src/transport/binder/binder.h
                 src/transport/binder/binder.cpp
                 src/transport/binder/binder_client_backend.cpp
                 src/transport/binder/binder_client_backend.hpp
                 src/transport/binder/binder_server_backend.cpp
                 src/transport/binder/binder_server_backend.hpp
                )

qi_submodule_create("qi-functors"        "qimessaging/functors/*.h[px][px]" "qimessaging/functors/detail/*.h[px][px]")

qi_submodule_create("qi-signature"       "qimessaging/signature/*.h[px][px]" "qimessaging/signature.hpp" "qimessaging/signature/detail/*.h[px][px]"
                                         "src/signature/*.[hc][px][px]")

qi_submodule_create("qi-serialization"   "qimessaging/serialization/*.h[px][px]" "qimessaging/serialization.hpp"
                                         "src/serialization/*.[hc][px][px]")

qi_create_lib(qimessaging SRC ${CONFIG_H}
                 SUBMODULE qi-messaging qi-messaging-src qi-messaging-c qi-functors qi-signature qi-serialization qi-messaging-transport qi-preference
                 #qi-cbinding
                 DEPENDS QI BOOST BOOST_DATE_TIME BOOST_THREAD TINYXML)
qi_stage_lib(qimessaging)

#add_subdirectory(test)
qi_add_subdirectory(examples WITH_EXAMPLES)
#qi_add_subdirectory(perf     WITH_PERF)
#add_subdirectory(bin)
#qi_gen_doc(DOXYFILES "Doxyfile")

#qi_add_subdirectory(bindings/python WITH_PYTHON)
#add_subdirectory(bindings/ruby)

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_ZEROMQ  : ${WITH_ZEROMQ}")
message(STATUS "WITH_PROTOBUF: ${WITH_PROTOBUF}")
message(STATUS "WITH_EXAMPLES: ${WITH_EXAMPLES}")
message(STATUS "WITH_PERF    : ${WITH_PERF}")
message(STATUS "WITH_PYTHON  : ${WITH_PYTHON}")


add_subdirectory("bin")
add_subdirectory("python")
add_subdirectory("tests")

