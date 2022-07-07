find_package(idl REQUIRED)

qi_create_client_lib(task_client_support CLASSES TaskGenerator)
add_library(qi::task_client_support ALIAS task_client_support)

add_library(task MODULE)
target_sources(
  task
  PRIVATE task_service.cpp
)
target_link_libraries(task PUBLIC qi::task_client_support qi::qi)

qi_create_test(test_task
  SRC test_task.cpp
  DEPENDS 
    qi::task_client_support
    qi::qi
    qi::testsession
  TIMEOUT 10
)

if(IS_GCC_OR_CLANG)
  set_target_properties(
    test_task
    PROPERTIES LINK_FLAGS "-Wl,-no-as-needed"
  )
endif()
