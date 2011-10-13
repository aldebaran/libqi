##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (QiGateway)

qi_create_bin(qi-gateway
  qigateway.cpp
  server.cpp server.hpp
  DEPENDS
    QI
    BOOST_PROGRAM_OPTIONS
)

