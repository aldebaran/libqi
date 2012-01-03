## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.
qi_create_bin(testlaunch NO_INSTALL testlaunch.cpp)
qi_create_bin(check_env  NO_INSTALL check_env.cpp)

qi_use_lib(check_env QI)

set_target_properties(testlaunch PROPERTIES FOLDER tests)
set_target_properties(check_env  PROPERTIES FOLDER tests)

qi_create_gtest(test_qipath      SRC test_qipath.cpp      DEPENDS QI GTEST)
qi_create_gtest(test_qilocal     SRC test_locale.cpp      DEPENDS QI GTEST)
qi_create_gtest(test_qios        SRC test_qios.cpp        DEPENDS QI GTEST)
qi_create_gtest(test_thread      SRC test_thread.cpp      DEPENDS QI BOOST_THREAD)
qi_create_gtest(test_qilaunch    SRC test_qilaunch.cpp    DEPENDS QI GTEST)
qi_create_gtest(test_qilog_sync  SRC test_qilog_sync.cpp  DEPENDS QI GTEST)
qi_create_gtest(test_qilog_async SRC test_qilog_async.cpp DEPENDS QI GTEST)
