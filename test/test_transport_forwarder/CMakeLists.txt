##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_transport_forwarder
             SRC          test_transport_forwarder.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
