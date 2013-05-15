
# Follow the user use case:
# The user writes an idl file: we provide it
# The user generates a service skeleton by invoking idl.py: we do exactly that
# The user fills in the skeleton: we do that with string replace
# The user builds a library with the filled skeleton: we do that

#find python
find_program(_python_executable
    NAMES python2 python python.exe
    NO_CMAKE_FIND_ROOT_PATH)
  if (NOT _python_executable)
    qi_error("needs python executable in PATH")
  endif()

file(MAKE_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/simpleservice)

#generate proxy header from idl
execute_process(COMMAND
  ${_python_executable} ${CMAKE_CURRENT_SOURCE_DIR}/../../src/idl.py
  ${CMAKE_CURRENT_SOURCE_DIR}/simpleservice.idl
  -m proxy
  -o ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-proxy.hpp
)

#generate service skeleton from idl

set(skel ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.skel)
execute_process(COMMAND
  ${_python_executable} ${CMAKE_CURRENT_SOURCE_DIR}/../../src/idl.py
  ${CMAKE_CURRENT_SOURCE_DIR}/simpleservice.idl
  -m cxxserviceregister
  -o ${skel}
)

# Implement the method in simpleservice-service
# here the user edits the skeleton file, we use 'sed'
file(READ ${skel} _fcontent)
STRING(REPLACE "// Implementation of addOne" "return p0+1;" _seded "${_fcontent}")
file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.cpp
"${_seded}")

#now build our service
qi_create_lib(simpleservice
  MODULE
  SRC ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.cpp
  DEPENDS qitype
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_gtest(test_load_service SRC test_load_service.cpp DEPENDS QI QITYPE GTEST TIMEOUT 10)


