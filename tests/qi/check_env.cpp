/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */


#include <iostream>
#include <qi/os.hpp>


int main()
{
  std::string foo = qi::os::getenv("FOO");
  if (foo != "BAR")
  {
    std::cerr << "FOO should be BAR and is: '" << foo << "'" << std::endl;
    return 1;
  }
  return 0;
}

