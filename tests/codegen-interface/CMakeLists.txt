find_package(idl REQUIRED)

#generate interface header from idl
qi_create_interface(${CMAKE_CURRENT_SOURCE_DIR}/adder.idl
  qi::Adder
  ${CMAKE_CURRENT_BINARY_DIR} _interfacefile
  )

#build the service, implementing interface
include_directories(${CMAKE_CURRENT_BINARY_DIR})
qi_create_lib(adder_interface
  MODULE
  SRC adder.cpp ${_interfacefile}
  DEPENDS QITYPE
)


#build the client
qimessaging_create_session_test(test_adder_interface SRC test_adder.cpp DEPENDS QI QITYPE QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
