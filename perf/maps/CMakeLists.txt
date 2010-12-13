##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(perf_maps
             SRC          perf_maps.cpp
             DEPENDENCIES QI GTEST
             TIMEOUT      120)
