## Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

add_executable(example_translate EXCLUDE_FROM_ALL main.cpp)
target_link_libraries(example_translate PRIVATE qi::qi)
