#add_subdirectory(test_messaging)
#add_subdirectory(test_messaging_publish)
#add_subdirectory(test_transport)
#add_subdirectory(test_transport_publish)

#add_subdirectory(test_endpoint_context)
add_subdirectory(libtestsession)

qi_create_module(naoqi.testanymodule        SRC testanymodule.cpp        SHARED DEPENDS QI NO_INSTALL)
qi_create_module(naoqi.testanymodulesession SRC testanymodulesession.cpp SHARED DEPENDS QI NO_INSTALL)
install(TARGETS naoqi.testanymodule        DESTINATION lib/naoqi COMPONENT test)
install(TARGETS naoqi.testanymodulesession DESTINATION lib/naoqi COMPONENT test)

qi_create_gtest(test_anymodule_service    SRC test_anymodule.cpp DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_url                  SRC test_url.cpp       DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_transportsocketcache SRC test_transportsocketcache.cpp
  ../../src/messaging/messagedispatcher.cpp ../../src/messaging/transportserverasio_p.cpp
  ../../src/messaging/remoteobject.cpp ../../src/messaging/objecthost.cpp
  ../../src/messaging/boundobject.cpp ../../src/messaging/transportsocketcache.cpp
  ../../src/messaging/message.cpp ../../src/messaging/transportserver.cpp
  ../../src/messaging/tcptransportsocket.cpp ../../src/messaging/transportsocket.cpp DEPENDS QI TIMEOUT 30)
#qi_create_gtest(test_message_visitor      SRC test_message_visitor.cpp DEPENDS QI GTEST TIMEOUT 120)
#Not working yet
#qi_create_gtest(test_value                SRC test_value.cpp           DEPENDS QI GTEST TIMEOUT 120)
qi_create_gtest(test_binarycoder          SRC test_binarycoder.cpp     DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_without_gateway      SRC test_without_gateway.cpp DEPENDS QI GTEST TIMEOUT 30)
#qi_create_gtest(test_with_gateway         SRC test_with_gateway.cpp    DEPENDS QI GTEST TIMEOUT 120)
# `qibuild test` on mac never returns with those tests.
qimessaging_create_session_test(test_event_remote         SRC test_event_remote.cpp         DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qi_create_gtest(test_event_connect                        SRC test_event_connect.cpp        DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_sd                                   SRC test_sd.cpp                   DEPENDS QI GTEST TIMEOUT 30)
qimessaging_create_session_test(test_event_remote_connect SRC test_event_remote_connect.cpp DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_call_many            SRC test_call_many.cpp            DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_session              SRC test_session.cpp              DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_session_harder       SRC test_session_harder.cpp       DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_call                 SRC test_call.cpp                 DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
#broken
#qimessaging_create_session_test(test_autoservice  SRC test_autoservice.cpp                   DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
#qi_create_gtest(test_application                  SRC test_application.cpp                   DEPENDS QI GTEST TIMEOUT 2)
#qi_create_gtest(test_gateway                      SRC test_gateway.cpp                       DEPENDS QI GTEST TIMEOUT 10)
qi_create_gtest(test_testsession                  SRC test_testsession.cpp                   DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qi_create_gtest(test_metavalue_argument           SRC test_metavalue_argument.cpp            DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_applicationsession           SRC test_applicationsession.cpp            DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_applicationsessionnoautoexit SRC test_applicationsession_noautoexit.cpp DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_authentication               SRC test_authentication.cpp                DEPENDS QI GTEST TIMEOUT 30)

qi_create_gtest(test_call_on_close_session SRC test_call_on_close_session.cpp              DEPENDS TESTSESSION QI BOOST BOOST_THREAD TIMEOUT 30)
qi_create_gtest(test_signaturegen SRC sig_generator.cpp sig_generator.h test_signature.cpp DEPENDS TESTSESSION QI BOOST)

qimessaging_create_session_test(test_service     SRC test_service.cpp     DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_send_object SRC test_send_object.cpp DEPENDS QI GTEST TESTSESSION TIMEOUT 30)
qimessaging_create_session_test(test_proxy       SRC test_proxy.cpp       DEPENDS QI GTEST TESTSESSION TIMEOUT 30)

# those are idl tests that currently only
# work on linux, and when not cross-compiling
option(DISABLE_CODEGEN "disable the code generation (broken)" ON)

if(UNIX AND NOT APPLE)
  if (NOT I_AM_A_ROBOT AND NOT ANDROID AND NOT DISABLE_CODEGEN)
    add_subdirectory(passobject-fromservice)
    add_subdirectory(passobject-fromclient)
    add_subdirectory(task-service)
  endif()
endif()
