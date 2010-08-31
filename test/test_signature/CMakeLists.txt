##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(test_signature
             SRC          test_signature.cpp
             DEPENDENCIES ALCOMMON-NG GTEST
             TIMEOUT      120)
