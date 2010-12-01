##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_messaging_publish
             SRC          test_messaging_publish.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)

create_gtest(test_messaging_generic_publish
             SRC          test_messaging_generic_publish.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)