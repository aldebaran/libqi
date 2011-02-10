##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_create_gtest(perf_functor
                SRC          perf_functor.cpp
                DEPENDS      QI GTEST
                TIMEOUT      120)
