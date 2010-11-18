##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##


create_bin(example_qi_client NO_INSTALL example_qi_client.cpp)
use_lib(example_qi_client QI)


create_bin(example_qi_server NO_INSTALL example_qi_server.cpp)
use_lib(example_qi_server QI)
