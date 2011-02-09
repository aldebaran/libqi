##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_add_gtest(test_functor
             SRC          test_functor.cpp
             DEPENDS QI GTEST
             TIMEOUT      120)
