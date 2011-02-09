
find_package(Protobuf)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRC PROTO_HDR alvalue.proto)

qi_add_gtest(perf_messaging
 SRC perf_messaging.cpp ${PROTO_SRC} ${PROTO_HDR}
 DEPENDS QI GTEST
 TIMEOUT 120)

qi_add_gtest(perf_messaging_stress
 SRC perf_messaging_stress.cpp
 DEPENDS QI GTEST
 TIMEOUT 120)

qi_add_gtest(perf_messaging_signatures
 SRC perf_messaging_signatures.cpp ${PROTO_SRC} ${PROTO_HDR}
 DEPENDS QI GTEST
 TIMEOUT 120)

qi_add_gtest(perf_messaging_publish
 SRC perf_messaging_publish.cpp ${PROTO_SRC} ${PROTO_HDR}
 DEPENDS QI GTEST
 TIMEOUT 120)

tc_wrap_proto(PROTO_F_SRC vfloat.proto)

create_gtest(perf_messaging_serialization
 SRC perf_messaging_serialization.cpp ${PROTO_F_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)
