include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_create_gtest(test_preference
 SRC test_preference.cpp
 DEPENDS QI QIMESSAGING GTEST
 TIMEOUT 120)

