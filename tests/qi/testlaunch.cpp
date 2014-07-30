/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc == 1)
    return 42;
  else
  {
    int sum = 0;
    for (int i = 1; i < argc; ++i)
      sum += atoi(argv[i]);
    return sum;
  }
  return -1;
}
