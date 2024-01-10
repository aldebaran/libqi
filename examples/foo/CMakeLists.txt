qi_add_module(foo EXCLUDE_FROM_ALL foo.cpp)

add_executable(example_foo EXCLUDE_FROM_ALL footest.cpp)
target_link_libraries(example_foo PRIVATE qi::qi)
add_dependencies(example_foo foo)

add_dependencies(examples example_foo)
