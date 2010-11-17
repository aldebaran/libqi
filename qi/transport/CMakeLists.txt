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

add_subdirectory("common")
cond_subdirectory("zeromq"  WITH_ZMQ)
