#include <qi/log.hpp>

// { process  : "naoqi",
//   pid      : "1242",
//   category : "qi.audio",
//   file     : "",
//   function : "",
//   lineno   : "",
//   log      : "balblalsal",
// }

int main(int argc, char **argv)
{
  //  qi::log::init("myprogramname", argc, argv);

  qiLogFatal("qi.audio", "%d\n", 41);
  qiLogError("qi.audio", "%d\n", 42);
  qiLogWarning("qi.audio", "%d\n", 43);
  qiLogInfo("qi.audio", "%d\n", 44);
  qiLogVerbose("qi.audio", "%d\n", 45);
  qiLogDebug("qi.audio", "%d\n", 46);

  qiLogFatal("qi.audio")   << "f" << 4 << std::endl;
  qiLogError("qi.audio")   << "e" << 4 << std::endl;
  qiLogWarning("qi.audio") << "w" << 4 << std::endl;
  qiLogInfo("qi.audio")    << "i" << 4 << std::endl;
  qiLogVerbose("qi.audio") << "v" << 4 << std::endl;
  qiLogDebug("qi.audio")   << "d" << 4 << std::endl;

  qiLogFatal("qi.audio", "%d", 21)   << "f" << 4 << std::endl;
  qiLogError("qi.audio", "%d", 21)   << "e" << 4 << std::endl;
  qiLogWarning("qi.audio", "%d", 21) << "w" << 4 << std::endl;
  qiLogInfo("qi.audio", "%d", 21)    << "i" << 4 << std::endl;
  qiLogVerbose("qi.audio", "%d", 21) << "v" << 4 << std::endl;
  qiLogDebug("qi.audio", "%d", 21)   << "d" << 4 << std::endl;

  //c style
  qiLogWarning("qi.audio", "oups my buffer is too bad: %x\n", 0x0BADCAFE);

  //c++ style
  qiLogError("qi.audio") << "dont drink and drive, just smoke and fly"
                         << "- what? " << 42 << "- oh.. I prefer!" << std::endl;

  //mixup style
  qiLogInfo("qi.audio", "%d %d", 41, 42) << 43 << 44 << std::endl;

  std::cout << "I've just finished to log!" << std::endl;
}
