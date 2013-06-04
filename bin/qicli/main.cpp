#include <boost/program_options.hpp>
#include <qi/application.hpp>

#include "qicli.hpp"

namespace po = boost::program_options;

std::map<std::string, SubCmd> subCmdMap;

void init()
{
  subCmdMap["service"] = &subCmd_service;
  subCmdMap["call"] = &subCmd_call;
  subCmdMap["post"] = &subCmd_post;
  subCmdMap["top"] = &subCmd_top;
  subCmdMap["watch"] = &subCmd_watch;
  subCmdMap["get"] = &subCmd_get;
  subCmdMap["set"] = &subCmd_set;
}

void initApp(bool qilog)
{
  static char *offArgv[] =
  {
    (char*)"qicli",
    (char*)"-q",
  };

  static char *onArgv[] =
  {
    (char*)"qicli",
    (char*)"-v",
  };

  char **argv;
  int argc = 2;

  if (qilog)
    argv = onArgv;
  else
    argv = offArgv;

  static qi::Application *app;

  app = new qi::Application(argc, argv);
  (void)app;
}

int                 main(int argc, char **argv)
{
  int             subCmdArgc = 0;
  char            **subCmdArgv = 0;
  SubCmd          subCmd = 0;

  init();
  for (int i = 0; i < argc; ++i)
  {
    std::map<std::string, SubCmd>::const_iterator it = subCmdMap.find(argv[i]);
    if (it != subCmdMap.end())
    {
      subCmdArgc = argc - i;
      subCmdArgv = &argv[i];
      subCmd = it->second;
      argc = i + 1;
      break;
    }
  }

  po::options_description desc("Usage: qicli [OPTIONS] [SUBCMD] [ARGS]");

  MainOptions options;

  desc.add_options()
      ("address,a", po::value<std::string>(&options.address)->default_value("tcp://127.0.0.1:9559"), "The address of the service directory")
      ("verbose,v", "turn on verbose mode")
      ("command,c", "sub command to execute")
      ("help,h", "Print this help message and exit")
      ("qilog,q", "turn on qilog");

  po::positional_options_description positionalOptions;
  positionalOptions.add("command", 1);
  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);
  } catch (po::error &e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  if (vm.count("help") || subCmdArgv == 0)
  {
    showHelp(desc);
    exit(0);
  }

  if (vm.count("verbose"))
    options.verbose = true;

  initApp(vm.count("qilog"));

  SessionHelper session;

  if (options.verbose)
  {
    std::cerr << "connecting to [" << options.address << "] : ";
    std::cerr.flush();
  }
  try {
    session.connect(options.address);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  if (options.verbose)
    std::cerr << "OK" << std::endl;
  int ret = subCmd(subCmdArgc, subCmdArgv, session, options);
  qi::FutureSync<void> fut = session.close();
  if (fut.hasError())
    std::cerr << "error while closing session: " << fut.error() << std::endl;
  return ret;
}
