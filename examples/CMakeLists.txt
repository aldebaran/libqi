##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

qi_create_bin(example_qi_client             NO_INSTALL example_qi_client.cpp             DEPENDS QI)
qi_create_bin(example_qi_server             NO_INSTALL example_qi_server.cpp             DEPENDS QI)
qi_create_bin(example_qi_publisher          NO_INSTALL example_qi_publisher.cpp          DEPENDS QI)
qi_create_bin(example_qi_subscriber         NO_INSTALL example_qi_subscriber.cpp         DEPENDS QI)
qi_create_bin(example_qi_signature_type     NO_INSTALL example_qi_signature_type.cpp     DEPENDS QI)
qi_create_bin(example_qi_signature_instance NO_INSTALL example_qi_signature_instance.cpp DEPENDS QI)
qi_create_bin(example_qi_signature_pp       NO_INSTALL example_qi_signature_pp.cpp       DEPENDS QI)
qi_create_bin(example_qi_signature_function NO_INSTALL example_qi_signature_function.cpp DEPENDS QI)
qi_create_bin(example_qi_signature_iterator NO_INSTALL example_qi_signature_iterator.cpp DEPENDS QI)
