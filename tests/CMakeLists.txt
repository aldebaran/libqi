

qi_create_gtest(test_object               SRC test_object.cpp               DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_signature            SRC test_signature.cpp            DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_signature_iterator   SRC test_signature_iterator.cpp   DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_future               SRC test_future.cpp               DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_object_eventloop     SRC test_object_eventloop.cpp     DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_metavalue_argument   SRC test_metavalue_argument.cpp   DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_signal               SRC test_signal.cpp               DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
