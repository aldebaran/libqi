## Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING file.
qi_create_test_helper(testlaunch         testlaunch.cpp)
qi_create_test_helper(testlaunchloop     testlaunchloop.cpp)
qi_create_test_helper(test_launchloop_with_spaces testlaunchloop.cpp)
set_property(TARGET test_launchloop_with_spaces PROPERTY OUTPUT_NAME "test launchloop with spaces")

qi_create_test_helper(check_env             check_env.cpp             DEPENDS qi::qi)
qi_create_test_helper(check_machineid       check_machineid.cpp       DEPENDS qi::qi)
qi_create_test_helper(print_process_uuid    print_process_uuid.cpp    DEPENDS qi::qi)

qi_create_test(
  test_qi

  SRC
  "test_application.cpp"
  "test_bind.cpp"
  "test_buffer.cpp"
  "test_bufferreader.cpp"
  "test_either.cpp"
  "test_errorhandling.cpp"
  "test_eventloop.cpp"
  "test_flags.cpp"
  "test_functional.cpp"
  "test_future.hpp"
  "test_future.cpp"
  "test_futuregroup.cpp"
  "test_locale.cpp"
  "test_numeric.cpp"
  "test_macro.cpp"
  "test_mutablestore.cpp"
  "test_path_conf.cpp"
  "test_periodictask.cpp"
  "test_ptruid.cpp"
  "test_objectuid.cpp"
  "test_qi.cpp" # main
  "test_qiatomic.cpp"
  "test_qiclock.cpp"
  "test_qilaunch.cpp"
  "test_qilog.hpp"
  "test_qilog.cpp"
  "test_qilog_async.cpp"
  "test_qilog_sync.cpp"
  "test_qilog_sync_invokecatch.cpp"
  "test_qios.cpp"
  "test_src.cpp"
  "test_strand.cpp"
  "test_trackable.cpp"
  "test_version.cpp"

  DEPENDS
  qi::qi
  qi::testutils
  GTest::gmock

  TIMEOUT 120
)

# TODO: Merge helptext tests in one program once Application can be constructed
#       and destroyed multiple times in the same process.
qi_create_test(test_helptext
  SRC
    "test_helptext.hpp"
    "test_helptext.cpp"
  DEPENDS qi::qi
  TIMEOUT 120
)

qi_create_test(test_helptext_session
  SRC
    "test_helptext.hpp"
    "test_helptext_session.cpp"
  DEPENDS qi::qi
  TIMEOUT 120
)

qi_create_test(periodictask_always_die_gracefully
  test_periodictask_kill.cpp
ARGUMENTS
  --qi-standalone
TIMEOUT
  10
DEPENDS
  qi::qi
)

# Links with qi_objects because it needs internal symbols.
qi_create_test(test_qipath SRC "test_qipath.cpp" DEPENDS qi::internal::qi_objects)

# test with the default chrono io, which is v1 in boost 1.55
qi_create_test(test_qiclock_chronoio SRC test_qiclock_chronoio.cpp DEPENDS qi::qi GTest::gtest_main)
