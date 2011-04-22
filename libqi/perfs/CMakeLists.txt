##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

qi_create_gtest(perf_maps    SRC perf_maps.cpp    DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(perf_functor SRC perf_functor.cpp DEPENDS QI GTEST TIMEOUT 120)
