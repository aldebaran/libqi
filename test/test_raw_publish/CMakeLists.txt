##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_raw_publish
             SRC          test_raw_publish.cpp
             DEPENDENCIES ALCOMMON-NG ZEROMQ GTEST
             TIMEOUT      120)

create_gtest(test_raw_subscribe
             SRC          test_raw_subscribe.cpp
             DEPENDENCIES ALCOMMON-NG ZEROMQ GTEST
             TIMEOUT      120)
