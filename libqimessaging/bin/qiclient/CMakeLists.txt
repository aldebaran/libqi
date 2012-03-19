##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (QiClient)

qi_create_bin(qi-client
  qimain.cpp
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

