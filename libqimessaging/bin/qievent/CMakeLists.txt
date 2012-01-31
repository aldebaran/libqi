##
## Author(s):
##  - Herve CUCHE  <hcuche@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.8)
project(qieventtcp)

qi_add_optional_package(LIBEVENT)

if (WITH_LIBEVENT)
  set(C_CLIENT
    main-client.cpp
    )
  set(C_SERVER
    main-server.cpp
    )
  set(C_GATEWAY
    main-gateway.cpp
    )


  qi_create_bin(qi-event-tcp-client main-client.cpp)
  qi_create_bin(qi-event-tcp-server main-server.cpp)
  qi_create_bin(qi-event-gateway    main-gateway.cpp)

  qi_use_lib(qi-event-tcp-client BOOST_THREAD QI LIBEVENT QIMESSAGING)
  qi_use_lib(qi-event-tcp-server BOOST_THREAD QI LIBEVENT QIMESSAGING)
  qi_use_lib(qi-event-gateway QI LIBEVENT QIMESSAGING)
endif ()
