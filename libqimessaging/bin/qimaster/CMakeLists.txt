##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2012 Aldebaran Robotics
##

project (QiMaster)

qi_create_bin(qi-master
  qimastermain.cpp
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

