/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <thread>
#include <chrono>

int main(int, char**)
{
  int i = 0;
  while(i < 30) // 30 secs to be killed
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    i++;
  }
  return 0;
}
