
use(PROTOBUF-TOOLS)
tc_wrap_proto(PROTO_SRC alvalue.proto)

create_gtest(perf_messaging
 SRC perf_messaging.cpp ${PROTO_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(perf_messaging_stress
 SRC perf_messaging_stress.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(perf_messaging_signatures
 SRC perf_messaging_signatures.cpp ${PROTO_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)
 
create_gtest(perf_messaging_publish
 SRC perf_messaging_publish.cpp ${PROTO_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

tc_wrap_proto(PROTO_F_SRC vfloat.proto)

create_gtest(perf_messaging_serialization
 SRC perf_messaging_serialization.cpp ${PROTO_F_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)
