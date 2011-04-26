##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(libqi)
include("qibuild.cmake")

qi_submodule_create("qi-log"             "qi/log.hpp"
                                         "src/log/*.[hc][px][px]")

qi_submodule_create("qi-perf"            "qi/perf/*.*")

qi_submodule_create("qi-c"               "qi/*.h" "src/c/*.[hc][px][px]")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (UNIX)
  add_definitions(" -fvisibility=hidden ")
endif()

qi_create_lib(qi SRC ${CONFIG_H}
                 SUBMODULE qi-log qi-c qi-perf
                 DEPENDS BOOST BOOST_DATE_TIME BOOST_THREAD)
qi_stage_lib(qi)


add_subdirectory("tests")
add_subdirectory("perfs")
