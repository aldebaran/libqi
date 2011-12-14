qi_create_bin(testlaunch NO_INSTALL testlaunch.cpp)
qi_create_bin(check_env NO_INSTALL check_env.cpp)
qi_use_lib(check_env QI)

set_target_properties(testlaunch PROPERTIES FOLDER tests)
set_target_properties(check_env  PROPERTIES FOLDER tests)

qi_create_gtest(test_qipath      SRC test_qipath.cpp      DEPENDS QI GTEST)
#create_gtest(test_qipaththrow SRC test_qipaththrow.cpp DEPENDS QI GTEST)
#create_gtest(test_qiproject   SRC test_qiproject.cpp   DEPENDS QI GTEST BOOST_FILESYSTEM)
qi_create_gtest(test_qios        SRC test_qios.cpp        DEPENDS QI GTEST)
qi_create_gtest(test_thread      SRC test_thread.cpp      DEPENDS QI BOOST_THREAD)
qi_create_gtest(test_qilaunch    SRC test_qilaunch.cpp DEPENDS QI GTEST)
qi_create_gtest(test_qilog_sync        SRC test_qilog_sync.cpp DEPENDS QI GTEST)
qi_create_gtest(test_qilog_async       SRC test_qilog_async.cpp DEPENDS QI GTEST)

#create_gtest(test_qilog       SRC test_log.cpp      DEPENDS QI GTEST RTTOOLS ALLOGREMOTE)
#should it work?
#create_gtest(test_qilocale SRC test_locale.cpp   DEPENDS QI GTEST)
