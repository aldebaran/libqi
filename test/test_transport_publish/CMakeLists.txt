##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_transport_publish
             SRC          test_transport_publish.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
