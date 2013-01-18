/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/os.hpp>
#include <testsession/testsession.hpp>

TestMode::Mode testMode = TestMode::Mode_Default;

void TestMode::initTestMode(int argc, char **argv)
{
  int i = 0;
  unsigned int id;
  std::map<std::string, TestMode::Mode>  _convert;
  extern TestMode::Mode testMode;
  std::string variable;

  // Initialize convertion map.
  _convert["direct"] = TestMode::Mode_Direct;
  _convert["sd"] = TestMode::Mode_SD;
  _convert["gateway"] = TestMode::Mode_Gateway;
  _convert["reversegateway"] = TestMode::Mode_ReverseGateway;
  _convert["remotegateway"] = TestMode::Mode_RemoteGateway;
  _convert["nightmare"] = TestMode::Mode_Nightmare;
  _convert["file"] = TestMode::Mode_NetworkMap;

  // Search for the TESTMODE environment variable
  variable = qi::os::getenv(ENVIRON_VARIABLE);
  if (variable.empty() == false)
  {
    if (_convert.find(variable) == _convert.end())
      throw TestSessionError("Environment variable : Unknown value.");

    testMode = _convert[variable];
    return;
  }

  // Search for the '--mode' argument.
  while (i < argc)
  {
    std::string arg(argv[i]);

    if ((id = arg.find("--mode=")) < arg.size())
    {
      variable = arg.substr(id + 7);
      if (variable.empty() == true || _convert.find(variable) == _convert.end())
        throw TestSessionError("--mode : Unknown value.");

      testMode = _convert[variable];
      return;
    }
    i++;
  }
}

TestMode::Mode TestMode::getTestMode()
{
  extern TestMode::Mode testMode;

  // Check testMode, if not set raise exception.
  if (testMode == TestMode::Mode_Default)
    throw TestSessionError("TestMod is not set. Use TESTMODE environment variable or TestMode::initTestMode(argc, argv).");

  return testMode;
}

void TestMode::forceTestMode(TestMode::Mode mode)
{
  extern TestMode::Mode testMode;

  // #1 Set value in global variable
  testMode = mode;
}
