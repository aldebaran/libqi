/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <qic/object.h>
#include <qic/session.h>
#include <qic/value.h>
#include <qic/future.h>
#include <qic/application.h>


void evcb(qi_value_t *value, void *user_data) {
  qi_value_t* str = qi_value_tuple_get(value, 0);
  const char *ts = qi_value_get_string(str);
  printf("Event recv %s\n", ts);
  free((void*)ts);
  qi_value_destroy(str);
}

int make_call(qi_application_t *app, char *addr, int ev)
{
  qi_session_t* session = qi_session_create();
  qi_object_t*  object = 0;
  qi_value_t*   message = 0;
  qi_value_t*   str = 0;
  qi_future_t*  fut = 0;
  const char*   ss = 0;
  qi_value_t*   ret = 0;
  const char*   result = 0;

  qi_session_connect(session, addr);

  object = qi_future_get_object(qi_session_get_service(session, "serviceTest"));

  if (object == 0)
  {
    printf("obj == 0\n");
    return (1);
  }

  if (ev) {
    qi_object_event_connect(object, "testEvent::(s)", &evcb, 0);
    qi_application_run(app);
  }

  message = qi_value_create("(s)");
  str     = qi_value_create("s");

  qi_value_set_string(str, "plaf");

  ss = qi_value_get_string(str);
  printf("str: %s\n", ss);


  qi_value_tuple_set(message, 0, str);

  fut = qi_object_call(object, "reply::(s)", message);

  qi_future_wait(fut, 0);

  if (qi_future_has_error(fut, QI_FUTURETIMEOUT_INFINITE))
    printf("Future has error : %s\n", qi_future_get_error(fut));

  if (!qi_future_is_finished(fut))
    printf("Future is not ready [:\n");

  ret = 0;
  if (qi_future_has_value(fut, QI_FUTURETIMEOUT_INFINITE))
    ret = qi_future_get_value(fut);

  result  = 0;
  if (ret)
    result = qi_value_get_string(ret);

  if (result)
    printf("Reply : %s\n", result);
  qi_future_destroy(fut);
  qi_value_destroy(message);
  qi_value_destroy(str);
  qi_value_destroy(ret);
  free((void*)result);
  qi_object_destroy(object);
  qi_session_destroy(session);
  return (0);
}

int main(int argc, char *argv[])
{
  qi_application_t * app = qi_application_create(&argc, argv);
  char*     sd_addr = 0;
  int       event = 0;
  int       ret = 0;

  // get the program options
  if (argc < 2)
  {
    printf("Usage : ./qi-client-c [--event] master-address");
    printf("Assuming master address is tcp://127.0.0.1:5555");
    sd_addr = strdup("tcp://127.0.0.1:5555");
  }
  else {
    if (!strcmp(argv[1], "--event")) {
      event = 1;
      sd_addr = argv[2];
    } else
      sd_addr = argv[1];
  }
  ret = make_call(app, sd_addr, event);

  qi_application_destroy(app);
  return ret;
}
