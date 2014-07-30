/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>
#include <qi/application.hpp>

int main(int argc, char** argv)
{
  std::cout << "Hello, world" << std::endl;
  qi::Application(argc, argv);
  return 0;
}