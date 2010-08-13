##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

set(headers
  thirftclient.cpp
  thirftserver.hpp
)

install_header(LIBIPPC SUBFOLDER alcommon-ng/transport/thrift ${headers})

