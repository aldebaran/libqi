###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(test-serialization-thrift)

create_bin(test_thrift_serialization main.cpp NO_INSTALL)
use_lib(test_thrift_serialization QI THRIFT)

