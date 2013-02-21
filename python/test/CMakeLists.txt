##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics

# Find python, but avoid using python from python package
find_program(_python_executable
  NAMES python2 python python.exe
  NO_CMAKE_FIND_ROOT_PATH)
if (NOT _python_executable)
  qi_error("python executable not found")
endif()

function(qi_add_python_qimessaging_test name)
  set(_test_name test_python_${name})
  qi_add_test(${_test_name}
    ${_python_executable}
      ARGUMENTS
        ${CMAKE_CURRENT_SOURCE_DIR}/run_qimessaging_test.py
        --sdk-dir ${QI_SDK_DIR}
        --src-dir "${CMAKE_CURRENT_SOURCE_DIR}/.."
        ${name}
      ${ARGN}
  )
  if(BUILD_TESTS)
    if(WIN32)
      set_tests_properties(${_test_name} PROPERTIES
            ENVIRONMENT "PATH=${QI_SDK_DIR}/${QI_SDK_BIN}"
          )
    elseif(APPLE)
      set_tests_properties(${_test_name} PROPERTIES
            ENVIRONMENT "DYLD_LIBRARY_PATH=${QI_SDK_DIR}/${QI_SDK_LIB}"
          )
    endif()
  endif()
endfunction()

qi_add_python_qimessaging_test(test_future   TIMEOUT 5)
