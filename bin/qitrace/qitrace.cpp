/** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _WIN32
#include <fnmatch.h>
#else
# include <shlwapi.h>
# pragma comment(lib, "shlwapi.lib")
#endif

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

#define foreach BOOST_FOREACH


inline bool globMatch(const std::string& pattern, const std::string& string)
{
#ifdef _WIN32
      return ::PathMatchSpec(string.c_str(), pattern.c_str()) == TRUE;
#else
      return !fnmatch(pattern.c_str(), string.c_str(), 0);
#endif
}

const char* callType[] = {
  "?", "C", "R", "E", "S"
};
typedef std::map<std::string, qi::ObjectPtr> ObjectMap;
ObjectMap objectMap;

bool numeric = false;
bool printMo = false;
bool disableTrace = false;
bool traceState = false;
bool cleaned = false;
std::string sdUrl;
std::vector<std::string> objectNames;
unsigned int maxServiceLength = 0;
qiLogCategory("qitrace");

void onTrace(ObjectMap::value_type ov, const qi::EventTrace& trace)
{
  static unsigned int maxLen = 0;
  std::string name = boost::lexical_cast<std::string>(trace.slotId);
  if (!numeric)
  {
    qi::MetaObject mo = ov.second->metaObject();
    qi::MetaMethod* m = mo.method(trace.slotId);
    if (m)
      name = m->name();
    else
    {
      qi::MetaSignal* s = mo.signal(trace.slotId);
      if (s)
        name = s->name();
      else
        name = name + "(??" ")"; // trigraph protect mode on
    }
  }
  maxLen = std::max(maxLen, (unsigned int)name.size());
  unsigned int traceKind = trace.kind;
  if (traceKind > 4)
    traceKind = 0;
  std::string spacing(maxLen + 8 - name.size(), ' ');
  std::string spacing2(maxServiceLength + 8 - ov.first.size(), ' ');
  std::cout << ov.first << spacing2 << trace.id << '\t' << callType[traceKind] << ' ' << name
   << spacing << trace.timestamp.tv_sec << '.' << trace.timestamp.tv_usec
   << "\t" << qi::encodeJSON(trace.arguments) << std::endl;
}

_QI_COMMAND_LINE_OPTIONS(
  "Qitrace options",
  ("numeric,n", bool_switch(&numeric), "Do not resolve slot Ids to names")
  ("service-directory,s", value<std::string>(&sdUrl), "url to connect to")
  ("object,o", value<std::vector<std::string> >(&objectNames), "Object(s) to monitor, specify multiple times, comma-separate, use '*' for all, use '-globPattern' to remove from list")
  ("print,p", bool_switch(&printMo), "Print out the Metaobject and exit")
  ("disable,d", bool_switch(&disableTrace), "Disable trace on objects and exit")
  ("trace-status", bool_switch(&traceState), "Show trace status on objects and exit")
  );

int main(int argc, char** argv)
{
  qi::Application app(argc, argv);
  if (std::find(argv + 1, argv + argc, std::string("-h"))-argv < argc
    ||  std::find(argv + 1, argv + argc, std::string("--help"))-argv < argc)
    return 0; // Fixme have Application report that!
  qi::Session s;
  if (sdUrl.empty())
    sdUrl = "localhost";
  qi::Url url(sdUrl, "tcp", 9559);
  qiLogVerbose() << "Connecting to sd";
  s.connect(url);
  qiLogVerbose() << "Resolving services";
  // resolve target service names
  std::vector<std::string> services;
  for (unsigned i=0; i<objectNames.size(); ++i)
  {

    std::vector<std::string> split;
    boost::split(split, objectNames[i], boost::algorithm::is_any_of(","));
    for (unsigned i=0; i<split.size(); ++i)
    {
      if (split[i].empty())
        continue;
      else if (split[i] == "*")
      {
        std::vector<qi::ServiceInfo> si = s.services();
        for (unsigned i=0; i<si.size(); ++i)
          services.push_back(si[i].name());
      }
      else if (split[i][0] == '-')
      {
        std::string pattern = split[i].substr(1);
        for (unsigned i=0; i<services.size(); ++i)
        {
          if (globMatch(pattern, services[i]))
          {
            services[i] = services[services.size() - 1];
            services.pop_back();
            --i; // don't forget to check the new element we juste swaped in
          }
        }
      }
      else
        services.push_back(split[i]);
    }

  }
  std::vector<std::string> servicesOk;
  qiLogVerbose() << "Fetching services: " << boost::join(services, ",");
  // access services
  for (unsigned i=0; i<services.size(); ++i)
  {
    qi::ObjectPtr o;
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
  app.run();
  while (!cleaned)
    qi::os::msleep(20);
  return 0;
}
