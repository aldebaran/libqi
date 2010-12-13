###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(perf-transport)


include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)

function("tc_add_target_definition" target property)
  set(_prev_compdef)
  get_target_property(_prev_compdef ${target} COMPILE_DEFINITIONS)
  if (NOT _prev_compdef)
    set(_prev_compdef "")
  endif()
  set_target_properties(${target} PROPERTIES COMPILE_DEFINITIONS "${_prev_compdef} ${property}")
endfunction()


if (WITH_ZMQ)
  create_gtest(perf_transport_zmqsingle SRC perf_transport.cpp DEPENDENCIES GTEST QI)
  create_gtest(perf_transport_zmqqueue  SRC perf_transport.cpp DEPENDENCIES GTEST QI)
  create_gtest(perf_transport_zmqpoll   SRC perf_transport.cpp DEPENDENCIES GTEST QI)
  tc_add_target_definition(perf_transport_zmqsingle_bin "TEST_TRANSPORT_ZMQ_SINGLE")
  tc_add_target_definition(perf_transport_zmqqueue_bin  "TEST_TRANSPORT_ZMQ_QUEUE")
  tc_add_target_definition(perf_transport_zmqpoll_bin   "TEST_TRANSPORT_ZMQ_POLL")

create_gtest(perf_transport_publish
             SRC          perf_transport_publish.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
endif()
