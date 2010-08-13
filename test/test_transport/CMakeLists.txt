###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(test-transport)

include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)
create_bin(test_transport main.cpp NO_INSTALL)
use_lib(test_transport ALCOMMON-NG)
