##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics

# Find python, but avoid using python from python package
find_program(_python_executable
  NAMES python${pyver} python${pyver}.exe python python.exe
  NO_CMAKE_FIND_ROOT_PATH)
if (NOT _python_executable)
  qi_error("python executable not found")
endif()

function(qi_add_python_qimessaging_test name)
  set(_test_name test_python${pyver}_${name})
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
    if("${QIBUILD_VERSION}" VERSION_GREATER "3.1")
      return() # done by default in qibuild 3.2
    endif()
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

qi_add_python_qimessaging_test(test_async   TIMEOUT 5)
qi_add_python_qimessaging_test(test_promise   TIMEOUT 20)
qi_add_python_qimessaging_test(test_signal   TIMEOUT 5)
qi_add_python_qimessaging_test(test_typespassing   TIMEOUT 5)
qi_add_python_qimessaging_test(test_call TIMEOUT 10)
qi_add_python_qimessaging_test(test_applicationsession TIMEOUT 3)

qi_create_lib(testregisterobject${pyver} SHARED SRC testregisterobject.cpp NO_INSTALL)
qi_use_lib(testregisterobject${pyver} QI QITYPE QIMESSAGING)
qi_add_python_qimessaging_test(test_createobject)


if(UNIX AND NOT APPLE)
  # If we link with toolchain python, we must set PYTHONHOME to use .py files
  # from toolchain python and not system.
  # At this point it only works on linux for unknown reasons.
  set(PYTHONHOME "${PYTHON_INCLUDE_DIRS}/../..")
endif()

qi_create_gtest(test_python${pyver}_registerpythonclass SRC registerfactory.cpp
                DEPENDS QIPYTHON${pyver} GTEST
                ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}/test_registerfactory.py" "${CMAKE_CURRENT_SOURCE_DIR}/.." "${QI_SDK_DIR}/${QI_SDK_LIB}" "${PYTHONHOME}")
qi_create_gtest(test_python${pyver}_typepassing SRC test_python_typepassing.cpp
                DEPENDS QIPYTHON${pyver} GTEST
                ARGUMENTS "${QI_SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/.." "${PYTHONHOME}")
