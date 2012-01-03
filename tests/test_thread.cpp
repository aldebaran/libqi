/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <iostream>
#include <cstdio>

#include <qi/os.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>

static class threadTest
{
public:
  inline threadTest();
  inline ~threadTest();

  void run()
  {
    while (_glInit)
    {
      printTest();
      qi::os::sleep(1);
    }

    std::cout << "Stop running" << std::endl;
    fflush(stdout);
  }

  void printTest()
  {
    std::cout << "Running..." << std::endl;
    fflush(stdout);
  }

public:
  bool                       _glInit;
  boost::thread              _glThread;
  boost::mutex               _glLock;
  boost::condition_variable  _glCond;
} threadGolbal;

inline threadTest::threadTest()
{
  _glInit = true;
  _glThread = boost::thread(&threadTest::run, &threadGolbal);
  std::cout << "Creating main thread" << std::endl;
  fflush(stdout);
};

inline threadTest::~threadTest()
{
  _glInit = false;

  _glThread.interrupt();
  std::cout << "Interrupting main thread" << std::endl;
  fflush(stdout);

  _glThread.join();
  std::cout << "Joinning main thread" << std::endl;
  fflush(stdout);

  printTest();
  std::cout << "Main thread destroyed" << std::endl;
  fflush(stdout);
};


int main(int argc, char *argv[])
{
  std::cout << "Entering main function" << std::endl;
  fflush(stdout);

  qi::os::sleep(5);

  std::cout << "Leaving main function" << std::endl;
  fflush(stdout);

  exit(0);
}
