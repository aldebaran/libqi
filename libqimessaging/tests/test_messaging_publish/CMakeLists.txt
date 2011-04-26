##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_create_gtest(test_messaging_publish
             SRC          test_messaging_publish.cpp
             DEPENDS QI ZEROMQ GTEST
             TIMEOUT      120)
