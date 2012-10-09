##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

include_directories(".")

function(qi_create_session_test name)
  if (DEFINED BUILD_TESTS AND NOT BUILD_TESTS)
    return()
  endif()
  cmake_parse_arguments(ARG "" "TIMEOUT" "ARGUMENTS;SRC;DEPENDS" ${ARGN})
  qi_create_gtest(${name} SRC ${ARG_SRC} DEPENDS ${ARG_DEPENDS} NO_ADD_TEST)
  qi_add_test(${name}_direct ${name} ARGUMENTS --mode=direct ${ARG_ARGUMENTS} TIMEOUT ${ARG_TIMEOUT})
# qi_add_test(${name}_nightmare ${name} ARGUMENTS --mode=nightmare ${ARG_ARGUMENTS} TIMEOUT ${ARG_TIMEOUT})
  qi_add_test(${name}_sd ${name} ARGUMENTS --mode=sd ${ARG_ARGUMENTS} TIMEOUT ${ARG_TIMEOUT})
endfunction()

set(PUBLIC_HEADERS
    testsession/testsession.hpp
    testsession/testsessionpair.hpp
)

qi_install_header(${PUBLIC_HEADERS} KEEP_RELATIVE_PATHS)
qi_create_lib(testsession STATIC SRC ${PUBLIC_HEADERS}
              src/testmode.cpp
              src/testsessionpair.cpp
              src/testsession.cpp
              src/testsession_p.hpp
              src/testsession_p.cpp
              src/testsessionerror.cpp
              src/traficgenerator.hpp
              src/traficgenerator.cpp
              src/populationgenerator.cpp
              src/populationgenerator.cpp
              src/sessioninitializer.hpp
              src/sessioninitializer.cpp
              src/defaultservice.hpp
              src/defaultservice.cpp
              DEPENDS QI QIMESSAGING
)

qi_stage_lib(testsession)
