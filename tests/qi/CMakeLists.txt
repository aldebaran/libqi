## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.
qi_create_test_helper(testlaunch         testlaunch.cpp)
qi_create_test_helper(testlaunchloop     testlaunchloop.cpp   DEPENDS QI)
qi_create_test_helper(check_env          check_env.cpp        DEPENDS QI)
qi_create_test_helper(check_machineid    check_machineid.cpp  DEPENDS QI)
qi_create_test_helper(test_application   test_application.cpp DEPENDS QI)

qi_create_gtest(test_path_conf    SRC test_path_conf.cpp ../../src/path_conf.cpp DEPENDS QI GTEST)
qi_create_gtest(test_qipath       SRC test_qipath.cpp    ../../src/utils.cpp     DEPENDS QI GTEST)
qi_create_gtest(test_thread       SRC test_thread.cpp       DEPENDS QI BOOST_THREAD)
qi_create_gtest(test_qiclock      SRC test_qiclock.cpp      DEPENDS QI GTEST)
# test with the default chrono io, which is v1 in boost 1.55
qi_create_gtest(test_qiclock_chronoio SRC test_qiclock_chronoio.cpp DEPENDS QI GTEST)
# test with the chrono io v2.
# commented out since it does not compile
#qi_create_gtest(test_qiclock_chronoio_v2 SRC test_qiclock_chronoio.cpp DEPENDS QI GTEST)
if(TARGET test_qiclock_chronoio_v2)
  set_target_properties(test_qiclock_chronoio_v2 PROPERTIES
      COMPILE_DEFINITIONS "BOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0")
endif()
qi_create_gtest(test_qilocal      SRC test_locale.cpp       DEPENDS QI GTEST)
qi_create_gtest(test_qios         SRC test_qios.cpp         DEPENDS QI GTEST)
qi_create_gtest(test_qiatomic     SRC test_qiatomic.cpp     DEPENDS QI GTEST)
qi_create_gtest(test_qilaunch     SRC test_qilaunch.cpp     DEPENDS QI GTEST)
qi_create_gtest(test_qilog_sync   SRC test_qilog_sync.cpp   DEPENDS QI GTEST)
qi_create_gtest(test_qilog_async  SRC test_qilog_async.cpp  DEPENDS QI GTEST)
qi_create_gtest(test_strand       SRC test_strand.cpp       DEPENDS QI GTEST)
qi_create_gtest(test_future       SRC test_future.cpp       DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_futuregroup  SRC test_futuregroup.cpp  DEPENDS QI GTEST TIMEOUT)
qi_create_gtest(test_buffer       SRC test_buffer.cpp       DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_bufferreader SRC test_bufferreader.cpp DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_version      SRC test_version.cpp      DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_macro        SRC test_macro.cpp        DEPENDS QI GTEST TIMEOUT 30)
