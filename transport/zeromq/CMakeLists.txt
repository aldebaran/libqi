##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

set(headers
  zeromq.hpp
  zmqclient.hpp
  zmqconnection_handler.hpp
  zmqserver.cpp
  zmqserver.hpp
  zmqsimpleserver.hpp
  zmqsimpleserver.cpp
)

install_header(LIBIPPC SUBFOLDER alcommon-ng/transport/zeromq ${headers})
