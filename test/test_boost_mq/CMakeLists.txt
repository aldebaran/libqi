
create_gtest(test_boost_mq
 SRC test_boost_mq.cpp
 DEPENDENCIES ALCOMMON-NG GTEST
 TIMEOUT 120)

create_gtest(test_boost_mq_pub
 SRC test_boost_mq_pub.cpp
 DEPENDENCIES ALCOMMON-NG GTEST
 TIMEOUT 120)
