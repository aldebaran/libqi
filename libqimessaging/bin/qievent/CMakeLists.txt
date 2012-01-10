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
    transport-client.cpp
    main-client.cpp
    )
  set(H_CLIENT
    transport-client.hpp
    )

  set(C_SERVER
    main-server.cpp
    )
  set(H_SERVER)

  qi_create_bin(qi-event-tcp-server ${H_SERVER} ${C_SERVER})
  qi_create_bin(qi-event-tcp-client ${H_CLIENT} ${C_CLIENT})

  qi_use_lib(qi-event-tcp-server QI BOOST_PROGRAM_OPTIONS LIBEVENT)
  qi_use_lib(qi-event-tcp-client BOOST_PROGRAM_OPTIONS BOOST_THREAD QI LIBEVENT)
endif ()
