## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.

project(qi_examples)

qi_create_bin(qipath_example qipath_example.cpp NO_INSTALL)
qi_use_lib(qipath_example QI)
set_target_properties(qipath_example PROPERTIES FOLDER "examples")

qi_create_bin(sharedlibrary_example sharedlibrary_example.cpp NO_INSTALL)
qi_use_lib(sharedlibrary_example QI)
set_target_properties(sharedlibrary_example PROPERTIES FOLDER "examples")

qi_create_bin(log_example log_example.cpp NO_INSTALL)
qi_use_lib(log_example QI BOOST_PROGRAM_OPTIONS PTHREAD)
set_target_properties(log_example PROPERTIES FOLDER "examples")

if (UNIX AND WITH_BOOST_LOCALE)
  qi_create_bin(qiconvloc_example qiconvloc_example.cpp NO_INSTALL)
  qi_use_lib(qiconvloc_example QI BOOST_LOCALE)
  set_target_properties(qiconvloc_example PROPERTIES FOLDER "examples")
endif()
