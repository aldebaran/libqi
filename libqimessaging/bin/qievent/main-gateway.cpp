/*
** main-gateway.cpp
** Login : <hcuche@hcuche-de>
** Started on  Fri Jan 13 10:53:50 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <qimessaging/transport/gateway.hpp>
#include <qimessaging/transport/network_thread.hpp>

int main(int argc, char *argv[])
{
  qi::NetworkThread *n = new qi::NetworkThread();
  sleep(1);
  qi::Gateway g;
  g.start("127.0.0.1", 9559, n);

  while (true)
    ;

  return 0;
}
