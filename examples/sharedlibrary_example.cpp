/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <iostream>
#include <fstream>

#include <qi/os.hpp>

using SinFunctor = double (*)(double);


int main(int, char*[])
{
  void *handle;

  handle = qi::os::dlopen("m");

  if (!handle) {
    std::cerr << "error: could not load libm:" << qi::os::dlerror() << std::endl;
    return 1;
  }

  SinFunctor sinptr;

  sinptr = (SinFunctor)qi::os::dlsym(handle, "sin");

  if (!sinptr) {
    std::cerr << "error: could no found symbol sin" << std::endl;
    return 1;
  }

  std::cout << "sin(42) = " << (*sinptr)(42) << std::endl;

  qi::os::dlclose(handle);
  return 0;
}

