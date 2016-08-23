/*
 * Copyright (c) 2012-2015 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <atomic>
#include <cstdio>
#include <mutex>

#include <qi/os.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>

struct SafeCout
{
  static std::mutex m;

  template<typename T>
  std::ostream& operator<<(const T& whatever)
  {
    std::unique_lock<std::mutex> l{m};
    auto& o = std::cout << whatever;
    fflush(stdout);
    return o;
  }
} safeCout;

std::mutex SafeCout::m = {};

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

    safeCout << "Stop running" << std::endl;
  }

  void printTest()
  {
    safeCout << "Running..." << std::endl;
  }

public:
  std::atomic<bool>          _glInit;
  boost::thread              _glThread;
} threadGlobal;

inline threadTest::threadTest()
{
  _glInit = true;
  safeCout << "Creating main thread" << std::endl;
  _glThread = boost::thread(&threadTest::run, &threadGlobal);
};

inline threadTest::~threadTest()
{
  _glInit = false;

  _glThread.interrupt();
  safeCout << "Interrupting main thread" << std::endl;

  _glThread.join();
  safeCout << "Joining main thread" << std::endl;

  printTest();
  safeCout << "Main thread destroyed" << std::endl;
};


int main(int argc, char *argv[])
{
  safeCout << "Entering main function" << std::endl;

  qi::os::sleep(5);

  safeCout << "Leaving main function" << std::endl;

  exit(0);
}
