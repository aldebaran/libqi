##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_add_gtest(test_transport_publish
             SRC          test_transport_publish.cpp
             DEPENDS QI ZEROMQ GTEST
             TIMEOUT      120)
