##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (qi)

cmake_minimum_required(VERSION 2.6.4)
include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/options.cmake)
add_definitions(" -D_SCL_SECURE_NO_WARNINGS ")

enable_testing()
create_config_h("config.hpp.in" "qi/config.hpp")

include_directories(bindings/c/)

file(GLOB CORE_SRC                 qi/core/*.cpp qi/core/*.h*)
file(GLOB MESSAGING_SRC            qi/messaging/*.cpp qi/messaging/*.h*)
file(GLOB MESSAGING_DETAIL_SRC     qi/messaging/detail/*.cpp qi/messaging/detail/*.h*)
file(GLOB SERIALIZATION_SRC        qi/serialization/*.cpp qi/serialization/*.h*)
file(GLOB SERIALIZATION_THRIFT_SRC qi/serialization/thrift/*.cpp qi/serialization/thrift/*.h*)
file(GLOB SERIALIZATION_BOOST_SRC  qi/serialization/boost/*.cpp qi/serialization/boost/*.h*)

file(GLOB TOOLS_SRC                qi/perf/*.cpp qi/perf/*.h*)
file(GLOB LOGS_SRC                 qi/log/*.cpp qi/log/*.h*)
file(GLOB EXCEPTIONS_SRC           qi/exceptions/*.cpp qi/exceptions/*.h*)
file(GLOB TRANSPORT_SRC                   qi/transport/*.cpp qi/transport/*.h*)
file(GLOB TRANSPORT_DETAIL_SRC            qi/transport/detail/*.cpp qi/transport/detail/*.h*)
file(GLOB TRANSPORT_ZMQ_SRC               qi/transport/detail/zmq/*.cpp qi/transport/detail/zmq/*.h*)
file(GLOB TRANSPORT_THRIFT_SRC            qi/transport/detail/thrift/*.cpp qi/transport/detail/thrift/*.h*)
file(GLOB TRANSPORT_NETWORK_SRC           qi/transport/detail/network/*.cpp qi/transport/detail/network/*.h*)
file(GLOB FUNCTORS_SRC                    qi/functors/*.cpp qi/functors/*.h*)
file(GLOB FUNCTORS_DETAIL_SRC             qi/functors/detail/*.h*)
file(GLOB SIGNATURE_SRC                   qi/signature/*.cpp qi/signature/*.h*)
file(GLOB SIGNATURE_DETAIL_SRC            qi/signature/detail/*.h*)
file(GLOB CBINDINGS_SRC                   bindings/c/qi/qi.h bindings/c/cbindings.cpp)

source_group( messaging                 FILES ${MESSAGING_SRC})
source_group( messaging\\detail         FILES ${MESSAGING_DETAIL_SRC})
source_group( functors                  FILES ${FUNCTORS_SRC})
source_group( functors\\detail          FILES ${FUNCTORS_DETAIL_SRC})
source_group( serialization             FILES ${SERIALIZATION_SRC})
source_group( serialization\\thrift     FILES ${SERIALIZATION_THRIFT_SRC})
source_group( serialization\\boost      FILES ${SERIALIZATION_BOOST_SRC})
source_group( exceptions                FILES ${EXCEPTIONS_SRC})
source_group( tools                     FILES ${TOOLS_SRC})
source_group( transport                 FILES ${TRANSPORT_SRC})
source_group( transport\\detail         FILES ${TRANSPORT_DETAIL_SRC})
source_group( transport\\detail\network FILES ${TRANSPORT_NETWORK_SRC})
source_group( signature                 FILES ${SIGNATURE_SRC})
source_group( signature\\detail         FILES ${SIGNATURE_DETAIL_SRC})
source_group( log                       FILES ${LOGS_SRC})

file(GLOB HEADERS qi/*.h*)

set(SRC
    ${CORE_SRC}
    ${MESSAGING_SRC}
    ${MESSAGING_DETAIL_SRC}
    ${SERIALIZATION_SRC}
    ${SERIALIZATION_BOOST_SRC}
    ${EXCEPTIONS_SRC}
    ${TOOLS_SRC}
    ${TRANSPORT_SRC}
    ${TRANSPORT_NETWORK_SRC}
    ${FUNCTORS_SRC}
    ${FUNCTORS_DETAIL_SRC}
    ${SIGNATURE_SRC}
    ${SIGNATURE_DETAIL_SRC}
    ${LOGS_SRC}
    ${CBINDINGS_SRC}
)

if(WITH_ZMQ)
  set(SRC ${SRC} ${TRANSPORT_ZMQ_SRC})
  source_group( transport\\detail\\zmq      FILES ${TRANSPORT_ZMQ_SRC})
endif()

if (WITH_THRIFT)
  set(SRC ${SRC} ${SERIALIZATION_THRIFT_SRC} ${TRANSPORT_THRIFT_SRC})
  source_group( transport\\detail\\thrift        FILES ${TRANSPORT_THRIFT_SRC})
  source_group( transport\\serialization\\thrift FILES ${SERIALIZATION_THRIFT_SRC})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif (UNIX)

create_lib(qi "${SRC}" "${HEADERS}")
use_lib (qi REQUIRED ALL BOOST)
use_lib (qi BOOST_SERIALIZATION BOOST_DATE_TIME BOOST_THREAD ZEROMQ PTHREAD)
use_lib (qi PROTOBUF)
if (WITH_THRIFT)
  use_lib(qi THRIFT)
endif()

add_subdirectory(qi/exceptions)
add_subdirectory(qi/serialization)
add_subdirectory(qi/transport)

install_header(qi SUBFOLDER qi ${HEADERS})

stage_lib (qi QI ${CMAKE_CURRENT_SOURCE_DIR})
if (UNIX)
  #add_definitions("-fvisibility=hidden -fvisibility-inlines-hidden")
endif(UNIX)

add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(bin)
create_doxygen("Doxyfile")

add_subdirectory(bindings/python)
