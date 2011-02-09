##
## Copyright (C) 2010 Aldebaran Robotics
##

###############################################
# Auto-generated file.                        #
# Do not edit                                 #
# This file is part of the qibuild project    #
###############################################

set(QIBUILD_BOOTSTRAP_VERSION 5)


##
# Create a use_qibuild.cmake file in CMAKE_BINARY_DIR,
# and include it.
# This allow us to find all qibuild/qibuild.cmake
function(bootstrap)
  find_program(PYTHON_EXECUTABLE NAMES python2 python python.exe)
  find_program(QI_BUILD_EXECUTABLE qibuild.py)

  if(NOT PYTHON_EXECUTABLE)
    message(STATUS
      "
      Could not find python executable.

      Please check your setup.

      "
    )
    message(FATAL_ERROR "")
  endif()

  if(NOT QI_BUILD_EXECUTABLE)
    message(STATUS
      "
      Could not find qibuild executable.

      Please check your setup.

      "
    )
    message(FATAL_ERROR "")
  endif()
  set(_cmd ${PYTHON_EXECUTABLE} ${QI_BUILD_EXECUTABLE})
  set(_cmd ${_cmd} configure --single --bootstrap "--build-directory=${CMAKE_BINARY_DIR}")
  execute_process(
      COMMAND ${_cmd}
      RESULT_VARIABLE  _retcode
      OUTPUT_VARIABLE  _stdout
      ERROR_VARIABLE   _stderr
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
  if(NOT ${_retcode} EQUAL 0)
    message(STATUS
        "
        qibuild bootstrap fail!
        Log:
        ====
        ${_cmd}
        ${_stdout}
        ${_stderr}

        ===

        Please file a bug report with a comple CMake log
        "
    )
      message(FATAL_ERROR "")
  endif()
endfunction()

if(NOT EXISTS ${CMAKE_BINARY_DIR}/dependencies.cmake)
  bootstrap()
endif()

if(EXISTS ${CMAKE_BINARY_DIR}/dependencies.cmake)
  include(${CMAKE_BINARY_DIR}/dependencies.cmake)
else()
  message(STATUS "can't find dependencies.cmake")
endif()

include(qibuild/general)

