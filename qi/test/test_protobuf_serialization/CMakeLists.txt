###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(test-serialization-protobuf)

use(PROTOBUF-TOOLS)

include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)
tc_wrap_proto(PROTO_SRC myproto.proto)

create_gtest(test_protobuf_serialization SRC main.cpp ${PROTO_SRC} DEPENDENCIES GTEST QI PROTOBUF)



