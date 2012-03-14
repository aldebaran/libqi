/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <stdlib.h>
#include <qi/os.hpp>

int main(int argc, char* argv[])
{
  int i = 0;
  while(i < 30) // 30 secs to be killed
  {
    qi::os::sleep(1);
    i++;
  }
  return 0;
}
