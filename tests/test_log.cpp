#include <qi/log.hpp>

// { process  : "naoqi",
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


int main(int argc, char **argv)
{
  qi::log::setVerbosity(qi::log::debug);
  int ret = 0;
  //qi::log::setVerbosity(qi::log::fatal);

  MyThread* myThread = new MyThread();
  ret = myThread->setRealtime(SCHED_FIFO, 35);
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
