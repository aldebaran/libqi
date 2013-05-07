find_package(idl REQUIRED)

# PART 1: in service project

# Generate the proxies
qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/adder.idl Adder ${CMAKE_CURRENT_BINARY_DIR} _proxy1)
qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/addtask.idl AddTask ${CMAKE_CURRENT_BINARY_DIR} _proxy2)

# Build the service
qi_create_lib(adder_fromclient
  MODULE
  SRC adder.cpp ${_proxy2}
  DEPENDS qitype
)

# PART 2: in client project
# Generate the client which uses the proxy.

include_directories(${CMAKE_CURRENT_BINARY_DIR})
qi_create_gtest(test_adder_fromclient SRC ${_proxy1} ${_proxy2} test_adder.cpp DEPENDS QI QITYPE QIMESSAGING GTEST TIMEOUT 10)
