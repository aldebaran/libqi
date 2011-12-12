create_bin(testlaunch NO_INSTALL testlaunch.cpp)
create_bin(check_env NO_INSTALL check_env.cpp)
qi_use_lib(check_env QI)
set_target_properties(testlaunch PROPERTIES FOLDER tests)
set_target_properties(check_env  PROPERTIES FOLDER tests)

create_gtest(test_qipath      SRC test_qipath.cpp      DEPENDENCIES QI GTEST)
#create_gtest(test_qipaththrow SRC test_qipaththrow.cpp DEPENDENCIES QI GTEST)
create_gtest(test_qiproject   SRC test_qiproject.cpp   DEPENDENCIES QI GTEST BOOST_FILESYSTEM)
create_gtest(test_qios        SRC test_qios.cpp        DEPENDENCIES QI GTEST)
create_gtest(test_thread      SRC test_thread.cpp      DEPENDENCIES QI BOOST_THREAD)
create_gtest(test_qilaunch    SRC test_qilaunch.cpp DEPENDENCIES QI GTEST)
create_gtest(test_qilog_sync        SRC test_qilog_sync.cpp DEPENDENCIES QI GTEST)
create_gtest(test_qilog_async       SRC test_qilog_async.cpp DEPENDENCIES QI GTEST)

#create_gtest(test_qilog       SRC test_log.cpp      DEPENDENCIES QI GTEST RTTOOLS ALLOGREMOTE)
#should it work?
#create_gtest(test_qilocale SRC test_locale.cpp   DEPENDENCIES QI GTEST)
