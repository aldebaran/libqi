find_package(idl REQUIRED)

qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/task.idl
  Task
  ${CMAKE_CURRENT_BINARY_DIR}
  _task_proxy)
qi_create_proxy(${CMAKE_CURRENT_SOURCE_DIR}/task.idl
  TaskGenerator
  ${CMAKE_CURRENT_BINARY_DIR}
  _taskgenerator_proxy)

qi_create_binder(
   _task_binder
   IDL ${CMAKE_CURRENT_SOURCE_DIR}/task.idl
   DIR ${CMAKE_CURRENT_BINARY_DIR}
   NAME Task
 )

 qi_create_binder(
   _taskgenerator_binder
   IDL ${CMAKE_CURRENT_SOURCE_DIR}/task.idl
   DIR ${CMAKE_CURRENT_BINARY_DIR}
   NAME TaskGenerator
   SERVICE
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_lib(task
  MODULE
  SRC task_service.cpp ${_task_binder} ${_taskgenerator_binder}
  DEPENDS QITYPE)

qi_create_gtest(test_task
  SRC test_task.cpp ${_task_proxy} ${_taskgenerator_proxy}
  DEPENDS QI QITYPE TESTSESSION GTEST TIMEOUT 10
)

