find_package(idl REQUIRED)


qi_create_interface(${CMAKE_CURRENT_SOURCE_DIR}/adder.idl
  Adder
  ${CMAKE_CURRENT_BINARY_DIR} _interfacefile
  )

#generate binder
qi_create_binder(
  _bind0
  IDL ${CMAKE_CURRENT_SOURCE_DIR}/adder.idl
  DIR ${CMAKE_CURRENT_BINARY_DIR}
  NAME Adder
  CLASS_NAME Adder0
  FACTORY
  )
qi_create_binder(
  _bind1
  IDL ${CMAKE_CURRENT_SOURCE_DIR}/adder.idl
  DIR ${CMAKE_CURRENT_BINARY_DIR}
  NAME Adder
  CLASS_NAME Adder1
  INTERFACE
  FACTORY
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_lib(adder_bind
  MODULE
  SRC adder.cpp ${_bind0} ${_bind1} ${_interfacefile}
  DEPENDS QITYPE
)

qimessaging_create_session_test(test_adder_bind SRC test_adder.cpp DEPENDS QI QITYPE QIMESSAGING GTEST TESTSESSION TIMEOUT 10)

