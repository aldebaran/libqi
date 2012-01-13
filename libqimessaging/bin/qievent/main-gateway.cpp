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
#include "gateway.hpp"
#include "network-thread.hpp"

int main(int argc, char *argv[])
{
  NetworkThread *n = new NetworkThread();
  sleep(1);
  Gateway g;
  g.run(n);

  while (true)
    ;

  return 0;
}
