##
## Author(s):
##  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

set(opt_exclude_from_all EXCLUDE_FROM_ALL)
if(BUILD_TESTING)
  unset(opt_exclude_from_all)
endif()

add_executable(example_qiperf ${opt_exclude_from_all} example_qiperf.cpp)
target_link_libraries(example_qiperf PRIVATE qi::qi Boost::program_options)
add_test(NAME example_qiperf COMMAND example_qiperf)

add_dependencies(examples example_qiperf)
