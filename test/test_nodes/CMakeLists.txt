
create_gtest(test_nodes
 SRC test_nodes.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(test_nodes_stress
 SRC test_nodes_stress.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)

create_gtest(test_nodes_signatures
 SRC test_nodes_signatures.cpp
 DEPENDENCIES QI GTEST
 TIMEOUT 120)
