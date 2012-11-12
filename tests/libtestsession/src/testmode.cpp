/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/os.hpp>
#include <testsession/testsession.hpp>

void TestMode::initTestMode(int argc, char **argv)
{
  int i = 0;
  unsigned int id;

  // #1 Search for the '--mode' argument.
  while (i < argc)
  {
    std::string arg(argv[i]);

    if ((id = arg.find("--mode=")) < arg.size())
    {
      // #2 Set in environment the command line argument
      // Note : "--mode=" argument will set an empty string in environment, activating random mode.
      qi::os::setenv(ENVIRON_VARIABLE, arg.substr(id + 7).c_str());
      return;
    }
    i++;
  }
}

TestMode::Mode TestMode::getTestMode()
{
  std::string variable;
  std::map<std::string, TestMode::Mode>  _convert;

  // #0 Initialize convertion map.
  _convert["direct"] = TestMode::Mode_Direct;
  _convert["sd"] = TestMode::Mode_SD;
  _convert["gateway"] = TestMode::Mode_Gateway;
  _convert["reversegateway"] = TestMode::Mode_ReverseGateway;
  _convert["remotegateway"] = TestMode::Mode_RemoteGateway;
  _convert["nightmare"] = TestMode::Mode_Nightmare;
  _convert["file"] = TestMode::Mode_NetworkMap;

  // #1 Get ENVVAR.
  variable = qi::os::getenv(ENVIRON_VARIABLE);

  // #2 Check ENVVAR, if not set pick random conf.
  if (variable.compare("") == 0)
  {
    // Todo : Pick random test mode.
    qiLogWarning("qimessaging.libtestsession") << "Test mode not set, picking Mode_SD.";
    return TestMode::Mode_SD;
  }

  // #3 Check if ENVVAR is set to some supported option.
  if (_convert.find(variable) == _convert.end())
    throw TestSessionError("[Internal] Unknown mode");

  // #4 Return suitable mode.
  return _convert[variable];
}

void TestMode::forceTestMode(TestMode::Mode mode)
{
  std::map<TestMode::Mode, std::string>  _convert;

  // #0 Initialize convertion map.
  _convert[TestMode::Mode_Direct] = "direct";
  _convert[TestMode::Mode_SD] = "sd";
  _convert[TestMode::Mode_Gateway] = "gateway";
  _convert[TestMode::Mode_ReverseGateway] = "reversegateway";
  _convert[TestMode::Mode_RemoteGateway] = "remotegateway";
  _convert[TestMode::Mode_Nightmare] = "nightmare";
  _convert[TestMode::Mode_NetworkMap] = "file";

  // #1 Set value in environment
  qi::os::setenv(ENVIRON_VARIABLE, _convert[mode].c_str());

  // #2 Log it.
  qiLogVerbose("qimessaging.libtestsession") << "Test mode forced to " << _convert[mode];
}
