
create_gtest(test_boost_mq
 SRC test_boost_mq.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(test_boost_mq_pub
 SRC test_boost_mq_pub.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)
