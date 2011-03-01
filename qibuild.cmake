## Copyright (C) 2011 Aldebaran Robotics

###############################################
# Auto-generated file.                        #
# Do not edit                                 #
# This file is part of the qibuild project    #
###############################################

set(QIBUILD_BOOTSTRAP_VERSION 8)


##
# Create a use_qibuild.cmake file in CMAKE_BINARY_DIR,
# and include it.
# This allow us to find all qibuild/qibuild.cmake
function(bootstrap)
  find_program(PYTHON_EXECUTABLE NAMES python2 python python.exe
    PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.6\\InstallPath]
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.7\\InstallPath]
  )


  if(NOT PYTHON_EXECUTABLE)
    message(STATUS
      "
      Could not find python executable.

      Please check your setup.

      "
    )
    message(FATAL_ERROR "")
  endif()

  get_filename_component(_python_root ${PYTHON_EXECUTABLE} PATH)
  find_program(QI_BUILD_EXECUTABLE NAMES qibuild.py qibuild
    PATHS
      "${_python_root}/Scripts"
  )

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


include(qibuild/general OPTIONAL RESULT_VARIABLE _qibuild_found)
if(_qibuild_found)
  return()
endif()

if(NOT EXISTS ${CMAKE_BINARY_DIR}/dependencies.cmake)
  bootstrap()
endif()

include(${CMAKE_BINARY_DIR}/dependencies.cmake)


include(qibuild/general)
