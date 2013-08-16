#include <qi/future.hpp>
#include <qi/iocolor.hpp>
#include <qitype/anyfunction.hpp>
#include <qitype/jsoncodec.hpp>
#include <boost/foreach.hpp>

#include "almemoryhelper.hpp"
#include "qicli.hpp"

ALMemoryHelper::ALMemoryHelper(qi::Session &session)
{
  try {
    _alm = session.service("ALMemory");
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("cannot fetch ALMemory: ") + e.what());
  }
}

void ALMemoryHelper::post(const std::string &pattern, const std::string &arg, bool json)
{
  std::vector<std::string> matchList = getMatchingEvents(pattern);
  for (unsigned int i = 0; i < matchList.size(); ++i)
  {
    qi::Future<void> raiseResult = _alm->call<void>("raiseMicroEvent", matchList[i], decodeArg(arg, json));
    if (raiseResult.hasError())
      throw std::runtime_error("cannot raise event \"" + matchList[i] + "\": " + raiseResult.error());
  }
}

void ALMemoryHelper::watch(const std::vector<std::string> &patternList, bool showTime)
{
  std::vector<std::string> matchList = getMatchingEvents(patternList);
  for (unsigned int i = 0; i < matchList.size(); ++i)
  {
    qi::AnyObject subscriber = _alm->call<qi::AnyObject>("subscriber", matchList[i]);

    WatchOptions options;

    options.showTime = showTime;
    options.signalName = matchList[i];
    qi::SignalSubscriber sigSub(qi::AnyFunction::fromDynamicFunction(boost::bind(&ALMemoryHelper::defaultWatcher, this, options, _1)));
    qi::FutureSync<qi::SignalLink> futLink = subscriber->connect("signal", sigSub);
    if (futLink.hasError())
      throw std::runtime_error("cannot watch event \"" + matchList[i] + "\": " + futLink.error());
    _subscriberList.push_back(subscriber);
  }
  ::getchar();
}

qi::AnyReference ALMemoryHelper::defaultWatcher(const WatchOptions &options, const std::vector<qi::AnyReference> &params)
{
  static boost::mutex m;
  std::ostringstream ss;
  if (options.showTime)
    ss << getTime() << ": ";
  ss << options.signalName << ": ";
  ss << params;
  ss << std::endl;
  {
    // gain ownership of std::cout to avoid text overlap on terminal
    boost::lock_guard<boost::mutex> lock(m);
    std::cout << ss.str();
  }
  return qi::AnyReference();
}

std::vector<std::string> ALMemoryHelper::getMatchingEvents(const std::string &pattern)
{
  std::vector<std::string> patternList(1);
  patternList[0] = pattern;
  return getMatchingEvents(patternList);
}

std::vector<std::string> ALMemoryHelper::getMatchingEvents(const std::vector<std::string> &patternList)
{
  std::vector<std::string> eventList = getEventList();
  std::set<std::string> matchSet;

  for (unsigned int i = 0; i < eventList.size(); ++i)
    for (unsigned int u = 0; u < patternList.size(); ++u)
      if (qi::os::fnmatch(patternList[u], eventList[i]))
        matchSet.insert(eventList[i]);

  if (matchSet.empty())
    throw std::runtime_error("no ALMemory events matching the given pattern were found");

  std::vector<std::string> matchList(matchSet.size());

  std::set<std::string>::const_iterator begin;
  std::set<std::string>::const_iterator end = matchSet.end();

  unsigned int i = 0;
  for (begin = matchSet.begin(); begin != end; ++begin)
  {
    matchList[i] = *begin;
    ++i;
  }
  return matchList;
}

std::vector<std::string> ALMemoryHelper::getEventList()
{
  qi::Future<std::vector<std::string> > eventListFut = _alm->call<std::vector<std::string> >("getEventList");
  if (eventListFut.hasError())
    throw std::runtime_error("cannot get ALMemory event list: " + eventListFut.error());

  return eventListFut.value();
}
