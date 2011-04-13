##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

set(headers
  thirft_client.cpp
  thirft_server.hpp
)

install_header(LIBIPPC SUBFOLDER qi/transport/detail/thrift ${headers})

