/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <qic/session.h>
#include <qic/value.h>
#include <qic/future.h>
#include <qic/application.h>

int make_call(char *addr)
{
  qi_session_t* session = qi_session_create();

  qi_session_connect(session, addr);

  qi_object_t* object = qi_session_get_service(session, "serviceTest");

  if (object == 0)
  {
    printf("obj == 0\n");
    return (1);
  }

  qi_value_t *message = qi_value_create("(s)");
  qi_value_t *str     = qi_value_create("s");

  qi_value_set_string(str, "plaf");

  const char *ss = qi_value_get_string(str);
  printf("str: %s\n", ss);


  qi_value_tuple_set(message, 0, str);

  qi_future_t* fut = qi_object_call(object, "reply::(s)", message);

  qi_future_wait(fut);

  if (qi_future_has_error(fut))
    printf("Future has error : %s\n", qi_future_get_error(fut));

  if (!qi_future_is_ready(fut))
    printf("Future is not ready [:\n");

  qi_value_t *ret = 0;
  if (!qi_future_has_error(fut) && qi_future_is_ready(fut))
    ret = qi_future_get_value(fut);

  const char *result = 0;
  int err = 0;
  if (ret)
    result = qi_value_get_string(ret);

  if (result)
    printf("Reply : %s\n", result);
  qi_future_destroy(fut);
  qi_value_destroy(message);
  qi_value_destroy(str);
  qi_value_destroy(ret);
  free(result);
  qi_object_destroy(object);
  qi_session_destroy(session);
  return (0);
}

int main(int argc, char *argv[])
{
  qi_application_t * app = qi_application_create(&argc, argv);
  char*     sd_addr = 0;

  // get the program options
  if (argc != 2)
  {
    printf("Usage : ./qi-client-c master-address");
    printf("Assuming master address is tcp://127.0.0.1:5555");
    sd_addr = strdup("tcp://127.0.0.1:5555");
  }
  else
    sd_addr = argv[1];
  int ret = make_call(sd_addr);

  qi_application_destroy(app);
  return ret;
}
