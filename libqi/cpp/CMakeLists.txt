##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(libqi-cpp)

qi_submodule_create("qi-log"             "qi/log/*.h[px][px]" "qi/log.hpp"
                                         "src/log/*.cpp")
qi_submodule_create("qi-functors"        "qi/functors/*.h[px][px]" "qi/functors/detail/*.h[px][px]")

qi_submodule_create("qi-signature"       "qi/signature/*.h[px][px]" "qi/signature.hpp" "qi/signature/detail/*.h[px][px]"
                                         "src/signature/*.cpp")

qi_submodule_create("qi-serialization"   "qi/serialization/*.h[px][px]" "qi/serialization.hpp"
                                         "src/serialization/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

qi_create_lib(qi SRC ${CONFIG_H}
                 SUBMODULE qi-log qi-functors qi-serialization qi-signature
                 DEPENDS BOOST BOOST_DATE_TIME BOOST_THREAD)
qi_stage_lib(qi)
