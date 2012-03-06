###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(perf-transport)


function("tc_add_target_definition" target property)
  set(_prev_compdef)
  get_target_property(_prev_compdef ${target} COMPILE_DEFINITIONS)
  if (NOT _prev_compdef)
    set(_prev_compdef "")
  endif()
  set_target_properties(${target} PROPERTIES COMPILE_DEFINITIONS "${_prev_compdef} ${property}")
endfunction()

#qi_create_gtest(perf_transport       SRC ../dataperftimer.* perf_transport.cpp       DEPENDS GTEST QI QIMESSAGING  )
#qi_create_gtest(perf_transport_event SRC ../dataperftimer.* perf_transport_event.cpp DEPENDS GTEST QI QIMESSAGING  )
