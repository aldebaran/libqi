qi_create_test(
  test_dataperf
  SRC
    test_dataperf.cpp
  DEPENDS
    qi::qi
    GTest::gtest_main
  TIMEOUT 10
)
qi_create_test(
  test_measure
  SRC
    test_measure.cpp
  DEPENDS
    qi::qi
    GTest::gtest_main
  TIMEOUT 10
)
