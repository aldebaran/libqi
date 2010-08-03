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
  zmqserver.hpp
)

install_header(LIBIPPC SUBFOLDER alippc/transport/zeromq ${headers})

