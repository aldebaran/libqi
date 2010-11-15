##
## Author(s):
##  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
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

create_config_h("config.hpp.in" "qi/config.hpp")

file(GLOB NODES_SRC                qi/nodes/*.cpp qi/nodes/*.h*)
file(GLOB NODES_DETAIL_SRC         qi/nodes/detail/*.cpp qi/nodes/detail/*.h*)
file(GLOB COLLECTIONS_SRC          qi/collections/*.cpp qi/collections/*.h*)
file(GLOB SERIALIZATION_SRC        qi/serialization/*.cpp qi/serialization/*.h*)
file(GLOB SERIALIZATION_THRIFT_SRC qi/serialization/thrift/*.cpp qi/serialization/thrift/*.h*)
file(GLOB SERIALIZATION_BOOST_SRC  qi/serialization/boost/*.cpp qi/serialization/boost/*.h*)

file(GLOB TOOLS_SRC                qi/perf/*.cpp qi/perf/*.h*)
file(GLOB LOGS_SRC                 qi/log/*.cpp qi/log/*.h* qi/log.hpp)
file(GLOB EXCEPTIONS_SRC           qi/exceptions/*.cpp qi/exceptions/*.h*)

file(GLOB MESSAGING_SRC            qi/messaging/*.cpp qi/messaging/*.h*)

file(GLOB TRANSPORT_SRC                   qi/transport/*.cpp qi/transport/*.h*)
file(GLOB TRANSPORT_COMMON_SRC            qi/transport/common/*.cpp qi/transport/common/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_UTIL_SRC   qi/transport/shm/util/*.cpp qi/transport/shm/util/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_CLIENT_SRC qi/transport/shm/client/*.cpp qi/transport/shm/client/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_MEMORY_SRC qi/transport/shm/memory/*.cpp qi/transport/shm/memory/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_SERVER_SRC qi/transport/shm/server/*.cpp qi/transport/shm/server/*.h*)
file(GLOB TRANSPORT_ZMQ_SRC               qi/transport/zeromq/*.cpp qi/transport/zeromq/*.h*)
file(GLOB TRANSPORT_THRIFT_SRC            qi/transport/thrift/*.cpp qi/transport/thrift/*.h*)
file(GLOB FUNCTORS_SRC                    qi/functors/*.cpp qi/functors/*.h*)
file(GLOB FUNCTORS_DETAIL_SRC             qi/functors/detail/*.h*)
file(GLOB SIGNATURE_SRC                   qi/signature/*.cpp qi/signature/*.h*)
file(GLOB SIGNATURE_DETAIL_SRC            qi/signature/detail/*.h*)

source_group( nodes                  FILES ${NODES_SRC})
source_group( nodes\\detail          FILES ${NODES_DETAIL_SRC})
source_group( messaging              FILES ${MESSAGING_SRC})
source_group( collections            FILES ${COLLECTIONS_SRC})
source_group( functors               FILES ${FUNCTORS_SRC})
source_group( functors\\detail       FILES ${FUNCTORS_DETAIL_SRC})
source_group( serialization          FILES ${SERIALIZATION_SRC})
source_group( serialization\\thrift  FILES ${SERIALIZATION_THRIFT_SRC})
source_group( serialization\\boost   FILES ${SERIALIZATION_BOOST_SRC})
source_group( exceptions             FILES ${EXCEPTIONS_SRC})
source_group( tools                  FILES ${TOOLS_SRC})
source_group( transport              FILES ${TRANSPORT_SRC})
source_group( transport\\common      FILES ${TRANSPORT_COMMON_SRC})
source_group( signature              FILES ${SIGNATURE_SRC})
source_group( signature\\detail      FILES ${SIGNATURE_DETAIL_SRC})
source_group( log                    FILES ${LOGS_SRC})

file(GLOB HEADERS qi/*.h*)

set(SRC
    ${NODES_SRC}
    ${NODES_DETAIL_SRC}
    ${MESSAGING_SRC}
    ${COLLECTIONS_SRC}
    ${SERIALIZATION_SRC}
    ${SERIALIZATION_BOOST_SRC}
    ${EXCEPTIONS_SRC}
    ${TOOLS_SRC}
    ${TRANSPORT_SRC}
    ${TRANSPORT_COMMON_SRC}
    ${FUNCTORS_SRC}
    ${FUNCTORS_DETAIL_SRC}
    ${SIGNATURE_SRC}
    ${SIGNATURE_DETAIL_SRC}
    ${LOGS_SRC}
)

if(WITH_ZMQ)
  set(SRC ${SRC} ${TRANSPORT_ZMQ_SRC})
  source_group( transport\\zeromq      FILES ${TRANSPORT_ZMQ_SRC})
endif()

if(WITH_SHM)
  set(SRC ${SRC} ${TRANSPORT_COMMON_SHM_UTIL_SRC}
                 ${TRANSPORT_COMMON_SHM_CLIENT_SRC}
                 ${TRANSPORT_COMMON_SHM_MEMORY_SRC}
                 ${TRANSPORT_COMMON_SHM_SERVER_SRC})

  source_group( transport\\shm\\util   FILES ${TRANSPORT_COMMON_SHM_UTIL_SRC})
  source_group( transport\\shm\\client FILES ${TRANSPORT_COMMON_SHM_CLIENT_SRC})
  source_group( transport\\shm\\memory FILES ${TRANSPORT_COMMON_SHM_MEMORY_SRC})
  source_group( transport\\shm\\server FILES ${TRANSPORT_COMMON_SHM_SERVER_SRC})
endif()


if (WITH_THRIFT)
  set(SRC ${SRC} ${SERIALIZATION_THRIFT_SRC} ${TRANSPORT_THRIFT_SRC})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif (UNIX)

create_lib(qi "${SRC}" "${HEADERS}")
use_lib (qi REQUIRED ALL BOOST)
use_lib (qi BOOST_SERIALIZATION BOOST_DATE_TIME BOOST_THREAD ZEROMQ PTHREAD)
if (WITH_SHM)
use_lib(qi ALVALUE SHMPOOL LIBCORE)
endif()

if (WITH_THRIFT)
  use_lib(qi THRIFT)
endif()

add_subdirectory(qi/collections)
add_subdirectory(qi/exceptions)
add_subdirectory(qi/serialization)
add_subdirectory(qi/transport)

install_header(qi SUBFOLDER qi ${HEADERS})

stage_lib (qi QI ${CMAKE_CURRENT_SOURCE_DIR})
if (UNIX)
  #add_definitions("-fvisibility=hidden -fvisibility-inlines-hidden")
endif(UNIX)

cond_subdirectory(qi/test BUILD_TESTS)

create_doxygen("Doxyfile")
