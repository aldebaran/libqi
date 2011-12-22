/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
