##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011 Aldebaran Robotics
##

project(qi)
cmake_minimum_required(VERSION 2.6.4)

include("qibuild.cmake")
include(CMakeDependentOption)

#find_package(Protobuf)
find_package(ZEROMQ)

cmake_dependent_option(WITH_ZEROMQ   "Use zeromq"   ON "ZEROMQ_FOUND" OFF)
cmake_dependent_option(WITH_PROTOBUF "Use protobuf" OFF "PROTOBUF_FOUND" OFF)
option(WITH_EXAMPLES "Examples"          ON)
option(WITH_PERF     "Performances test" ON)


#include(${CMAKE_CURRENT_SOURCE_DIR}/options.cmake)
if (WIN32)
  add_definitions(" -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN")
endif()

if (UNIX)
 add_definitions(" -fno-strict-aliasing ")
endif()

enable_testing()

qi_create_config_h(CONFIG_H "qi/config.hpp.in" "qi/config.hpp")

include_directories(bindings/c/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(qi)
add_subdirectory(bindings/c)

qi_create_lib(qi SRC ${CONFIG_H}
                 SUBMODULE qi-core qi-exceptions qi-messaging qi-serialization qi-perf qi-log qi-transport
                           qi-functors qi-signature qi-cbinding
                 DEPENDS BOOST BOOST_DATE_TIME BOOST_THREAD)
qi_stage_lib(qi)

add_subdirectory(test)
qi_add_subdirectory(examples WITH_EXAMPLES)
qi_add_subdirectory(perf     WITH_PERF)
add_subdirectory(bin)
#qi_gen_doc(DOXYFILES "Doxyfile")

add_subdirectory(bindings/python)
#add_subdirectory(bindings/ruby)

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_ZEROMQ  : ${WITH_ZEROMQ}")
message(STATUS "WITH_PROTOBUF: ${WITH_PROTOBUF}")
message(STATUS "WITH_EXAMPLES: ${WITH_EXAMPLES}")
message(STATUS "WITH_PERF    : ${WITH_PERF}")
