qi_create_gtest(test_c_bindings SRC test_c_bindings.cpp DEPENDS QIC GTEST TIMEOUT 10)
qi_create_gtest(test_c_future SRC test_c_future.cpp DEPENDS QIC GTEST TIMEOUT 10)
qi_create_gtest(test_c_signal SRC test_c_signal.cpp DEPENDS QIC GTEST TIMEOUT 10)
