
add_subdirectory(test_messaging)
add_subdirectory(test_messaging_publish)
#add_subdirectory(test_transport)
#add_subdirectory(test_transport_publish)

# This is not a test
add_subdirectory(test_c_bindings)
#add_subdirectory(test_endpoint_context)

qi_create_gtest(test_functor            SRC test_functor.cpp            DEPENDS QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_message_visitor    SRC test_message_visitor.cpp    DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_serializable       SRC test_serializable.cpp       DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_signature          SRC test_signature.cpp          DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_signature_qt       SRC test_signature_qt.cpp       DEPENDS QI QIMESSAGING GTEST QT_QTCORE TIMEOUT 120)
qi_create_gtest(test_signature_iterator SRC test_signature_iterator.cpp DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
