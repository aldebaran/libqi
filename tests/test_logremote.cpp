#include <qi/log.hpp>

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
//   pid      : "1242",
//   category : "core.log.test",
//   file     : "",
//   function : "",
//   lineno   : "",
//   log      : "balblalsal",
// }

#include <rttools/rtthread.h>

class MyThread: public RtThread
{
  virtual void* execute();
};

#if 0
void *MyThread::execute()
{
  periodInit();
  int i = 0;
  while (1) {
    periodWait();

    printf("core.log.test1 %d\n", 41);
    printf("core.log.test1", "%d\n", 42);
    printf("core.log.test1", "%d\n", 43);
    printf("core.log.test1", "%d\n", 44);
    printf("core.log.test1", "%d\n", 45);
    printf("core.log.test1", "%d\n", 46);

    std::cout << "core.log.test2"  << "f" << 4 << std::endl;
    std::cout << "core.log.test2"    << "e" << 4 << std::endl;
    std::cout << "core.log.test2"  << "w" << 4 << std::endl;
    std::cout << "core.log.test2"     << "i" << 4 << std::endl;
    std::cout << "core.log.test2"  << "v" << 4 << std::endl;
    std::cout << "core.log.test2"    << "d" << 4 << std::endl;

    std::cout << "core.log.test3" << "%d"<< 21   << "f" << 4 << std::endl;
    std::cout << "core.log.test3" << "%d"<< 21   << "e" << 4 << std::endl;
    std::cout << "core.log.test3" << "%d"<< 21 << "w" << 4 << std::endl;
    std::cout << "core.log.test3" << "%d"<< 21    << "i" << 4 << std::endl;
    std::cout << "core.log.test3" << "%d"<< 21 << "v" << 4 << std::endl;
    std::cout << "core.log.test3" << "%d"<< 21   << "d" << 4 << std::endl;

    //c style
    printf("core.log.test4 oups my buffer is too bad: %x\n", 0x0BADCAFE);

    //c++ style
    std::cout << "core.log.test4" << "dont drink and drive, just smoke and fly"
        << "- what? " << 42 << "- oh.. I prefer!" << std::endl;

    //mixup style
    printf("core.log.test4 %d %d", 41, 42);
    std::cout << 43 << 44 << std::endl;
    ++i;
  }
}
#else
void *MyThread::execute()
{
  periodInit();
  int i = 0;
  while (1) {
    periodWait();

    qiLogFatal("core.log.test1", "%d\n", 41);
    qiLogError("core.log.test1", "%d\n", 42);
    qiLogWarning("core.log.test1", "%d\n", 43);
    qiLogInfo("core.log.test1", "%d\n", 44);
    qiLogVerbose("core.log.test1", "%d\n", 45);
    qiLogDebug("core.log.test1", "%d\n", 46);

    qiLogFatal("core.log.test2")   << "f" << 4 << std::endl;
    qiLogError("core.log.test2")   << "e" << 4 << std::endl;
    qiLogWarning("core.log.test2") << "w" << 4 << std::endl;
    qiLogInfo("core.log.test2")    << "i" << 4 << std::endl;
    qiLogVerbose("core.log.test2") << "v" << 4 << std::endl;
    qiLogDebug("core.log.test2")   << "d" << 4 << std::endl;

    qiLogFatal("core.log.test3", "%d", 21)   << "f" << 4 << std::endl;
    qiLogError("core.log.test3", "%d", 21)   << "e" << 4 << std::endl;
    qiLogWarning("core.log.test3", "%d", 21) << "w" << 4 << std::endl;
    qiLogInfo("core.log.test3", "%d", 21)    << "i" << 4 << std::endl;
    qiLogVerbose("core.log.test3", "%d", 21) << "v" << 4 << std::endl;
    qiLogDebug("core.log.test3", "%d", 21)   << "d" << 4 << std::endl;

    //c style
    qiLogWarning("core.log.test4", "oups my buffer is too bad: %x\n", 0x0BADCAFE);

    //c++ style
    qiLogError("core.log.test4") << "dont drink and drive, just smoke and fly"
                                 << "- what? " << 42 << "- oh.. I prefer!" << std::endl;

    //mixup style
    qiLogInfo("core.log.test4", "%d %d", 41, 42) << 43 << 44 << std::endl;
    ++i;
  }
}

#endif

#include <allogremote/allogremotehandler.h>

int main(int argc, char **argv)
{
  int ret = 0;
  //qi::log::setVerbosity(qi::log::fatal);
  AL::ALLogRemoteHandler::getInstance()->logToForwarder("tcp://127.0.0.1:5566");

  MyThread* myThread = new MyThread();
  ret = myThread->setRealtime(SCHED_FIFO, 35);
  std::cout << "RT?" << std::endl;
  if (!ret)
    std::cout << "Warning: thread is not realtime" << std::endl;
  //10ms like the dcm
  myThread->setPeriod(10000);
  myThread->create();
  myThread->join();

  // for (int i = 0; i < 1000; i++)
  // {
  //   qiLogFatal("core.log.test1", "%d\n", 41);
  //   qiLogError("core.log.test1", "%d\n", 42);
  //   qiLogWarning("core.log.test1", "%d\n", 43);
  //   qiLogInfo("core.log.test1", "%d\n", 44);
  //   qiLogVerbose("core.log.test1", "%d\n", 45);
  //   qiLogDebug("core.log.test1", "%d\n", 46);

  //   qiLogFatal("core.log.test2")   << "f" << 4 << std::endl;
  //   qiLogError("core.log.test2")   << "e" << 4 << std::endl;
  //   qiLogWarning("core.log.test2") << "w" << 4 << std::endl;
  //   qiLogInfo("core.log.test2")    << "i" << 4 << std::endl;
  //   qiLogVerbose("core.log.test2") << "v" << 4 << std::endl;
  //   qiLogDebug("core.log.test2")   << "d" << 4 << std::endl;

  //   qiLogFatal("core.log.test3", "%d", 21)   << "f" << 4 << std::endl;
  //   qiLogError("core.log.test3", "%d", 21)   << "e" << 4 << std::endl;
  //   qiLogWarning("core.log.test3", "%d", 21) << "w" << 4 << std::endl;
  //   qiLogInfo("core.log.test3", "%d", 21)    << "i" << 4 << std::endl;
  //   qiLogVerbose("core.log.test3", "%d", 21) << "v" << 4 << std::endl;
  //   qiLogDebug("core.log.test3", "%d", 21)   << "d" << 4 << std::endl;

  //   //c style
  //   qiLogWarning("core.log.test4", "oups my buffer is too bad: %x\n", 0x0BADCAFE);

  //   //c++ style
  //   qiLogError("core.log.test4") << "dont drink and drive, just smoke and fly"
  //                                << "- what? " << 42 << "- oh.. I prefer!" << std::endl;

  //   //mixup style
  //   qiLogInfo("core.log.test4", "%d %d", 41, 42) << 43 << 44 << std::endl;
  // }
  // std::cout << "I've just finished to log!" << std::endl;
}
