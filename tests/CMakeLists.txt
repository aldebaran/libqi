if (MSVC)
  if (NOT CMAKE_BUILD_TOOL STREQUAL "nmake")
    set_source_files_properties(qipath_test.cpp PROPERTIES COMPILE_DEFINITIONS "REALLY_MSVC")
  endif()
endif()

create_gtest(test_qipath      SRC qipath_test.cpp      DEPENDENCIES QI GTEST)
create_gtest(test_qipaththrow SRC qipaththrow_test.cpp DEPENDENCIES QI GTEST)
create_gtest(test_qios        SRC qios_test.cpp        DEPENDENCIES QI GTEST)
create_bin(testlaunch NO_INSTALL testlaunch.cpp)
set_target_properties(testlaunch PROPERTIES FOLDER tests)
create_gtest(test_qilaunch    SRC qilaunch_test.cpp DEPENDENCIES QI GTEST)
#create_gtest(test_qilog       SRC test_log.cpp      DEPENDENCIES QI GTEST RTTOOLS ALLOGREMOTE)

#should it work?
#create_gtest(test_qilocale SRC test_locale.cpp   DEPENDENCIES QI GTEST)
