#include <qi/log.hpp>

// { process  : "naoqi",
//   pid      : "1242",
//   category : "core.log.test",
//   file     : "",
//   function : "",
//   lineno   : "",
//   log      : "balblalsal",
// }

int main(int argc, char **argv)
{
  //  qi::log::init("myprogramname", argc, argv);

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

  std::cout << "I've just finished to log!" << std::endl;
}
