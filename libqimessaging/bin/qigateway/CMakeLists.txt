##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (QiGateway)

qi_add_optional_package(LIBEVENT)

if (WITH_LIBEVENT)
  qi_create_bin(qi-gateway
    qigateway.cpp
    gateway.cpp gateway.hpp
    master.cpp master.hpp
    daemon.cpp daemon.h
    DEPENDS
      QI
      QIMESSAGING
      BOOST_PROGRAM_OPTIONS
      LIBEVENT
  )
endif ()
