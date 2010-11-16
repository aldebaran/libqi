##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_process_identity
             SRC          test_process_identity.cpp
             DEPENDENCIES QI ZEROMQ GTEST
             TIMEOUT      120)
