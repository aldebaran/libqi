##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_create_gtest(zmq_raw_publish
             SRC          test_raw_publish.cpp
             DEPENDS QI ZEROMQ GTEST
             TIMEOUT      120)

qi_create_gtest(zmq_raw_subscribe
             SRC          test_raw_subscribe.cpp
             DEPENDS QI ZEROMQ GTEST
             TIMEOUT      120)
