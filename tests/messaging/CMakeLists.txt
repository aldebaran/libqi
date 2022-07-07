# A helper library to test two sessions communicating in various modes
add_subdirectory(libtestsession)

qi_create_module(
  naoqi.testanymodule
  SRC testanymodule.cpp
  NO_INSTALL
)
qi_create_module(
  naoqi.testanymodulesession
  SRC testanymodulesession.cpp
  NO_INSTALL
)

# A separate binary that runs a simple service directory.
# Takes arguments supported by qi::ApplicationSession.
qi_create_test_helper(simplesd "simplesd.cpp" DEPENDS qi::qi qi::testssl)

# A separate binary that runs a service directory mirroring the target.
# Takes arguments supported by qi::ApplicationSession.
qi_create_test_helper(mirrorsd "mirrorsd.cpp" DEPENDS qi::qi qi::testssl)

# A separate binary that registers a service on a service directory.
# Takes arguments supported by qi::ApplicationSession.
# Expects the url of the service directory to connect to.
qi_create_test_helper(remoteserviceowner
  "remoteperformanceservice.hpp"
  "remoteserviceowner.cpp"
DEPENDS
  qi::qi
  qi::testssl
)

# General tests
qi_create_test(
  test_messaging

  "test_anymodule.cpp"
  "test_authentication.cpp"
#  "test_autoservice.cpp" # TODO: repair
  "test_binarycoder.cpp"
  "test_event_connect.cpp"
  "test_gateway.cpp"
  "test_messaging.cpp" # main
  "test_metavalue_argument.cpp"
  "test_sd.cpp"
  "test_ssl.cpp"
  "test_uri.cpp"
  "test_url.cpp"
#  "test_with_gateway.cpp" # TODO: repair
  "test_without_gateway.cpp"
  "test_streamcontext.cpp"
  "test_send_object_standalone.cpp"
  "test_message.cpp"

  DEPENDS
  qi::qi
  qi::testutils
  qi::testssl
  GTest::gmock

  TIMEOUT 120
)
add_dependencies(
  test_messaging
    naoqi.testanymodule
    naoqi.testanymodulesession
)

# Tests using two sessions communicating in various modes.
qimessaging_create_session_test(
  test_messaging_with_sessionpair

  SRC
  "objectio.hpp"
  "remoteperformanceservice.hpp"
  "test_call.cpp"
  "test_call_many.cpp"
  "test_call_on_close_session.cpp"
  "test_event_remote.cpp"
  "test_event_remote_connect.cpp"
  "test_messaging_with_sessionpair.cpp" # main
  "test_service.cpp"
  "test_session.cpp"
  "test_session_harder.cpp"

  DEPENDS
  qi::qi
  qi::testutils
  qi::testsession

  TIMEOUT 500
)

qimessaging_create_session_test(
  test_messaging_objects_with_sessionpair

  SRC
  test_send_object.cpp
  test_send_object_identification.cpp
  test_messaging_with_sessionpair.cpp # main

  DEPENDS
  qi::qi
  qi::testutils
  qi::testsession

  TIMEOUT 500
)

# Tests on Application require a separate binary.
qi_create_test(test_applicationsession SRC test_applicationsession.cpp DEPENDS qi::qi TIMEOUT 30)
qi_create_test(test_applicationsession_option SRC test_applicationsession_option.cpp DEPENDS qi::qi TIMEOUT 30)
qi_create_test(test_applicationsessionnoautoexit SRC test_applicationsession_noautoexit.cpp DEPENDS qi::qi TIMEOUT 30)

# Broken!
#qi_create_test(test_application SRC "test_messaging_application_exit.cpp" DEPENDS qi)

qi_create_test(
  test_messaging_internal

  "test_messaging_internal.cpp"
  "test_remoteobject.cpp"
  "test_tcpscheme.cpp"
  "test_transportsocketcache.cpp"
  "sock/networkmock.cpp"
  "sock/networkmock.hpp"
  "sock/networkcommon.hpp"
  "sock/networkasionooplock.hpp"
  "sock/test_accept.cpp"
  "sock/test_connect.cpp"
  "sock/test_resolve.cpp"
  "sock/test_receive.cpp"
  "sock/test_send.cpp"
  "test_tcpmessagesocket.cpp"
  "test_appsession_internal.cpp"
  "test_servicedirectory.cpp"

  DEPENDS
  # Links with qi_objects because it needs internal symbols.
  qi::internal::qi_objects
  qi::testutils
  qi::testssl
  GTest::gmock

  TIMEOUT 120
)

qi_create_test(
  test_url
    "test_url.cpp"
  DEPENDS
    qi::qi
    qi::testutils
    GTest::gmock_main
  TIMEOUT 120
)

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
