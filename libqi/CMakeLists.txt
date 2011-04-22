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

add_subdirectory("cpp")
add_subdirectory("tests")
add_subdirectory("perfs")
