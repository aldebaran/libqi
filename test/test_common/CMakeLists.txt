
create_gtest(test_nodes
 SRC test_nodes.cpp
 DEPENDENCIES ALCOMMON-NG GTEST
 TIMEOUT 120)

create_gtest(test_nodes_stress
 SRC test_nodes_stress.cpp
 DEPENDENCIES ALCOMMON-NG GTEST
 TIMEOUT 120)

create_gtest(test_nodes_signatures
 SRC test_nodes_signatures.cpp
 DEPENDENCIES ALCOMMON-NG GTEST
 TIMEOUT 120)
