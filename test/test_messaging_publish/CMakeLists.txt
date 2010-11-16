##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_messaging_publish
             SRC          test_messaging_publish.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
