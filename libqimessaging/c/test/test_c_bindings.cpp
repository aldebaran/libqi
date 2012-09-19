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
#include <qimessaging/servicedirectory.hpp>
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

std::string           connectionAddr;


TEST(TestCBindings, Call)
{

  qi_message_t*         message;
  qi_session_t*         session;
  qi_session_t*         client_session;
  qi_object_builder_t*  ob;
  qi_object_t*          object;
  qi_object_t*          remote;

  int                   id;

  ob = qi_object_builder_create();
  qi_object_builder_register_method(ob, "reply::s(s)", &reply, 0);
  object = qi_object_builder_get_object(ob);

  session = qi_session_create();
  ASSERT_TRUE(session != 0);

  qi_session_connect(session, connectionAddr.c_str());

  ASSERT_TRUE(qi_session_listen(session, "tcp://0.0.0.0:0"));

  id = qi_session_register_service(session, "serviceTest", object);
  client_session = qi_session_create();
  ASSERT_TRUE(client_session != 0);

  qi_session_connect(client_session, connectionAddr.c_str());

  remote = qi_session_get_service(client_session, "serviceTest");
  ASSERT_TRUE(remote != 0);

  ASSERT_TRUE(qi_session_get_service(client_session, "pute") == 0);

  message = qi_message_create();
  ASSERT_TRUE(message != 0);

  char* result = 0;

  // call
  qi_message_write_string(message, "plaf");
  qi_future_t* fut = qi_object_call(object, "reply::(s)", message);

  qi_future_wait(fut);
  qi_message_t *msg = 0;

  ASSERT_TRUE((bool) qi_future_is_error(fut) == false);
  ASSERT_TRUE((bool) qi_future_is_ready(fut) == true);

  msg = (qi_message_t*) qi_future_get_value(fut);
  ASSERT_TRUE(msg != 0);

  result = qi_message_read_string(msg);
  ASSERT_TRUE(strcmp(result, "plafbim") == 0);

  qi_message_destroy(message);
  qi_session_unregister_service(session, id);
  qi_session_destroy(session);
  qi_session_destroy(client_session);
  qi_object_builder_destroy(ob);
  qi_object_destroy(object);
  qi_object_destroy(remote);
}

int main(int argc, char **argv) {
  qi_application_t*     app;
  qi::ServiceDirectory*  sd;
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  unsigned int sdPort = qi::os::findAvailablePort(5555);
  std::stringstream sdAddr;
  sdAddr << "tcp://127.0.0.1:" << sdPort;

  connectionAddr = sdAddr.str();

  sd = new qi::ServiceDirectory();
  sd->listen(sdAddr.str());
  std::cout << "Service Directory ready." << std::endl;

  int ret = RUN_ALL_TESTS();
  sd->close();
  qi_application_destroy(app);
  return ret;
}
