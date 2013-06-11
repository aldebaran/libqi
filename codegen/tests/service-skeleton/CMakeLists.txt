
# Follow the user use case:
# The user writes an idl file: we provide it
# The user generates a service skeleton by invoking idl.py: we do exactly that
# The user fills in the skeleton: we do that with string replace
# The user builds a library with the filled skeleton: we do that


find_package(idl REQUIRED)
find_package(idl.py REQUIRED)

find_program(_python_executable
    NAMES python2 python python.exe
    NO_CMAKE_FIND_ROOT_PATH)
  if (NOT _python_executable)
    qi_error("needs python executable in PATH")
  endif()

file(MAKE_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/simpleservice)

qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/simpleservice.idl
  SimpleService ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/ _simpleservice_proxy)


#generate service skeleton from idl

set(skel ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.skel)
set(filled_skel ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.cpp)

qi_generate_src(${skel}
  SRC ${IDL.PY_EXECUTABLE}
  COMMAND ${_python_executable} ${IDL.PY_EXECUTABLE}
   ${CMAKE_CURRENT_SOURCE_DIR}/simpleservice.idl
  -m cxxserviceregister
  -o ${skel}
)


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/fill-skel.cmake
  "file(READ ${skel} _fcontent)
  STRING(REPLACE \"// Implementation of addOne\" \"return p0+1;\" _seded \"\${_fcontent}\")
  file(WRITE
    ${CMAKE_CURRENT_BINARY_DIR}/simpleservice/simpleservice-service.cpp
    \"\${_seded}\"
  )"
)

qi_generate_src(${filled_skel}
  SRC ${skel}
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/fill-skel.cmake
)


#now build our service
qi_create_lib(simpleservice
  MODULE
  SRC ${filled_skel} ${_simpleservice_proxy}
  DEPENDS qitype
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_gtest(test_load_service SRC test_load_service.cpp DEPENDS QI QITYPE GTEST TIMEOUT 10)


