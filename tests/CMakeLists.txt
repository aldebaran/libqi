
#add_subdirectory(test_messaging)
#add_subdirectory(test_messaging_publish)
#add_subdirectory(test_transport)
#add_subdirectory(test_transport_publish)

#add_subdirectory(test_endpoint_context)
add_subdirectory(libtestsession)

qi_create_lib(testmodule  SRC testmodule.cpp SHARED DEPENDS QI QIMESSAGING SUBFOLDER naoqi NO_INSTALL)

qi_create_gtest(test_module               SRC test_module.cpp               DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_url                  SRC test_url.cpp                  DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
#qi_create_gtest(test_message_visitor     SRC test_message_visitor.cpp      DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
#Not working yet
#qi_create_gtest(test_value               SRC test_value.cpp                DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
qi_create_gtest(test_binarycoder          SRC test_binarycoder.cpp           DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_without_gateway      SRC test_without_gateway.cpp      DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
#qi_create_gtest(test_with_gateway        SRC test_with_gateway.cpp         DEPENDS QI QIMESSAGING GTEST TIMEOUT 120)
# `qibuild test` on mac never returns with those tests.
qimessaging_create_session_test(test_event_remote         SRC test_event_remote.cpp         DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qi_create_gtest(test_event_connect        SRC test_event_connect.cpp        DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qimessaging_create_session_test(test_event_remote_connect SRC test_event_remote_connect.cpp DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 25)
qimessaging_create_session_test(test_call_many            SRC test_call_many.cpp            DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qimessaging_create_session_test(test_session              SRC test_session.cpp              DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_session_harder       SRC test_session_harder.cpp       DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qimessaging_create_session_test(test_call                 SRC test_call.cpp                 DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 20)
#qi_create_gtest(test_application          SRC test_application.cpp          DEPENDS QI QIMESSAGING GTEST TIMEOUT 2)
qi_create_gtest(test_metavalue_argument   SRC test_metavalue_argument.cpp   DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)
qi_create_gtest(test_testsession          SRC test_testsession.cpp          DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 2)
#qi_create_gtest(test_gateway              SRC test_gateway.cpp              DEPENDS QI QIMESSAGING GTEST TIMEOUT 10)

qi_create_bin(test_call_on_close_session SRC test_call_on_close_session.cpp DEPENDS QI QIMESSAGING TESTSESSION BOOST BOOST_THREAD)
qi_create_bin(test_signature SRC sig_generator.cpp sig_generator.h test_signature.cpp DEPENDS TESTSESSION QI QIMESSAGING BOOST)

qimessaging_create_session_test(test_service              SRC test_service.cpp DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qimessaging_create_session_test(test_send_object              SRC test_send_object.cpp DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)
qimessaging_create_session_test(test_proxy           SRC test_proxy.cpp DEPENDS QI QIMESSAGING GTEST TESTSESSION TIMEOUT 10)

add_subdirectory(passobject-fromservice)
add_subdirectory(passobject-fromclient)
add_subdirectory(codegen-interface)
add_subdirectory(codegen-bind)
add_subdirectory(task-service)

