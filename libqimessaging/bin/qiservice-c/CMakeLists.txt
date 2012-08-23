##
## Author(s):
##  - Pierre Roullon        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2012 Aldebaran Robotics
##

project (QiService-c)

qi_create_bin(qi-service-c
  qimain.cpp
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

