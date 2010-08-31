##
## Author(s):
##  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##


project (ALCommon-NG)

cmake_minimum_required(VERSION 2.6.4)
include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/options.cmake)
add_definitions(" -D_SCL_SECURE_NO_WARNINGS ")

create_config_h("config.hpp.in" "alcommon-ng/config.hpp")

file(GLOB COMMON_SRC               common/*.cpp common/*.h*)
file(GLOB COMMON_DETAIL_SRC        common/detail/*.cpp common/detail/*.h*)
file(GLOB COLLECTIONS_SRC          collections/*.cpp collections/*.h*)
file(GLOB SERIALIZATION_SRC        serialization/*.cpp serialization/*.h*)
file(GLOB SERIALIZATION_THRIFT_SRC serialization/thrift/*.cpp serialization/thrift/*.h*)
file(GLOB SERIALIZATION_BOOST_SRC  serialization/boost/*.cpp serialization/boost/*.h*)

file(GLOB TOOLS_SRC                tools/*.cpp tools/*.h*)
file(GLOB EXCEPTIONS_SRC           exceptions/*.cpp exceptions/*.h*)

file(GLOB MESSAGING_SRC            messaging/*.cpp messaging/*.h*)

file(GLOB TRANSPORT_SRC                   transport/*.cpp transport/*.h*)
file(GLOB TRANSPORT_COMMON_SRC            transport/common/*.cpp transport/common/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_UTIL_SRC   transport/shm/util/*.cpp transport/shm/util/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_CLIENT_SRC transport/shm/client/*.cpp transport/shm/client/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_MEMORY_SRC transport/shm/memory/*.cpp transport/shm/memory/*.h*)
file(GLOB TRANSPORT_COMMON_SHM_SERVER_SRC transport/shm/server/*.cpp transport/shm/server/*.h*)
file(GLOB TRANSPORT_ZMQ_SRC               transport/zeromq/*.cpp transport/zeromq/*.h*)
file(GLOB TRANSPORT_THRIFT_SRC            transport/thrift/*.cpp transport/thrift/*.h*)
file(GLOB FUNCTOR_SRC                     functor/*.cpp functor/*.h*)

source_group( Common                 FILES ${COMMON_SRC})
source_group( Common\\Detail         FILES ${COMMON_DETAIL_SRC})
source_group( Messaging              FILES ${MESSAGING_SRC})
source_group( Collections            FILES ${COLLECTIONS_SRC})
source_group( Functor                FILES ${FUNCTOR_SRC})
source_group( Serialization          FILES ${SERIALIZATION_SRC})
source_group( Serialization\\Thrift  FILES ${SERIALIZATION_THRIFT_SRC})
source_group( Serialization\\Boost   FILES ${SERIALIZATION_BOOST_SRC})
source_group( Utils                  FILES ${UTIL_SRC})
source_group( Exceptions             FILES ${EXCEPTIONS_SRC})
source_group( Tools                  FILES ${TOOLS_SRC})
source_group( Transport              FILES ${TRANSPORT_SRC})
source_group( Transport\\Common      FILES ${TRANSPORT_COMMON_SRC})


set(HEADERS
  ippc.hpp
  win_stdint.hpp
)
source_group( Headers FILES ${HEADERS})


set(SRC
    ${COMMON_SRC}
    ${COMMON_DETAIL_SRC}
    ${MESSAGING_SRC}
    ${COLLECTIONS_SRC}
    ${SERIALIZATION_SRC}
    ${SERIALIZATION_BOOST_SRC}
    ${EXCEPTIONS_SRC}
    ${TOOLS_SRC}
    ${TRANSPORT_SRC}
    ${TRANSPORT_COMMON_SRC}
    ${FUNCTOR_SRC}
)

if(WITH_ZMQ)
  set(SRC ${SRC} ${TRANSPORT_ZMQ_SRC})
  source_group( Transport\\ZeroMq      FILES ${TRANSPORT_ZMQ_SRC})
endif()

if(WITH_SHM)
  set(SRC ${SRC} ${TRANSPORT_COMMON_SHM_UTIL_SRC}
                 ${TRANSPORT_COMMON_SHM_CLIENT_SRC}
                 ${TRANSPORT_COMMON_SHM_MEMORY_SRC}
                 ${TRANSPORT_COMMON_SHM_SERVER_SRC})

  source_group( Transport\\Shm\\Util   FILES ${TRANSPORT_COMMON_SHM_UTIL_SRC})
  source_group( Transport\\Shm\\Client FILES ${TRANSPORT_COMMON_SHM_CLIENT_SRC})
  source_group( Transport\\Shm\\Memory FILES ${TRANSPORT_COMMON_SHM_MEMORY_SRC})
  source_group( Transport\\Shm\\Server FILES ${TRANSPORT_COMMON_SHM_SERVER_SRC})
endif()


if (WITH_THRIFT)
  set(SRC ${SRC} ${SERIALIZATION_THRIFT_SRC} ${TRANSPORT_THRIFT_SRC})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif (UNIX)

create_static_and_shared_libs(alcommon-ng  "${SRC}" "${HEADERS}")
use_lib (alcommon-ng REQUIRED ALL BOOST)
use_lib (alcommon-ng BOOST_SERIALIZATION BOOST_DATE_TIME ALVALUE LIBCORE LIBTHREAD SHMPOOL ZEROMQ RTTOOLS)

if (WITH_THRIFT)
  use_lib(alcommon-ng THRIFT)
endif()

add_subdirectory(collections)
add_subdirectory(exceptions)
add_subdirectory(serialization)
add_subdirectory(transport)

install_header(ALCOMMON-NG SUBFOLDER alippc ${HEADERS})

stage_lib (alcommon-ng ALCOMMON-NG  ${CMAKE_CURRENT_SOURCE_DIR}/..)
cond_subdirectory(test BUILD_TESTS)
