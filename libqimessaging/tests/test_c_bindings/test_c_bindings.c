/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/server.h>
#include <qimessaging/client.h>
#include <qimessaging/message.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void brandade(const char *signature, qi_message_t *params, qi_message_t *ret, void *data)
{
  printf("paf la brandade!\n");
}

void usage() {
  printf("usage: test_c_bindings address:port\n");
  exit(1);
}

int main(int argc, char **argv)
{
  char *result;

  if (argc <= 1)
    usage();
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
  qi_server_advertise_service(server, "brandade:::", &brandade, 0);
  sleep(1);

  qi_message_t *message = qi_message_create();
  qi_message_t *ret = qi_message_create();

  qi_message_write_string(message, "brandade:::");
//  qi_message_write_string(message, "master.listServices::{ss}:");
//  qi_message_write_string()
  qi_client_call(client, "brandade:::", message, ret);

  sleep(1);
  return 0;
}

