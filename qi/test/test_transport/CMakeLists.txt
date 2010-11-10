###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(test-transport)


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
  create_gtest(test_transport_zmqsingle SRC test_transport.cpp DEPENDENCIES GTEST QI)
  create_gtest(test_transport_zmqqueue  SRC test_transport.cpp DEPENDENCIES GTEST QI)
  create_gtest(test_transport_zmqpoll   SRC test_transport.cpp DEPENDENCIES GTEST QI)
  tc_add_target_definition(test_transport_zmqsingle_bin "TEST_TRANSPORT_ZMQ_SINGLE")
  tc_add_target_definition(test_transport_zmqqueue_bin  "TEST_TRANSPORT_ZMQ_QUEUE")
  tc_add_target_definition(test_transport_zmqpoll_bin   "TEST_TRANSPORT_ZMQ_POLL")
endif()

if (WITH_SHM)
  create_gtest(test_transport_shm       SRC test_transport.cpp DEPENDENCIES GTEST QI)
  tc_add_target_definition(test_transport_shm_bin       "TEST_TRANSPORT_SHM")
endif()
