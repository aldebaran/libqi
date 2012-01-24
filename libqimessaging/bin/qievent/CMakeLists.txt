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
    network-thread.cpp
    transport-client.cpp
    main-client.cpp
    )
  set(H_CLIENT
    network-thread.hpp
    transport-client.hpp
    )

  set(C_SERVER
    network-thread.cpp
    transport-server.cpp
    main-server.cpp
    )
  set(H_SERVER
    network-thread.hpp
    transport-server.hpp
    )

  set(C_GATEWAY
    network-thread.cpp
    transport-server.cpp
    transport-client.cpp
    gateway.cpp
    main-gateway.cpp
    )
  set(H_GATEWAY
    network-thread.hpp
    transport-server.hpp
    transport-client.hpp
    gateway.hpp
    )

  qi_create_bin(qi-event-tcp-client ${H_CLIENT} ${C_CLIENT})
  qi_create_bin(qi-event-tcp-server ${H_SERVER} ${C_SERVER})
  qi_create_bin(qi-event-gateway ${H_GATEWAY} ${C_GATEWAY})

  qi_use_lib(qi-event-tcp-client BOOST_THREAD QI LIBEVENT)
  qi_use_lib(qi-event-tcp-server BOOST_THREAD QI LIBEVENT)
  qi_use_lib(qi-event-gateway QI LIBEVENT)
endif ()
