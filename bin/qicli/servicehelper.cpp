#include <boost/date_time.hpp>
#include <boost/regex.hpp>

#include <qi/future.hpp>
#include <qi/iocolor.hpp>
#include <qitype/anyfunction.hpp>
#include <qitype/jsoncodec.hpp>
#include <boost/foreach.hpp>

#include "servicehelper.hpp"
#include "qicli.hpp"


ServiceHelper::ServiceHelper(const qi::AnyObject& service, const std::string &name)
  :_name(name),
    _service(service)
{}

ServiceHelper::ServiceHelper(const ServiceHelper &other)
  :_name(other._name),
    _service(other._service)
{}

ServiceHelper::~ServiceHelper()
{
  BOOST_FOREACH(qi::SignalLink link, _signalLinks)
  {
    if (link != qi::SignalBase::invalidSignalLink)
    {
      qi::FutureSync<void> fut = _service->disconnect(link);
      if (fut.hasError())
        printError("cannot disconnect signal: " + fut.error());
    }
    else
      printError("cannot disconnect signal: invalid link");
  }
}

template<typename T>
std::list<std::string> ServiceHelper::getMatchingMembersName(const std::map<unsigned int, T> &metaMemberMap, const std::string &pattern, bool getHidden) const
{
  std::list<std::string> metaMemberVec;

  if (isNumber(pattern))
  {
    unsigned int uid = ::atoi(pattern.c_str());
    if (metaMemberMap.count(uid))
      metaMemberVec.push_back(metaMemberMap.find(uid)->second.name());
    return metaMemberVec;
  }
  std::pair<unsigned int, T> it;
  BOOST_FOREACH(it, metaMemberMap)
  {
    if (it.second.name() == pattern)
    {
      metaMemberVec.push_back(it.second.name());
      return metaMemberVec;
    }
    if (byPassMember(it.second.name(), it.second.uid(), getHidden))
      continue;
    if (qi::os::fnmatch(pattern, it.second.name()))
      metaMemberVec.push_back(it.second.name());
  }
  return metaMemberVec;
}

std::list<std::string> ServiceHelper::getMatchingSignalsName(const std::string &pattern, bool getHidden) const
{
  return getMatchingMembersName<qi::MetaSignal>(_service->metaObject().signalMap(), pattern, getHidden);
}

std::list<std::string> ServiceHelper::getMatchingMethodsName(const std::string &pattern, bool getHidden) const
{
  return getMatchingMembersName<qi::MetaMethod>(_service->metaObject().methodMap(), pattern, getHidden);
}

std::list<std::string> ServiceHelper::getMatchingPropertiesName(const std::string &pattern, bool getHidden) const
{
  return getMatchingMembersName<qi::MetaProperty>(_service->metaObject().propertyMap(), pattern, getHidden);
}

const ServiceHelper& ServiceHelper::operator=(const qi::AnyObject &service)
{
  _service = service;
  return *this;
}

const ServiceHelper& ServiceHelper::operator=(const ServiceHelper &other)
{
  _service = other._service;
  _name = other._name;
  return *this;
}

const std::string &ServiceHelper::name() const
{
  return _name;
}

const qi::AnyObject& ServiceHelper::objPtr() const
{
  return _service;
}

bool ServiceHelper::showProperty(const std::string &propertyName)
{
  printServiceMember(_name, propertyName);
  int propertyId = _service->metaObject().propertyId(propertyName);
  if (propertyId == -1)
  {
    printError("property not found");
    return false;
  }
  qi::FutureSync<qi::AnyValue> result = _service->property(propertyId);

  if (result.hasError())
  {
    printError(result.error());
    return false;
  }
  std::cout << qi::encodeJSON(result.value()) << std::endl;
  return true;
}

bool ServiceHelper::setProperty(const std::string &propertyName, const qi::AnyValue& gvArg)
{
  printServiceMember(_name, propertyName);
  qi::FutureSync<void> result = _service->setProperty(propertyName, gvArg);

  if (result.hasError())
  {
    printError(result.error());
    return false;
  }
  printSuccess();
  return true;
}

bool ServiceHelper::watch(const std::string &signalName, bool showTime)
{
  WatchOptions options;
  options.showTime = showTime;
  options.signalName = signalName;
  qi::SignalSubscriber sigSub(qi::AnyFunction::fromDynamicFunction(boost::bind(&ServiceHelper::defaultWatcher, this, options, _1)));
  qi::FutureSync<qi::SignalLink> futLink = _service->connect(signalName, sigSub);
  if (futLink.hasError())
  {
    printServiceMember(_name, signalName);
    printError(futLink.error());
    return false;
  }
  _signalLinks.push_back(futLink.value());
  return true;
}

bool ServiceHelper::post(const std::string &signalName, const qi::GenericFunctionParameters &gvArgList)
{
  printServiceMember(_name, signalName);
  std::cout.flush();
  _service->metaPost(signalName, gvArgList);
  printSuccess();
  return true;
}

bool ServiceHelper::call(const std::string &methodName, const qi::GenericFunctionParameters &gvArgList)
{
  printServiceMember(_name, methodName);
  std::cout.flush();
  qi::FutureSync<qi::AnyReference> result = _service->metaCall(methodName, gvArgList);
  if (result.hasError())
  {
    printError(result.error());
    return false;
  }
  std::cout << qi::encodeJSON(result.value()) << std::endl;
  return true;
}

qi::AnyReference ServiceHelper::defaultWatcher(const WatchOptions &options, const std::vector<qi::AnyReference> &params)
{
  static boost::mutex m;
  std::ostringstream ss;
  if (options.showTime)
    ss << getTime() << ": ";
  printServiceMember(_name, options.signalName);
  ss << params;
  ss << std::endl;
  {
    // gain ownership of std::cout to avoid text overlap on terminal
    boost::lock_guard<boost::mutex> lock(m);
    std::cout << ss.str();
  }
  return qi::AnyReference();
}

bool ServiceHelper::byPassMember(const std::string &name, unsigned int uid, bool showHidden) const
{
  if (showHidden)
    return false;
  if (qi::MetaObject::isPrivateMember(name, uid))
    return true;
  return false;
}
