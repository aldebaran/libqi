find_package(idl REQUIRED)

qi_create_client_lib(task_client_support CLASSES TaskGenerator)
qi_stage_lib(task_client_support)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_lib(task
  MODULE
  SRC task_service.cpp
  DEPENDS task_client_support QI)

qi_create_gtest(test_task
  SRC test_task.cpp
  DEPENDS task_client_support QI TESTSESSION GTEST TIMEOUT 10
)

if(NOT WIN32)
set_target_properties(test_task PROPERTIES
  LINK_FLAGS "-Wl,-no-as-needed")
endif()
