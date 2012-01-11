/*
** main-client.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Jan  3 13:52:00 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include "network-thread.hpp"
#include "transport-server.hpp"

int main(int argc, char *argv[])
{
  NetworkThread *nthd = new NetworkThread();
  sleep(1);
  TransportServer *ts = new TransportServer("127.0.0.1", 9559);
  ts->run(nthd->getEventBase());

  while (true)
    ;

  return 0;
}
