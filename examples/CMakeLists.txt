## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

project(qi_examples)

add_executable(example_qipath EXCLUDE_FROM_ALL qipath_example.cpp)
target_link_libraries(example_qipath PRIVATE qi::qi Boost::filesystem)

add_executable(example_sharedlibrary EXCLUDE_FROM_ALL sharedlibrary_example.cpp)
target_link_libraries(example_sharedlibrary PRIVATE qi::qi)

add_executable(example_log EXCLUDE_FROM_ALL log_example.cpp)
target_link_libraries(example_log PRIVATE qi::qi Boost::program_options)

if(UNIX)
  target_link_libraries(example_log PRIVATE Threads::Threads)
endif()

add_executable(example_buffer EXCLUDE_FROM_ALL buffer_example.cpp)
target_link_libraries(example_buffer PRIVATE qi::qi)

add_dependencies(
  examples
    example_qipath
    example_sharedlibrary
    example_log
    example_buffer
)

if (UNIX AND WITH_BOOST_LOCALE)
  add_executable(example_qiconvloc EXCLUDE_FROM_ALL qiconvloc_example.cpp)
  target_link_libraries(example_qiconvloc PRIVATE qi::qi Boost::locale)
  add_dependencies(examples example_qiconvloc)

  add_subdirectory(qitranslate)
endif()

add_subdirectory(foo)
add_subdirectory(perf)
