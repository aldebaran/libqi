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

install_header(LIBIPPC SUBFOLDER alcommon-ng/transport/ ${headers})

add_subdirectory("common")
add_subdirectory("shm")
add_subdirectory("zeromq")
