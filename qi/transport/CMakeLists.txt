##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

set(headers
  clientbase.hpp
  serverbase.hpp
  transport.hpp
)

install_header(LIBIPPC SUBFOLDER qi/transport/ ${headers})

cond_subdirectory("detail/zmq"  WITH_ZMQ)