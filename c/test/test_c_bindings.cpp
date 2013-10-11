/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qic/session.h>
#include <qic/value.h>
#include <qic/object.h>
#include <qic/future.h>
#include <qic/value.h>
#include <qic/application.h>
#include <gtest/gtest.h>


class TestCBindings: public ::testing::Test
{
public:
  TestCBindings()
  {
    sd = qi_session_create();
  }

  ~TestCBindings() {
    qi_session_destroy(sd);
  }

protected:
  void SetUp()
  {
    qi_session_listen_standalone(sd, "tcp://127.0.0.1:0");
    qi_value_t *val = qi_session_endpoints(sd);
    qi_value_t *ele = qi_value_list_get(val, 0);
    addr = qi_value_get_string(ele);
    printf("session address: %s\n", addr);
    qi_value_destroy(ele);
    qi_value_destroy(val);
  }

  void TearDown()
  {
    free((void*)addr);
    addr = 0;
    qi_session_close(sd);
  }

public:
  qi_session_t *sd;
  const char   *addr;
};


void print(const char *signature, qi_value_t *message, qi_value_t *answer, void *data)
{
  printf("coucou\n");
}

void reply(const char *signature, qi_value_t *message, qi_value_t *answer, void *data)
{
  qi_value_t *str = qi_value_tuple_get(message, 0);
  const char* msg = qi_value_get_string(str);
  char* rep = (char *) malloc(strlen(msg) + 4);

  memcpy(rep, msg, strlen(msg) + 1);
  printf("Message recv: %s\n", msg);
  strcat(rep, "bim");

  qi_value_set_string(answer, rep);
  free((void*)msg);
  free((void*)rep);
  qi_value_destroy(str);
}


TEST_F(TestCBindings, CallWithComplexTypes)
{
  // Mirror python test.
  qi_session_t*  session;
  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_register_method(ob, "print", &print, 0);
  qi_object_t* obj = qi_object_builder_get_object(ob);

  session = qi_session_create();
  qi_future_t* fuc = qi_session_connect(session, addr);
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);

  fuc = qi_session_listen(session, "tcp://0.0.0.0:0");
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);

  unsigned long long id = qi_future_get_uint64_default(qi_session_register_service(session, "service", obj), 0);
  EXPECT_GT(id, 0u);

  qi_object_t* proxy = qi_future_get_object(qi_session_get_service(session, "service"));

  qi_future_t *futc = qi_session_close(session);
  qi_future_wait(futc, QI_FUTURETIMEOUT_INFINITE);
  qi_future_destroy(futc);
  qi_session_destroy(session);
  qi_object_destroy(obj);
  qi_object_destroy(proxy);
  qi_object_builder_destroy(ob);
}

TEST_F(TestCBindings, TestDestructionOrder)
{
  qi_session_t*  session;

  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_register_method(ob, "reply", &reply, 0);
  qi_object_t* obj = qi_object_builder_get_object(ob);

  session = qi_session_create();
  qi_future_t* fuc = qi_session_connect(session, addr);
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);

  fuc = qi_session_listen(session, "tcp://0.0.0.0:0");
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);

  EXPECT_GT(qi_future_get_uint64_default(qi_session_register_service(session, "service", obj), 0), 0u);

  qi_future_t *fu = qi_session_get_service(session, "service");
  qi_value_t* fuv = qi_future_get_value(fu);
  qi_object_t* proxy = qi_value_get_object(fuv);

  qi_value_destroy(fuv);
  qi_future_destroy(fu);

  qi_future_t *futc = qi_session_close(session);
  qi_future_wait(futc, QI_FUTURETIMEOUT_INFINITE);
  qi_future_destroy(futc);
  qi_session_destroy(session);
  qi_object_destroy(obj);
  qi_object_destroy(proxy);
  qi_object_builder_destroy(ob);
}

TEST_F(TestCBindings, AlreadyRegistered)
{
  qi_session_t*  session;
  std::stringstream ss;

  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_register_method(ob, "reply", &reply, 0);
  qi_object_t* obj = qi_object_builder_get_object(ob);

  session = qi_session_create();
  qi_future_t* fuc = qi_session_connect(session, addr);
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);

  fuc = qi_session_listen(session, "tcp://0.0.0.0:0");
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);


  EXPECT_GT(qi_future_get_uint64_default(qi_session_register_service(session, "service", obj), 0), 0u);
  EXPECT_EQ(0u, qi_future_get_uint64_default(qi_session_register_service(session, "service", obj), 0));
  qi_object_destroy(obj);
  qi_session_destroy(session);
}

TEST_F(TestCBindings, Call)
{
  qi_session_t*         session;
  qi_session_t*         client_session;
  qi_object_builder_t*  ob;
  qi_object_t*          object;
  qi_object_t*          remote;

  unsigned long long id;

  ob = qi_object_builder_create();
  qi_object_builder_register_method(ob, "reply::s(s)", &reply, 0);
  object = qi_object_builder_get_object(ob);

  session = qi_session_create();
  ASSERT_TRUE(session != 0);

  qi_future_wait(qi_session_connect(session, addr), QI_FUTURETIMEOUT_INFINITE);


  ASSERT_TRUE(qi_session_listen(session, "tcp://0.0.0.0:0"));

  id = qi_future_get_uint64_default(qi_session_register_service(session, "serviceTest", object), 0);
  client_session = qi_session_create();
  ASSERT_TRUE(client_session != 0);

  qi_future_t *futco = qi_session_connect(client_session, addr);
  qi_future_wait(futco, QI_FUTURETIMEOUT_INFINITE);
  qi_future_destroy(futco);
  remote = qi_future_get_object(qi_session_get_service(session, "serviceTest"));
  ASSERT_TRUE(remote != 0);

  ASSERT_TRUE(qi_future_get_object(qi_session_get_service(client_session, "I don't exist")) == 0);

  qi_value_t* cont = qi_value_create("(s)");
  qi_value_t* message = qi_value_create("s");
  qi_value_set_string(message, "plaf");
  qi_value_tuple_set(cont, 0, message);
  ASSERT_TRUE(message != 0);

  const char* result = 0;

  // call
  qi_future_t* fut = qi_object_call(object, "reply::(s)", cont);

  qi_future_wait(fut, QI_FUTURETIMEOUT_INFINITE);
  qi_value_t *ret = 0;

  ASSERT_EQ(0, qi_future_has_error(fut, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_EQ(1, qi_future_is_finished(fut));

  ret = qi_future_get_value(fut);
  ASSERT_TRUE(ret != 0);
  result = qi_value_get_string(ret);
  ASSERT_TRUE(strcmp(result, "plafbim") == 0);
  free((void*)result);
  qi_future_destroy(fut);
  qi_value_destroy(ret);
  qi_value_destroy(cont);
  qi_value_destroy(message);
  qi_session_unregister_service(session, id);
  qi_session_destroy(session);
  qi_session_destroy(client_session);
  qi_object_builder_destroy(ob);
  qi_object_destroy(object);
  qi_object_destroy(remote);
}

int main(int argc, char **argv) {
  qi_application_t*     app;
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  qi_application_destroy(app);
  return ret;
}
