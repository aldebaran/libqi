qi_create_module(foo foo.cpp NO_INSTALL EXCLUDE_FROM_ALL)

add_executable(example_foo EXCLUDE_FROM_ALL footest.cpp)
target_link_libraries(example_foo PRIVATE qi::qi)
add_dependencies(example_foo foo)

add_dependencies(examples example_foo)
