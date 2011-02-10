

find_package(Protobuf)

protobuf_generate_cpp(PROTO_SRC PROTO_H alvalue.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
#use(PROTOBUF-TOOLS)
#tc_wrap_proto(PROTO_SRC alvalue.proto)

qi_create_gtest(test_messaging
 SRC test_messaging.cpp ${PROTO_SRC} ${PROTO_H}
 DEPENDS QI GTEST
 TIMEOUT 120)

qi_create_gtest(test_messaging_stress
 SRC test_messaging_stress.cpp
 DEPENDS QI GTEST
 TIMEOUT 120)

qi_create_gtest(test_messaging_signatures
 SRC test_messaging_signatures.cpp ${PROTO_SRC} ${PROTO_H}
 DEPENDS QI GTEST
 TIMEOUT 120)

#qi_create_gtest(test_messaging_naoqi
# SRC test_messaging_naoqi.cpp
# DEPENDS QI GTEST
# TIMEOUT 120)