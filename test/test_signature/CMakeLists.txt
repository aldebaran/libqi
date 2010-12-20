##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

use(PROTOBUF-TOOLS)
tc_wrap_proto(PROTO_SRC alvalue.proto)

create_gtest(test_signature
             SRC          test_signature.cpp ${PROTO_SRC}
             DEPENDENCIES QI GTEST
             TIMEOUT      120)

create_gtest(test_signature_iterator
             SRC          test_signature_iterator.cpp ${PROTO_SRC}
             DEPENDENCIES QI GTEST
             TIMEOUT      120)
