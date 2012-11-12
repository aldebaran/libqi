
#add_subdirectory(test_messaging)
#add_subdirectory(test_messaging_publish)
#add_subdirectory(test_transport)
#add_subdirectory(test_transport_publish)

#add_subdirectory(test_endpoint_context)
add_subdirectory(libtestsession)

qi_create_lib(testmodule  SRC testmodule.cpp SHARED DEPENDS QI QIMESSAGING SUBFOLDER naoqi)

qi_create_gtest(test_module               SRC test_module.cpp               DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_url                  SRC test_url.cpp                  DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
#qi_create_gtest(test_message_visitor     SRC test_message_visitor.cpp      DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_serializable         SRC test_serializable.cpp         DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
#Not working yet
#qi_create_gtest(test_value               SRC test_value.cpp                DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_binarycoder          SRC test_binarycoder.cpp           DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_without_gateway      SRC test_without_gateway.cpp      DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
#qi_create_gtest(test_with_gateway        SRC test_with_gateway.cpp         DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_file                 SRC test_file.cpp                 DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
# `qibuild test` on mac never returns with those tests.
qi_create_session_test(test_event_remote         SRC test_event_remote.cpp         DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qi_create_gtest(test_event_connect        SRC test_event_connect.cpp        DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_session_test(test_event_remote_connect SRC test_event_remote_connect.cpp DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qi_create_session_test(test_call_many            SRC test_call_many.cpp            DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qi_create_session_test(test_session              SRC test_session.cpp              DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 30)
qi_create_session_test(test_session_harder       SRC test_session_harder.cpp       DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qi_create_session_test(test_call                 SRC test_call.cpp                 DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qi_create_gtest(test_metavalue_argument   SRC test_metavalue_argument.cpp   DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_testsession          SRC test_testsession.cpp          DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 2)
qi_create_gtest(test_gateway              SRC test_gateway.cpp              DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)

qi_create_gtest(test_service              SRC test_service.cpp DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)

#qi_create_gtest(test_serialization_qt   SRC test_serialization_qt.cpp   DEPENDS QI QIMESSAGING GTEST QT_QTCORE TIMEOUT 120)
#qi_create_gtest(test_signature_qt       SRC test_signature_qt.cpp       DEPENDS QI QIMESSAGING GTEST QT_QTCORE TIMEOUT 120)
