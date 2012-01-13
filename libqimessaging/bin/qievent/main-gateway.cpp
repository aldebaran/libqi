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

int main(int argc, char *argv[])
{
  Gateway g;
  g.onRead("call.12.audio.say.hello world!");

  return 0;
}
