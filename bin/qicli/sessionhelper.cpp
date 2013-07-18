#include <iomanip>
#include <boost/regex.hpp>
#include <qi/iocolor.hpp>
#include <boost/foreach.hpp>
#include <qitype/jsoncodec.hpp>
#include <boost/lexical_cast.hpp>

#include "sessionhelper.hpp"
#include "qicli.hpp"

SessionHelper::SessionHelper(const std::string &address)
{
  _session.connect(address);
  _servicesInfos = _session.services();
}

SessionHelper::~SessionHelper()
{
  _currentMatchMap.clear();
  _session.close();
}

void SessionHelper::info(const std::vector<std::string> &patternVec, bool verbose, bool showHidden, bool showDoc)
{
  std::set<std::string> matchServs;

  //pattern match, and display
  for (unsigned int i = 0; i < patternVec.size(); ++i)
  {
    std::list<std::string> tmp = getMatchingServices(patternVec[i], showHidden);
    matchServs.insert(tmp.begin(), tmp.end());
  }

  if (matchServs.empty())
    throw std::runtime_error("No service matching the given patterns were found");

  for (unsigned int j = 0; j < _servicesInfos.size(); ++j)
    BOOST_FOREACH(const std::string &it, matchServs)
      if (it == _servicesInfos[j].name())
        showServiceInfo(_servicesInfos[j], verbose, showHidden, showDoc);
}

void SessionHelper::call(const std::string &pattern, const std::vector<std::string> &argList, bool hidden, bool json)
{
  forEachService(pattern, boost::bind(&ServiceHelper::call, _1, _2, decodeArgs(argList, json)),
                  &ServiceHelper::getMatchingMethodsName, hidden);
}

void SessionHelper::post(const std::string &pattern, const std::vector<std::string> &argList, bool hidden, bool json)
{
  forEachService(pattern, boost::bind(&ServiceHelper::post, _1, _2, decodeArgs(argList, json)),
                  &ServiceHelper::getMatchingSignalsName, hidden);
}

void SessionHelper::get(const std::vector<std::string> &patternList, bool hidden)
{
  forEachService(patternList, boost::bind(&ServiceHelper::showProperty, _1, _2),
                  &ServiceHelper::getMatchingPropertiesName, hidden);
}

void SessionHelper::set(const std::vector<std::string> &patternList, const std::string &arg, bool hidden, bool json)
{
  forEachService(patternList, boost::bind(&ServiceHelper::setProperty, _1, _2, decodeArg(arg, json)),
                  &ServiceHelper::getMatchingPropertiesName, hidden);
}

void SessionHelper::watch(const std::vector<std::string> &patternList, bool showTime, bool hidden)
{
  forEachService(patternList, boost::bind(&ServiceHelper::watch, _1, _2, showTime),
                  &ServiceHelper::getMatchingSignalsName, hidden);
}

bool SessionHelper::byPassService(const std::string &name, bool showHidden)
{
  if (showHidden)
    return false;
  if (!name.empty() && name[0] == '_')
    return true;
  return false;
}

ServiceHelper SessionHelper::getServiceHelper(const std::string &serviceName)
{
  qi::FutureSync<qi::AnyObject> future = _session.service(serviceName);

  if (future.hasError())
    throw std::runtime_error(future.error());
  return ServiceHelper(future.value(), serviceName);
}

std::list<std::string> SessionHelper::getMatchingServices(const std::string &pattern, bool getHidden)
{
  std::list<std::string>         matchingServices;

  for (unsigned int i = 0; i < _servicesInfos.size(); ++i)
  {
    if ((isNumber(pattern) && static_cast<unsigned int>(::atoi(pattern.c_str())) == _servicesInfos[i].serviceId())
        || _servicesInfos[i].name() == pattern)
    {
      matchingServices.push_back(_servicesInfos[i].name());
      return matchingServices;
    }
    if (byPassService(_servicesInfos[i].name(), getHidden))
      continue;
    if (qi::os::fnmatch(pattern, _servicesInfos[i].name()))
      matchingServices.push_back(_servicesInfos[i].name());
  }
  return matchingServices;
}

SessionHelper::MatchMap SessionHelper::getMatchMap(const std::vector<std::string> &patternList, ShPatternResolver patternResolver, bool hidden)
{
  MatchMap matchMap;

  for (unsigned int i = 0; i < patternList.size(); ++i)
  {
    std::string servicePattern;
    std::string memberPattern;

    splitName(patternList[i], servicePattern, memberPattern, true);
    std::list<std::string> matchingServices = getMatchingServices(servicePattern, hidden);

    BOOST_FOREACH(const std::string &it, matchingServices)
    {
      if (!matchMap.count(it))
        matchMap[it].first = getServiceHelper(it);

      std::pair<ServiceHelper, std::set<std::string> > &matchPair = matchMap[it];
      std::list<std::string> matchingMembers = (matchPair.first.*patternResolver)(memberPattern, hidden);
      matchMap[it].second.insert(matchingMembers.begin(), matchingMembers.end());
    }
  }
  return matchMap;
}

void SessionHelper::showServiceInfo(const qi::ServiceInfo &infos, bool verbose, bool showHidden, bool showDoc)
{
  std::cout << qi::StreamColor_Fuchsia
            << std::right << std::setw(3) << std::setfill('0')
            << infos.serviceId() << qi::StreamColor_Reset
            << std::left << std::setw(0) << std::setfill(' ')
            << " [" << qi::StreamColor_Red << infos.name() << qi::StreamColor_Reset << "]" << std::endl;

  if (!verbose)
    return;

  std::string firstEp;
  if (infos.endpoints().begin() != infos.endpoints().end())
    firstEp = infos.endpoints().begin()->str();
  std::cout << qi::StreamColor_Green << "  * " << qi::StreamColor_Fuchsia << "Info" << qi::StreamColor_Reset << ":"
            << std::endl;

  std::cout << "   machine   " << infos.machineId() << std::endl
            << "   process   " << infos.processId() << std::endl
            << "   endpoints " << firstEp << std::endl;

  for (qi::UrlVector::const_iterator it_urls = infos.endpoints().begin(); it_urls != infos.endpoints().end(); ++it_urls) {
    if (it_urls != infos.endpoints().begin())
      std::cout << "             " << it_urls->str() << std::endl;
  }

  try
  {
    ServiceHelper service = getServiceHelper(infos.name());
    qi::details::printMetaObject(std::cout, service.objPtr()->metaObject(), true, showHidden, showDoc);
  }
  catch (...)
  {
    std::cout << "  Can't connect to service " << infos.name();
    return;
  }
}

void SessionHelper::forEachService(const std::string &pattern, ShMethod methodToCall, ShPatternResolver patternResolver, bool hidden)
{
  std::vector<std::string> tmpPatternList(1);
  tmpPatternList[0] = pattern;
  forEachService(tmpPatternList, methodToCall, patternResolver, hidden);
}

void SessionHelper::forEachService(const std::vector<std::string> &patternList, ShMethod methodToCall, ShPatternResolver patternResolver, bool hidden)
{
  _currentMatchMap = getMatchMap(patternList, patternResolver, hidden);

  if (_currentMatchMap.empty())
    throw std::runtime_error("no services matching the given pattern(s) were found");

  bool foundOne = false;
  BOOST_FOREACH(MatchMapPair &it, _currentMatchMap)
  {
    if (!it.second.second.empty())
      foundOne = true;

    BOOST_FOREACH(const std::string &it2, it.second.second)
      methodToCall(it.second.first, it2);
  }
  if (!foundOne)
    throw std::runtime_error("no Service.Member combinaisons matching the given pattern(s) were found");
}

bool SessionHelper::splitName(const std::string &fullName, std::string &beforePoint, std::string &afterPoint, bool throwOnError)
{
  size_t pos = fullName.find(".");

  if (pos == std::string::npos || pos == fullName.length() - 1 || pos == 0)
  {
    if (throwOnError)
      throw (std::runtime_error("Service.Member syntax not respected"));
    else
      return false;
  }

  beforePoint = fullName.substr(0, pos);
  afterPoint = fullName.substr(pos + 1);
  return true;
}

qi::AnyValue SessionHelper::decodeArgByCast(const std::string &arg)
{
  try {
    return qi::AnyValue(boost::lexical_cast<qi::int64_t>(arg));
  } catch (const boost::bad_lexical_cast &) {
    try {
      return qi::AnyValue(boost::lexical_cast<double>(arg));
    } catch (const boost::bad_lexical_cast &) {
      try {
        return qi::AnyValue(boost::lexical_cast<std::string>(arg));
      } catch (const boost::bad_lexical_cast &) {
        throw std::runtime_error("error while decoding arg");
      }
    }
  }
}

qi::AnyValue SessionHelper::decodeArg(const std::string &arg, bool json)
{
  if (json)
    return qi::decodeJSON(arg);
  else
    return decodeArgByCast(arg);
}

qi::GenericFunctionParameters SessionHelper::decodeArgs(const std::vector<std::string> &argList, bool json)
{
  qi::GenericFunctionParameters gvArgList;

  for (unsigned int i = 0; i < argList.size(); ++i)
  {
    qi::AnyValue gvArg = decodeArg(argList[i], json);
    gvArgList.push_back(gvArg.clone());
  }
  return gvArgList;
}
