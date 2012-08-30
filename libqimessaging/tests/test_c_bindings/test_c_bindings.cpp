/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/c/api_c.h>
#include <qimessaging/c/qi_c.h>
#include <qi/os.hpp>
#include <gtest/gtest.h>
#include <qimessaging/service_directory.hpp>
#include <string>
#include <iostream>

void reply(const char *signature, qi_message_t *message, qi_message_t *answer, void *data)
{
  char* msg = qi_message_read_string(message);
  char* rep = (char *) malloc(strlen(msg) + 4);

  memcpy(rep, msg, strlen(msg) + 1);
  printf("Message recv: %s\n", msg);
  strcat(rep, "bim");

  qi_message_write_string(answer, rep);
}


qi_message_t*         message;
qi_application_t*     app;
qi_session_t*         session;
qi_session_t*         client_session;
qi_object_t*          object;
qi_object_t*          remote;
qi::ServiceDirectory*  sd;
std::string           connectionAddr;
int                   id;

TEST(TestCBindings, InitService)
{
  assert(app != 0);
  object = qi_object_create("replyObj");
  assert(object != 0);
}

TEST(TestCBindings, BindMethod)
{
  qi_object_register_method(object, "reply::s(s)", &reply, 0);

  // TODO find a way to test this
}

TEST(TestCBindings, ConnectSession)
{
  session = qi_session_create();
  assert(session != 0);

  qi_session_connect(session, connectionAddr.c_str());
  qi_session_wait_for_connected(session, 3000);

  assert(qi_session_listen(session, "tcp://0.0.0.0:0"));

  id = qi_session_register_service(session, "serviceTest", object);
}

TEST(TestCBindings, InitClient)
{
  client_session = qi_session_create();
  assert(client_session != 0);

  qi_session_connect(client_session, connectionAddr.c_str());
  qi_session_wait_for_connected(client_session, 3000);

  remote = qi_session_get_service(client_session, "serviceTest");
  assert(remote != 0);

  assert(qi_session_get_service(client_session, "pute") == 0);
}

TEST(TestCBindings, CallReply)
{
  message = qi_message_create();
  assert(message != 0);

  int payload_size = 0;
  char* result = 0;

  // compute payload and call
  payload_size = strlen("plaf") + 4;
  qi_message_write_string(message, "plaf");
  qi_future_t* fut = qi_object_call(object, "reply::s(s)", message);

  qi_future_wait(fut);
  qi_message_t *msg = 0;

  assert((bool) qi_future_is_error(fut) == false);
  assert((bool) qi_future_is_ready(fut) == true);

  msg = (qi_message_t*) qi_future_get_value(fut);
  assert(msg != 0);

  result = qi_message_read_string(msg);
  assert(strcmp(result, "plafbim") == 0);
}

TEST(TestCBindings, TearDown)
{
  sd->close();
  qi_message_destroy(message);
  qi_session_unregister_service(session, id);
  qi_session_destroy(session);
  qi_session_destroy(client_session);
  qi_object_destroy(object);
  qi_object_destroy(remote);
  qi_application_destroy(app);
}

int main(int argc, char **argv) {
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  unsigned int sdPort = qi::os::findAvailablePort(5555);
  std::stringstream sdAddr;
  sdAddr << "tcp://127.0.0.1:" << sdPort;

  connectionAddr = sdAddr.str();

  sd = new qi::ServiceDirectory();
  sd->listen(sdAddr.str());
  std::cout << "Service Directory ready." << std::endl;

  return RUN_ALL_TESTS();
}