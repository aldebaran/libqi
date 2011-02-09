##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

qi_add_gtest(perf_maps
             SRC          perf_maps.cpp
             DEPENDS QI GTEST
             TIMEOUT      120)
