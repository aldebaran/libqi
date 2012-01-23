##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011, 2012 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(qi-framework)
find_package(qibuild)

add_definitions(" -DBOOST_FILESYSTEM_VERSION=3 ")

enable_testing()
add_subdirectory("tinyxml")
add_subdirectory("libqimessaging")
