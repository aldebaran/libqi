
qi_create_gtest(test_c_bindings  SRC test_c_bindings.c DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_c_signature SRC test_c_signature.c DEPENDS QI GTEST TIMEOUT 120)
