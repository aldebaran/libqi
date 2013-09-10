
qi_create_gtest(test_value                SRC test_value.cpp                 DEPENDS QI QITYPE GTEST TIMEOUT 10)
qi_create_gtest(test_event                SRC test_event.cpp                DEPENDS QI QITYPE GTEST TIMEOUT 10)
qi_create_gtest(test_object               SRC test_object.cpp               DEPENDS QI QITYPE GTEST TIMEOUT 10)
qi_create_gtest(test_signature            SRC test_signature.cpp            DEPENDS QI QITYPE GTEST TIMEOUT 10)
qi_create_gtest(test_object_eventloop     SRC test_object_eventloop.cpp     DEPENDS QI QITYPE GTEST TIMEOUT 10)
qi_create_gtest(test_signal               SRC test_signal.cpp               DEPENDS QI QITYPE GTEST TIMEOUT 10)
qi_create_gtest(test_json                 SRC test_json.cpp                 DEPENDS QI QITYPE GTEST TIMEOUT 10)
