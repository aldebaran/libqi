##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_add_gtest(test_messaging
             SRC          test_messaging.cpp
             DEPENDS QI
             TIMEOUT      120)
