## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.
qi_create_test_helper(testlaunch         testlaunch.cpp)
qi_create_test_helper(testlaunchloop     testlaunchloop.cpp   DEPENDS QI)
qi_create_test_helper(test_launchloop_with_spaces testlaunchloop.cpp   DEPENDS QI)
if(QI_WITH_TESTS)
  set_property(TARGET test_launchloop_with_spaces PROPERTY OUTPUT_NAME "test launchloop with spaces")
endif()

qi_create_test_helper(check_env          check_env.cpp        DEPENDS QI)
qi_create_test_helper(check_machineid    check_machineid.cpp  DEPENDS QI)

qi_create_gtest(
  test_qi

  SRC
  "test_bind.cpp"
  "test_buffer.cpp"
  "test_bufferreader.cpp"
  "test_eventloop.cpp"
  "test_future.hpp"
  "test_future.cpp"
  "test_futuregroup.cpp"
  "test_locale.cpp"
  "test_macro.cpp"
  "test_path_conf.cpp"
  "test_periodictask.cpp"
  "test_qi.cpp" # main
  "test_qiatomic.cpp"
  "test_qiclock.cpp"
  "test_qilaunch.cpp"
  "test_qilog_async.cpp"
  "test_qilog_sync.cpp"
  "test_qios.cpp"
  "test_strand.cpp"
  "test_trackable.cpp"
  "test_version.cpp"
  "test_algorithm.cpp"
  "test_conceptpredicates.cpp"
  "test_macroregular.cpp"

  DEPENDS
  qi

  TIMEOUT 120
)

qi_create_gtest(test_qipath SRC "test_qipath.cpp" "../../src/utils.cpp" DEPENDS qi)

# test with the default chrono io, which is v1 in boost 1.55
qi_create_gtest(test_qiclock_chronoio SRC test_qiclock_chronoio.cpp DEPENDS QI GTEST)
# test with the chrono io v2.
# commented out since it does not compile
#qi_create_gtest(test_qiclock_chronoio_v2 SRC test_qiclock_chronoio.cpp DEPENDS QI GTEST)
if(TARGET test_qiclock_chronoio_v2)
  set_target_properties(test_qiclock_chronoio_v2 PROPERTIES
      COMPILE_DEFINITIONS "BOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0")
endif()
