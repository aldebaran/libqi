/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <qi/log.hpp>
#include <qimessaging/c/qi_c.h>

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

  qi_message_t *message = qi_message_create();
  char* result = 0;

  // call
  qi_message_write_string(message, "plaf");
  qi_future_t* fut = qi_object_call(object, "reply::(s)", message);

  //if you want a callback
  //qi_future_set_callback(fut, cb, NULL);

  qi_future_wait(fut);
  qi_message_t *msg = 0;

  if (qi_future_is_error(fut))
    qiLogError("qimessaging.qi-client-c") << "Future has error : " << qi_future_get_error(fut);

  if (!qi_future_is_ready(fut))
    qiLogError("qimessaging.qi-client-c") << "Future is not ready [:\n";

  msg = 0;
  if (!qi_future_is_error(fut) && qi_future_is_ready(fut))
    msg = (qi_message_t*) qi_future_get_value(fut);

  if (msg)
    result = qi_message_read_string(msg);

  if (result)
    qiLogInfo("qimessaging.qi-client-c") << "Reply : " << result;
  qi_future_destroy(fut);
  qi_message_destroy(message);
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
    qiLogInfo("qimessaging.qi-client-c") << "Usage : ./qi-client-c master-address";
    qiLogInfo("qimessaging.qi-client-c") << "Assuming master address is tcp://127.0.0.1:5555";
    sd_addr = strdup("tcp://127.0.0.1:5555");
  }
  else
    sd_addr = argv[1];
  int ret = make_call(sd_addr);

  qi_application_destroy(app);
  return ret;
}
