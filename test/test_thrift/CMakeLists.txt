###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(testippcthrift)
include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)

create_bin(testippcthrift main.cpp NO_INSTALL)

use_lib(testippcthrift LIBIPPC THRIFT)

