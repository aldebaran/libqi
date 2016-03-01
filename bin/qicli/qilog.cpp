/**
 * Copyright (C) 2013 Aldebaran Robotics
 */

#include <string>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <qi/log.hpp>
#include <qi/detail/log.hxx>

#include <qi/applicationsession.hpp>
#include <qi/anymodule.hpp>
#include <qicore/logmessage.hpp>
#include <qicore/logmanager.hpp>
#include <qicore/loglistener.hpp>

#include "qicli.hpp"

#define foreach BOOST_FOREACH

qiLogCategory("qicli.qilog");

static void onMessage(const qi::LogMessage& msg)
{
  std::stringstream ss;
  ss << qi::log::logLevelToString(static_cast<qi::LogLevel>(msg.level))
     << " " << msg.category
     << " " << msg.source
     << " " << msg.message;
  std::cout << ss.str() << std::endl;
}

static void setFilter(const std::string& rules, qi::LogListenerPtr listener)
{
  std::string cat;
  qi::LogLevel level;
  for (auto &&p: qi::log::detail::parseFilterRules(rules))
  {
    std::tie(cat, level) = std::move(p);
    listener->addFilter(cat, level);
  }
}

int subCmd_logView(int argc, char **argv, qi::ApplicationSession& app)
{
  po::options_description   desc("Usage: qicli log-view");

  desc.add_options()
      ("help,h", "Print this help message and exit")
      ("verbose,v", "Set maximum logs verbosity shown to verbose.")
      ("debug,d", "Set maximum logs verbosity shown to debug.")
      ("level,l", po::value<int>()->default_value(4), "Change the log minimum level: [0-6] (default:4). This option accepts the same arguments' format than --qi-log-level.")
      ("filters,f", po::value<std::string>(), "Set log filtering options. This option accepts the same arguments' format than --qi-log-filters.")
      ;

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc), vm, desc))
    return 1;

  qiLogVerbose() << "Connecting to service directory";
  app.startSession();
  qi::SessionPtr s = app.session();

  qiLogVerbose() << "Resolving services";

  app.loadModule("qicore");
  qi::LogManagerPtr logger = app.session()->service("LogManager");
  qi::LogListenerPtr listener = logger->createListener();
  listener->clearFilters();

  if (vm.count("level"))
  {
    int level = vm["level"].as<int>();

    if (level > 6)
      level =  6;
    else if (level <= 0)
      level = 0;

    listener->addFilter("*", static_cast<qi::LogLevel>(level));
  }

  if (vm.count("verbose"))
    listener->addFilter("*", qi::LogLevel_Verbose);

  if (vm.count("debug"))
    listener->addFilter("*", qi::LogLevel_Debug);

  if (vm.count("filters"))
  {
    std::string filters = vm["filters"].as<std::string>();
    setFilter(filters, listener);
  }

  listener->onLogMessage.connect(&onMessage);
  app.run();

  return 0;
}

int subCmd_logSend(int argc, char **argv, qi::ApplicationSession& app)
{
  po::options_description   desc("Usage: qicli log-send <message>");

  desc.add_options()
      ("help,h", "Print this help message and exit")
      ("verbose,v", "Set sent message verbosity to verbose.")
      ("debug,d", "Set sent message verbosity to debug.")
      ("level,l", po::value<int>()->default_value(4), "Change the log minimum level: [0-6] (default:4). This option accepts the same arguments' format than --qi-log-level.")
      ("category,c", po::value<std::string>(), "Message's category (default: \"qicli.qilog.logsend\").")
      ("message,m", po::value<std::string>(), "Message to send.")
      ;

  po::positional_options_description positionalOptions;
  positionalOptions.add("message", -1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv)
                 .options(desc).positional(positionalOptions), vm, desc))
    return 1;

  qiLogVerbose() << "Connecting to service directory";
  app.startSession();
  qi::SessionPtr s = app.session();

  qiLogVerbose() << "Resolving services";

  // import module
  qi::AnyModule mod = qi::import("qicore");

  // get service Logger
  qi::LogManagerPtr logger = app.session()->service("LogManager");

  qi::LogMessage msg;

  msg.source = __FILE__;
  msg.source += ':';
  msg.source += __FUNCTION__;
  msg.source += ':';
  msg.source += boost::lexical_cast<std::string>(__LINE__);

  msg.level = qi::LogLevel_Info;
  if (vm.count("level"))
  {
    int level = vm["level"].as<int>();

    if (level > 6)
      level = qi::LogLevel_Debug;
    else if (level <= 0)
      level = qi::LogLevel_Silent;

    msg.level = static_cast<qi::LogLevel>(level);
  }
  if (vm.count("verbose"))
    msg.level = qi::LogLevel_Verbose;

  if (vm.count("debug"))
    msg.level = qi::LogLevel_Debug;

  msg.category = "qicli.qilog.logsend";
  if (vm.count("category"))
    msg.category = vm["category"].as<std::string>();

  msg.location = qi::os::getMachineId() + ":" + boost::lexical_cast<std::string>(qi::os::getpid());

  if (vm.count("message"))
    msg.message = vm["message"].as<std::string>();

  msg.date = qi::Clock::now();
  msg.systemDate = qi::SystemClock::now();

  std::vector<qi::LogMessage> msgs;
  msgs.push_back(msg);
  logger->log(msgs);

  logger.reset();

  return 0;
}
