##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_endpoint_context
             SRC          test_endpoint_context.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
             
create_gtest(test_negotiate_endpoint
             SRC          test_negotiate_endpoint.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
