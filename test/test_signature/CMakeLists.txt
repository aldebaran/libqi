##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

find_package(Protobuf)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRC PROTO_HDR alvalue.proto)

qi_add_gtest(test_signature
             SRC          test_signature.cpp ${PROTO_SRC} ${PROTO_HDR}
             DEPENDS QI GTEST
             TIMEOUT      120)

qi_add_gtest(test_signature_iterator
                SRC          test_signature_iterator.cpp ${PROTO_SRC}
                DEPENDS      QI GTEST
                TIMEOUT      120)
