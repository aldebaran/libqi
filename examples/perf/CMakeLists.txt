##
## Author(s):
##  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

qi_create_perf_test(example_qiperf example_qiperf.cpp
  DEPENDS
    QI QIPERF BOOST_PROGRAM_OPTIONS)
