##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_messaging
             SRC          test_messaging.cpp
             DEPENDENCIES QI
             TIMEOUT      120)
