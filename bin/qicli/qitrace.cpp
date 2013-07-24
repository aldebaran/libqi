/** Copyright (C) 2012 Aldebaran Robotics
*/


#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>
#include <qitype/jsoncodec.hpp>


#include "qicli.hpp"

#define foreach BOOST_FOREACH


static const char* callType[] = {
  "?", "C", "R", "E", "S"
};
typedef std::map<std::string, qi::AnyObject> ObjectMap;
static ObjectMap objectMap;

static bool numeric = false;
static bool printMo = false;
static bool disableTrace = false;
static bool traceState = false;
static bool cleaned = false;
static bool full = false;

static std::vector<std::string> objectNames;
static unsigned int maxServiceLength = 0;
qiLogCategory("qitrace");

void onTrace(ObjectMap::value_type ov, const qi::EventTrace& trace)
{
  static qi::int64_t secStart = 0;
  if (!secStart && !full)
    secStart = trace.timestamp().tv_sec;
  static unsigned int maxLen = 0;
  std::string name = boost::lexical_cast<std::string>(trace.slotId());
  if (!numeric)
  {
    qi::MetaObject mo = ov.second->metaObject();
    qi::MetaMethod* m = mo.method(trace.slotId());
    if (m)
      name = m->name();
    else
    {
      qi::MetaSignal* s = mo.signal(trace.slotId());
      if (s)
        name = s->name();
      else
        name = name + "(??" ")"; // trigraph protect mode on
    }
  }
  if (!full && name.size() > 25)
  {
    name = name.substr(0, 22) + "...";
  }
  std::string serviceName = ov.first;
  if (!full && serviceName.size() > 17)
    serviceName = serviceName.substr(0, 14) + "...";
  maxLen = std::max(maxLen, (unsigned int)name.size());
  unsigned int traceKind = trace.kind();
  if (traceKind > 4)
    traceKind = 0;
  std::string spacing(maxLen + 2 - name.size(), ' ');
  std::string spacing2((full?maxServiceLength:17) + 2 - ov.first.size(), ' ');
  if (trace.kind() == qi::EventTrace::Event_Result)
  {
    std::cout << serviceName << spacing2 << trace.id() << ' ' << callType[traceKind] << ' ' << name
      << spacing << (trace.timestamp().tv_sec - secStart) << '.' << trace.timestamp().tv_usec
      << ' ' << trace.userUsTime() << ' ' << trace.systemUsTime() << ' ' << qi::encodeJSON(trace.arguments()) << std::endl;
  }
  else
  {
    std::cout << serviceName << spacing2 << trace.id() << ' ' << callType[traceKind] << ' ' << name
      << spacing << (trace.timestamp().tv_sec - secStart) << '.' << trace.timestamp().tv_usec
      << ' ' << qi::encodeJSON(trace.arguments()) << std::endl;
  }
}

int subCmd_trace(int argc, char **argv, const MainOptions &options)
{
  po::options_description     desc("Usage: qicli trace [-o services]");
  std::vector<std::string>    serviceList;

  desc.add_options()
  ("numeric,n", po::bool_switch(&numeric), "Do not resolve slot Ids to names")
  ("full,f", po::bool_switch(&full), "Do not abreviate anything")
  ("object,o", po::value<std::vector<std::string> >(&objectNames), "Object(s) to monitor, specify multiple times, comma-separate, use '*' for all, use '-globPattern' to remove from list")
  ("print,p", po::bool_switch(&printMo), "Print out the Metaobject and exit")
  ("disable,d", po::bool_switch(&disableTrace), "Disable trace on objects and exit")
  ("trace-status", po::bool_switch(&traceState), "Show trace status on objects and exit");

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc), vm, desc))
    return 1;

  qi::Session s;

  qiLogVerbose() << "Connecting to sd";
  s.connect(options.address);
  qiLogVerbose() << "Resolving services";

  std::vector<std::string> allServices;
  std::vector<qi::ServiceInfo> si = s.services();
  for (unsigned i=0; i<si.size(); ++i)
    allServices.push_back(si[i].name());
  std::vector<std::string> services = parseServiceList(objectNames, allServices);

  std::vector<std::string> servicesOk;
  qiLogVerbose() << "Fetching services: " << boost::join(services, ",");
  // access services
  for (unsigned i=0; i<services.size(); ++i)
  {
    qi::AnyObject o;
    try
    {
      o = s.service(services[i]);
    }
    catch (const std::exception& e)
    {
      qiLogError() << "Error fetching " << services[i] << " : " << e.what();
      services[i] = "";
      continue;
    }
    if (!o)
    {
      qiLogError() << "Error fetching " << services[i];
      services[i] = "";
      continue;
    }
    objectMap[services[i]] = o;
    servicesOk.push_back(services[i]);
    if (printMo)
    {
      std::cout << "\n\n" << services[i] << "\n";
      qi::details::printMetaObject(std::cout, o->metaObject());
    }
    if (disableTrace)
    {
      try
      {
        o->call<void>("enableTrace", false);
      }
      catch(...)
      {}
    }
    if (traceState)
    {
      try
      {
        bool s = o->call<bool>("isTraceEnabled");
        std::cout << services[i] << ": " << s << std::endl;
      }
      catch(...)
      {}
    }
  }

  if (printMo || disableTrace || traceState || objectMap.empty())
    return 0;

  qiLogVerbose() << "Monitoring services: " << boost::join(servicesOk, ",");
  foreach(ObjectMap::value_type& ov, objectMap)
  {
    maxServiceLength = std::max(maxServiceLength, (unsigned int)ov.first.size());
    ov.second->connect("traceObject", (boost::function<void(qi::EventTrace)>)
      boost::bind(&onTrace, ov, _1)).async();
  }
  qi::Application::run();
  while (!cleaned)
    qi::os::msleep(20);
  return 0;
}
