###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(test-serialization-protobuf)

include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)
create_gtest(test_protobuf_serialization SRC main.cpp DEPENDENCIES GTEST ALCOMMON-NG PROTOBUF)
