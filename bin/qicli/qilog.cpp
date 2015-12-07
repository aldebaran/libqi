/**
 * Copyright (C) 2013 Aldebaran Robotics
 */

#include <string>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <qi/log.hpp>

#include <qi/applicationsession.hpp>

#include "qicli.hpp"

#define foreach BOOST_FOREACH

qiLogCategory("qicli.qilog");

static void onMessage(const qi::AnyValue& msg)
{
  qi::AnyReference ref = msg.asReference();
  std::stringstream ss;
  ss << qi::log::logLevelToString(static_cast<qi::LogLevel>(ref[1].asInt32())) // level
      << " " << ref[3].asString() // category
      << " " << ref[0].asString() // source
      << " " << ref[5].asString(); // message
  std::cout << ss.str() << std::endl;
}

static void setFilter(const std::string& rules, qi::AnyObject listener)
{
  // See doc in header for format
  size_t pos = 0;
  while (true)
  {
    if (pos >= rules.length())
      break;
    size_t next = rules.find(':', pos);
    std::string token;
    if (next == rules.npos)
      token = rules.substr(pos);
    else
      token = rules.substr(pos, next-pos);
    if (token.empty())
    {
      pos = next + 1;
      continue;
    }
    if (token[0] == '+')
      token = token.substr(1);
    size_t sep = token.find('=');
    if (sep != token.npos)
    {
      std::string sLevel = token.substr(sep+1);
      std::string cat = token.substr(0, sep);
      int level = qi::log::stringToLogLevel(sLevel.c_str());
      qiLogFatal() << cat << level;
      listener.call<void>("addFilter", cat, level);
    }
    else
    {
      if (token[0] == '-')
      {
        qiLogFatal() << token.substr(1) << 0;
        listener.call<void>("addFilter", token.substr(1), 0);
      }
      else
      {
        qiLogFatal() << token << 6;
        listener.call<void>("addFilter", token, 6);
      }
    }
    if (next == rules.npos)
      break;
    pos = next+1;
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

  qi::AnyObject logger = s->service("LogManager");
  qi::AnyObject listener = logger.call<qi::AnyObject>("getListener");

  listener.call<void>("clearFilters");

  if (vm.count("level"))
  {
    int level = vm["level"].as<int>();

    if (level > 6)
      level =  6;
    else if (level <= 0)
      level = 0;

    listener.call<void>("addFilter", "*", level);
  }

  if (vm.count("verbose"))
    listener.call<void>("addFilter", "*", 5);

  if (vm.count("debug"))
    listener.call<void>("addFilter", "*", 6);

  if (vm.count("filters"))
  {
    std::string filters = vm["filters"].as<std::string>();
    setFilter(filters, listener);
  }

  listener.connect("onLogMessage", &onMessage);

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

  qi::AnyObject logger = s->service("LogManager");

  qi::os::timeval tv(qi::SystemClock::now());

  std::string source(__FILE__);
  source += ':';
  source += __FUNCTION__;
  source += ':';
  source += boost::lexical_cast<std::string>(__LINE__);

  int level = 4;
  if (vm.count("level"))
  {
    level = vm["level"].as<int>();

    if (level > 6)
      level =  6;
    else if (level <= 0)
      level = 0;
  }
  if (vm.count("verbose"))
    level = 5;

  if (vm.count("debug"))
    level = 6;

  std::string category = "qicli.qilog.logsend";
  if (vm.count("category"))
    category = vm["category"].as<std::string>();

  std::string location = qi::os::getMachineId() + ":" + boost::lexical_cast<std::string>(qi::os::getpid());;
  std::string message = "";
  if (vm.count("message"))
    message = vm["message"].as<std::string>();

  // timestamp
  qi::AnyReferenceVector timeVectRef;
  timeVectRef.push_back(qi::AnyReference::from(tv.tv_sec));
  timeVectRef.push_back(qi::AnyReference::from(tv.tv_usec));
  qi::AnyValue timeVal = qi::AnyValue::makeTuple(timeVectRef);

  qi::AnyReferenceVector msgVectRef;
  msgVectRef.push_back(qi::AnyReference::from(source));
  msgVectRef.push_back(qi::AnyReference::from(level));
  msgVectRef.push_back(timeVal.asReference()); //timestamp
  msgVectRef.push_back(qi::AnyReference::from(category));
  msgVectRef.push_back(qi::AnyReference::from(location));
  msgVectRef.push_back(qi::AnyReference::from(message));
  msgVectRef.push_back(qi::AnyReference::from(0));

  std::vector<qi::AnyValue> msgs;
  msgs.push_back(qi::AnyValue::makeTuple(msgVectRef));
  logger.call<void>("log", qi::AnyValue::from(msgs));

  logger.reset();

  return 0;
}
