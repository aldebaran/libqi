##
## Author(s):
##  - Pierre Roullon       <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (QiClient-c)

qi_create_bin(qi-client-c
  qimain.cpp
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

