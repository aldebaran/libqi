##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(zmq_raw_publish
             SRC          raw_raw_publish.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)

create_gtest(zmq_raw_subscribe
             SRC          zmq_raw_subscribe.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
