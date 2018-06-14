##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

include_directories(".")

qi_stage_dir(".")
qi_install_data("share/qi/server.crt" SUBFOLDER qi)
qi_install_data("share/qi/server.key" SUBFOLDER qi)

function(qimessaging_create_session_test name)
  if ((DEFINED BUILD_TESTS AND NOT BUILD_TESTS) OR NOT QI_WITH_TESTS)
    return()
  endif()
  cmake_parse_arguments(ARG "" "TIMEOUT" "ARGUMENTS;SRC;DEPENDS" ${ARGN})

  MATH( EXPR ARG_TIMEOUT_SSL "${ARG_TIMEOUT} * 2" )

  qi_create_gmock(${name} SRC ${ARG_SRC} DEPENDS ${ARG_DEPENDS} NO_ADD_TEST)
  qi_add_test(${name}_direct ${name} ARGUMENTS --mode=direct ${ARG_ARGUMENTS}
    TIMEOUT ${ARG_TIMEOUT} GMOCK_TEST)
  #qi_add_test(${name}_nightmare ${name} ARGUMENTS --mode=nightmare ${ARG_ARGUMENTS}
  #  TIMEOUT ${ARG_TIMEOUT} GMOCK_TEST)
  qi_add_test(${name}_sd ${name} ARGUMENTS --mode=sd ${ARG_ARGUMENTS}
    TIMEOUT ${ARG_TIMEOUT} GMOCK_TEST)
  qi_add_test(${name}_gateway ${name} ARGUMENTS --mode=gateway ${ARG_ARGUMENTS}
    TIMEOUT ${ARG_TIMEOUT} GMOCK_TEST)
  qi_add_test(${name}_ssl ${name} ARGUMENTS --mode=ssl ${ARG_ARGUMENTS}
    TIMEOUT ${ARG_TIMEOUT_SSL} GMOCK_TEST)
endfunction()


set(TESTSESSION_H
  testsession/testsession.hpp
  testsession/testsessionpair.hpp
)

set(TESTSESSION_C
  src/testmode.cpp
  src/testsessionpair.cpp
  src/testsession.cpp
  src/testsession_p.hpp
  src/testsession_p.cpp
  src/testsessionerror.cpp
  src/trafficgenerator.hpp
  src/trafficgenerator.cpp
  src/populationgenerator.cpp
  src/populationgenerator.cpp
  src/sessioninitializer.hpp
  src/sessioninitializer.cpp
  src/defaultservice.hpp
  src/defaultservice.cpp
)

qi_install_header(${TESTSESSION_H} KEEP_RELATIVE_PATHS)

qi_create_lib(
  testsession
  STATIC
  ${TESTSESSION_H} ${TESTSESSION_C}
  DEPENDS QI
)

qi_stage_lib(testsession)

if(QI_WITH_TESTS)
  qi_create_gtest(
    test_testsession

    "test/test_testsession.cpp"

    DEPENDS
    qi
    testsession

    TIMEOUT 30
  )
endif()
