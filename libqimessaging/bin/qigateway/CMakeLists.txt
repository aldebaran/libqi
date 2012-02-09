##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Herve CUCHE        <hcuche@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2012 Aldebaran Robotics
##

project (QiGateway)

qi_create_bin(qi-gateway
  qigateway.cpp
  DEPENDS
  QI
  QIMESSAGING
  BOOST_PROGRAM_OPTIONS
  )
