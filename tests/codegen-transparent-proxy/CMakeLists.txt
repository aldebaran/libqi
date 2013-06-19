find_package(idl REQUIRED)

# Shared task interface
qi_create_interface(${CMAKE_CURRENT_SOURCE_DIR}/task.idl
  Task
  ${CMAKE_CURRENT_BINARY_DIR} _itask
  )

# Internal service interface (to ensure implem compliance to idl)
qi_create_interface(${CMAKE_CURRENT_SOURCE_DIR}/taskservice.idl
  TaskService
  ${CMAKE_CURRENT_BINARY_DIR} _itaskservice
  )

# Task proxy in transparent mode (to allow AnyObject->ITaskPtr conversion)
qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/task.idl
  Task
  ${CMAKE_CURRENT_BINARY_DIR}
  _itaskproxy INTERFACE)

# TaskService Proxy in standard mode, to be used by test client
qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/taskservice.idl
  TaskService
  ${CMAKE_CURRENT_BINARY_DIR}
  _taskserviceproxy)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(generated
  ${_itask}
  ${_itaskservice}
  ${_itaskproxy}
  ${_taskserviceproxy}
)

qimessaging_create_session_test(test_transparent_proxy
  SRC test_client.cpp service.cpp ${generated}
  DEPENDS QI QITYPE QIMESSAGING GTEST TESTSESSION TIMEOUT 10)