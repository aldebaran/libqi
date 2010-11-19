/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/qi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  qi_client_t *client = qi_client_create("simplecli", argv[1]);

  qi_message_t *message = qi_message_create();
  qi_message_t *ret = qi_message_create();

  qi_message_write_string(message, "master.locateService::s:s");
  qi_message_write_string(message, "master.listServices::{ss}:");

  qi_client_call(client, "master.locateService::s:s", message, ret);

  char *result = qi_message_read_string(ret);
  printf("locate returned: %s\n", result);
  return 0;
}
