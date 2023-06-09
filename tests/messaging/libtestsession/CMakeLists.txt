##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

function(qimessaging_create_session_test name)
  cmake_parse_arguments(ARG "" "TIMEOUT" "ARGUMENTS;SRC;DEPENDS" ${ARGN})

  add_executable("${name}")
  target_sources("${name}" PRIVATE ${ARG_SRC})
  target_link_libraries("${name}" PRIVATE ${ARG_DEPENDS} GTest::gmock)
  add_test(NAME "${name}_direct" COMMAND "${name}" --mode=direct ${ARG_ARGUMENTS})
  add_test(NAME "${name}_sd" COMMAND "${name}" --mode=sd ${ARG_ARGUMENTS})
  add_test(NAME "${name}_gateway" COMMAND "${name}" --mode=gateway ${ARG_ARGUMENTS})
  add_test(NAME "${name}_ssl" COMMAND "${name}" --mode=ssl ${ARG_ARGUMENTS})
  #add_test(NAME "${name}_nightmare" COMMAND "${name}" --mode=nightmare ${ARG_ARGUMENTS})

  math(EXPR ARG_TIMEOUT_SSL "${ARG_TIMEOUT} * 2")
  set_tests_properties(
    "${name}_direct"
    "${name}_sd"
    "${name}_gateway"
    PROPERTIES
      TIMEOUT "${ARG_TIMEOUT}"
  )
  set_tests_properties(
    "${name}_ssl"
    PROPERTIES
      TIMEOUT "${ARG_TIMEOUT_SSL}"
  )
endfunction()

qi_create_test(
  test_testsession

  test/test_testsession.cpp

  DEPENDS
  qi::qi
  qi::testsession

  TIMEOUT 30
)
