
use(PROTOBUF-TOOLS)
tc_wrap_proto(PROTO_SRC alvalue.proto)

create_gtest(test_messaging
 SRC test_messaging.cpp ${PROTO_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(test_messaging_stress
 SRC test_messaging_stress.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(test_messaging_signatures
 SRC test_messaging_signatures.cpp ${PROTO_SRC}
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

