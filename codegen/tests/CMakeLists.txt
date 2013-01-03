
# Follow the user use case.

#find python
find_program(_python_executable
    NAMES python2 python python.exe
    NO_CMAKE_FIND_ROOT_PATH)
  if (NOT _python_executable)
    qi_error("needs python executable in PATH")
  endif()

#generate proxy header from idl
execute_process(COMMAND
  ${_python_executable} ${CMAKE_CURRENT_SOURCE_DIR}/../src/idl.py
  ${CMAKE_CURRENT_SOURCE_DIR}/simpleservice/simpleservice.idl
  -m proxy
  -o ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-proxy.hpp
)

#generate service skeleton from idl

set(skel ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.skel)
execute_process(COMMAND
  ${_python_executable} ${CMAKE_CURRENT_SOURCE_DIR}/../src/idl.py
  ${CMAKE_CURRENT_SOURCE_DIR}/simpleservice/simpleservice.idl
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
  SRC ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.cpp
  DEPENDS qitype
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_gtest(test_load_service SRC test_load_service.cpp DEPENDS QI QITYPE GTEST TIMEOUT 10)


