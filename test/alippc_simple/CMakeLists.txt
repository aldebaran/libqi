##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

create_gtest(alippc_simple_tests
             SRC          alippc_simple_tests.cpp
             DEPENDENCIES LIBIPPC
             TIMEOUT      120)
