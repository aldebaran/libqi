/** Copyright (C) 2013 Aldebaran Robotics
*/


#include <iomanip>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>


#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

#include "qicli.hpp"

#define foreach BOOST_FOREACH

qiLogCategory("qicli.qitop");

static std::string sdUrl;
static std::vector<std::string> objectNames;
static bool full = false;
static bool numeric = false;
static float interval;

static unsigned int maxServiceLength = 0;

typedef std::map<std::string, qi::AnyObject> ObjectMap;

static ObjectMap objectMap;


// ((modulename, methodname), stats)
typedef std::pair<std::pair<std::string, std::string>, qi::MethodStatistics> Stat;
typedef std::vector<Stat> Stats;

// When sorting use cumulated user+system time
struct StatCompare
{
  bool operator()(const Stat& a, const Stat& b)
  {
    return a.second.user().cumulatedValue() + a.second.system().cumulatedValue() <
    b.second.user().cumulatedValue() + b.second.system().cumulatedValue();
  }
};

// Helper for formatting all stats values
class US
{
public:
  US() : v(0) {}
  US(qi::int64_t v) : v(v) {}
  qi::int64_t v;
};

std::ostream& operator << (std::ostream& o, const US& v)
{
  o.width(5);
  o << v.v;
  o.width(0);
  return o;
}

US us(float v)
{
  return US(v * 1000000.0);
}

void main_loop()
{

  while (true)
  {
    unsigned int maxLen = 4;
    // Reset all counters
    foreach(ObjectMap::value_type& ov, objectMap)
      ov.second->call<void>("clearStats");

    qi::os::msleep(interval * 1000);

    Stats stats;
    // Fetch stats from monitored objects and fill stats
    foreach(ObjectMap::value_type& ov, objectMap)
    {
      qi::ObjectStatistics os = ov.second->call<qi::ObjectStatistics>("stats");
      const std::string& serviceName = ov.first;
      foreach(qi::ObjectStatistics::value_type& s, os)
      {
        if (s.first == 83)
          continue; // hide clearstats
        std::string name = boost::lexical_cast<std::string>(s.first);
        if (!numeric)
        {
          qi::MetaObject mo = ov.second->metaObject();
          qi::MetaMethod* m = mo.method(s.first);
          if (m)
            name = m->name();
          else
          {
            qi::MetaSignal* sig = mo.signal(s.first);
            if (sig)
              name = sig->name();
            else
              name = name + "(??" ")"; // trigraph protect mode on
          }
        }
        maxLen = std::max(maxLen, (unsigned int)name.size());
        stats.push_back(std::make_pair(std::make_pair(serviceName, name), s.second));
      }
    }
    if (!full)
      maxLen = std::min(maxLen, 25u);
    // Now, sort
    std::sort(stats.begin(), stats.end(), StatCompare());
    // display
    std::cout << "MODULE" << std::string(maxLen + 2 - 6, ' ')
     << "METHOD" << std::string((full?maxServiceLength:17) + 2 - 6, ' ')
     << "  %CPU  " << "COUNT         "
     << "USER" << std::string(6*3 - 2, ' ')
     << "SYS " << std::string(6*3 - 2, ' ')
     << "WALL"
     << std::endl;
    foreach(const Stat& s, stats)
    {
      const qi::MethodStatistics& ms = s.second;
      std::string serviceName = s.first.first;
      std::string methodName = s.first.second;
      if (!full && serviceName.size() > 17)
        serviceName = serviceName.substr(0, 14) + "...";
      if (!full && methodName.size() > 25)
        methodName = methodName.substr(0, 22) + "...";
      std::string spacing(maxLen + 2 - methodName.size(), ' ');
      std::string spacing2((full?maxServiceLength:17) + 2 - serviceName.size(), ' ');

      std::cout << serviceName << spacing2 << methodName << spacing
        << std::setw(6) << (((ms.user().cumulatedValue() + ms.system().cumulatedValue()) * 100) / interval) << "% "
        << std::setw(3)
        << ms.count() << "x  "
        << us(ms.user().cumulatedValue() / (float)ms.count()) << ' ' << us(ms.user().minValue()) << ' ' << us(ms.user().maxValue())
        << "   "
        << us(ms.system().cumulatedValue() / (float)ms.count()) << ' ' << us(ms.system().minValue()) << ' ' << us(ms.system().maxValue())
        << "   "
        << us(ms.wall().cumulatedValue() / (float)ms.count()) << ' ' << us(ms.wall().minValue()) << ' ' << us(ms.wall().maxValue())
        << "   "
        << std::endl;
    }
  }
}

int subCmd_top(int argc, char **argv, const MainOptions &options)
{
  po::options_description     desc("Usage: qicli top [-i interval] [<ServicePattern>..]");
  std::vector<std::string>    serviceList;

  desc.add_options()
  ("numeric,n", po::bool_switch(&numeric), "Do not resolve slot Ids to names")
  ("full,f", po::bool_switch(&full), "Do not abreviate anything")
  ("service-directory,s", po::value<std::string>(&sdUrl), "url to connect to")
  ("service,s", po::value<std::vector<std::string> >(&objectNames), "Object(s) to monitor, specify multiple times, comma-separate, use '*' for all, use '-globPattern' to remove from list")
  ("interval,i", po::value<float>(&interval)->default_value(1), "Poll interval in seconds");

  po::positional_options_description positionalOptions;
  positionalOptions.add("service", -1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;


  qi::Session s;
  qiLogVerbose() << "Connecting to sd";
  s.connect(options.address);
  qiLogVerbose() << "Resolving services";
  // resolve target service names
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
  }

  if (objectMap.empty())
    return 0;

  qiLogVerbose() << "Monitoring services: " << boost::join(servicesOk, ",");
  foreach(ObjectMap::value_type& ov, objectMap)
  {
    maxServiceLength = std::max(maxServiceLength, (unsigned int)ov.first.size());
    ov.second->call<void>("enableStats", true).async();
  }
  boost::thread t(&main_loop);
  qi::Application::atStop(boost::bind(&boost::thread::interrupt, boost::ref(t)));
  qi::Application::run();
  t.join();
  qiLogInfo() << "Disabling statistics gathering..." << std::endl;
  std::vector<qi::Future<void> > futures;
  foreach(ObjectMap::value_type& ov, objectMap)
  {
    futures.push_back(ov.second->call<void>("enableStats", false));
  }
  qi::waitForAll(futures);
  return 0;
}
