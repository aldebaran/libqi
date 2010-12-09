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

create_bin(example_qi_advertise NO_INSTALL example_qi_advertise.cpp)
use_lib(example_qi_advertise QI)

create_bin(example_qi_signature_type NO_INSTALL example_qi_signature_type.cpp)
use_lib(example_qi_signature_type QI)

create_bin(example_qi_signature_instance NO_INSTALL example_qi_signature_instance.cpp)
use_lib(example_qi_signature_instance QI)

create_bin(example_qi_signature_pp NO_INSTALL example_qi_signature_pp.cpp)
use_lib(example_qi_signature_pp QI)

create_bin(example_qi_signature_function NO_INSTALL example_qi_signature_function.cpp)
use_lib(example_qi_signature_function QI)
