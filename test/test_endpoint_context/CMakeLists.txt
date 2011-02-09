##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_add_gtest(test_endpoint_context
             SRC          test_endpoint_context.cpp
             DEPENDS QI ZEROMQ GTEST
             TIMEOUT      120)
             
qi_add_gtest(test_negotiate_endpoint
             SRC          test_negotiate_endpoint.cpp
             DEPENDS QI ZEROMQ GTEST
             TIMEOUT      120)
