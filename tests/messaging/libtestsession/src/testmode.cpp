/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iomanip>

#include <iostream>
#include <qi/os.hpp>
#include <testsession/testsession.hpp>
#include <boost/optional.hpp>

static boost::optional<TestMode::Mode> globalTestMode;

void TestMode::help()
{
  std::cout << "LibTestSession Options:" << std::endl;
  std::cout << std::left << std::setw(20) << "\t--help" << "Print this usage." << std::endl;
  std::cout << std::endl;

  std::cout << std::left << std::setw(20) << "\t--mode=VALUE";
  std::cout << "Set test mode used by test sessions. Mode can also be set using with TESTMODE environment variable" << std::endl;
  std::cout << std::left << std::setw(20) << "\t";
  std::cout << "Avalaible values : direct, sd." << std::endl << std::endl;
}

void TestMode::initTestMode(int argc, char **argv)
{
  int i = 0;
  unsigned int id;
  std::map<std::string, TestMode::Mode>  _convert;
  std::string variable;

  // Print help
  for (int it = 0; it < argc; it++)
  {
    std::string arg = argv[it];
    if (arg == "--help")
    {
      TestMode::help();
      break;
    }
  }

  // Initialize conversion map.
  _convert["direct"] = TestMode::Mode_Direct;
  _convert["sd"] = TestMode::Mode_SD;
  _convert["ssl"] = TestMode::Mode_SSL;
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

    globalTestMode = _convert[variable];
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

      globalTestMode = _convert[variable];
      return;
    }
    i++;
  }
}

TestMode::Mode TestMode::getTestMode()
{
  // Check globalTestMode, if not set raise exception.
  if (!globalTestMode)
    throw TestSessionError("TestMod is not set. Use TESTMODE environment variable or TestMode::initTestMode(argc, argv).");

  return *globalTestMode;
}

void TestMode::forceTestMode(TestMode::Mode mode)
{
  // #1 Set value in global variable
  globalTestMode = mode;
}
