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

add_subdirectory("c")
add_subdirectory("cpp")
add_subdirectory("python")

