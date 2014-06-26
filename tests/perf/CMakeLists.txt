qi_create_gtest(test_dataperf         SRC test_dataperf.cpp       DEPENDS QI QIPERF GTEST TIMEOUT 10)
qi_create_gtest(test_measure          SRC test_measure.cpp        DEPENDS QI QIPERF GTEST TIMEOUT 10)
