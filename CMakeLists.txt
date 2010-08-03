##
## Author(s):
##  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (ALCommon-NG)

FILE(GLOB COLLECTIONS_SRC collections/*.cpp collections/*.h*)
FILE(GLOB SERIALIZATION_SRC serialization/*.cpp serialization/*.h*)
FILE(GLOB UTIL_SRC util/*.cpp util/*.h*)
FILE(GLOB EXCEPTIONS_SRC exceptions/*.cpp exceptions/*.h*)

FILE(GLOB TRANSPORT_SRC transport/*.cpp transport/*.h*)
FILE(GLOB TRANSPORT_COMMON_SRC transport/common/*.cpp transport/common/*.h*)
FILE(GLOB TRANSPORT_COMMON_SHM_UTIL_SRC transport/shm/util/*.cpp transport/shm/util/*.h*)
FILE(GLOB TRANSPORT_COMMON_SHM_CLIENT_SRC transport/shm/client/*.cpp transport/shm/client/*.h*)
FILE(GLOB TRANSPORT_COMMON_SHM_MEMORY_SRC transport/shm/memory/*.cpp transport/shm/memory/*.h*)
FILE(GLOB TRANSPORT_COMMON_SHM_SERVER_SRC transport/shm/server/*.cpp transport/shm/server/*.h*)
file(GLOB TRANSPORT_ZMQ_SRC transport/zeromq/*.cpp transport/zeromq/*.h*)

SOURCE_GROUP( Collections   FILES ${COLLECTIONS_SRC})
SOURCE_GROUP( Serialization FILES ${SERIALIZATION_SRC})
SOURCE_GROUP( Utils         FILES ${UTIL_SRC})
SOURCE_GROUP( Exceptions    FILES ${EXCEPTIONS_SRC})

SOURCE_GROUP( Transport              FILES ${TRANSPORT_SRC})
SOURCE_GROUP( Transport\\Common      FILES ${TRANSPORT_COMMON_SRC})
SOURCE_GROUP( Transport\\Shm\\Util   FILES ${TRANSPORT_COMMON_SHM_UTIL_SRC})
SOURCE_GROUP( Transport\\Shm\\Client FILES ${TRANSPORT_COMMON_SHM_CLIENT_SRC})
SOURCE_GROUP( Transport\\Shm\\Memory FILES ${TRANSPORT_COMMON_SHM_MEMORY_SRC})
SOURCE_GROUP( Transport\\Shm\\Server FILES ${TRANSPORT_COMMON_SHM_SERVER_SRC})
SOURCE_GROUP( Transport\\ZeroMq      FILES ${TRANSPORT_ZMQ_SRC})

set(HEADERS
  ippc.hpp
  win_stdint.hpp
)
SOURCE_GROUP( Headers FILES ${HEADERS})

SET( SRC
    ${COLLECTIONS_SRC}
    ${SERIALIZATION_SRC}
    ${UTIL_SRC}
    ${TRANSPORT_SRC}
    ${TRANSPORT_COMMON_SRC}
    ${TRANSPORT_COMMON_SHM_UTIL_SRC}
    ${TRANSPORT_COMMON_SHM_CLIENT_SRC}
    ${TRANSPORT_COMMON_SHM_MEMORY_SRC}
    ${TRANSPORT_COMMON_SHM_SERVER_SRC}
    ${TRANSPORT_ZMQ_SRC}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif (UNIX)

add_definitions(" -D_SCL_SECURE_NO_WARNINGS ")
create_static_and_shared_libs(alippc  "${SRC}" "${HEADERS}")
use_lib (alippc REQUIRED ALL BOOST)
use_lib (alippc BOOST_SERIALIZATION BOOST_DATE_TIME ALVALUE LIBCORE LIBTHREAD SHMPOOL ZEROMQ)

add_subdirectory(collections)
add_subdirectory(exceptions)
add_subdirectory(serialization)
add_subdirectory(transport)

install_header(LIBIPPC SUBFOLDER alippc ${HEADERS})

stage_lib (alippc LIBIPPC  ${CMAKE_CURRENT_SOURCE_DIR}/..)
cond_subdirectory(test BUILD_TESTS)
