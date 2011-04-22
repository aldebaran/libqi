

qi_create_gtest(test_functor            SRC test_functor.cpp            DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_log                SRC test_log.cpp                DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_message_visitor    SRC test_message_visitor.cpp    DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_serializable       SRC test_serializable.cpp       DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_signature          SRC test_signature.cpp          DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_signature_iterator SRC test_signature_iterator.cpp DEPENDS QI GTEST TIMEOUT 120)
