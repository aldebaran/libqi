/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/qi.h>
#include <stdio.h>


void branle(const char *signature, qi_message_t *params, qi_message_t *ret, void *data)
{
  printf("paf la branle!\n");
}

int main(int argc, char **argv)
{
  char *result;

  qi_client_t *client = qi_client_create("simplecli");
  qi_client_connect(client, argv[1]);
  //qi_message_t *message = qi_message_create();
  //qi_message_t *ret = qi_message_create();

//  qi_message_write_string(message, "master.locateService::s:ss");
//  qi_message_write_string(message, "master.listServices::{ss}:");
//  qi_message_write_string()
//  qi_client_call(client, "master.locateService::s:ss", message, ret);

  //result = qi_master_locate_service(client, "master.locateService::s:ss");

  //result = qi_message_read_string(ret);
  printf("locate returned: %s\n", result);


  qi_server_t *server = qi_server_create("serv");
  qi_server_connect(server, argv[1]);
  qi_server_advertise_service(server, "bande:::", &branle, 0);
  sleep(1);

  qi_message_t *message = qi_message_create();
  qi_message_t *ret = qi_message_create();

  qi_message_write_string(message, "bande:::");
//  qi_message_write_string(message, "master.listServices::{ss}:");
//  qi_message_write_string()
  qi_client_call(client, "bande:::", message, ret);

  sleep(1);
  return 0;
}

