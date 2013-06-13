#include <boost/date_time.hpp>
#include <boost/regex.hpp>

#include <qi/future.hpp>
#include <qi/iocolor.hpp>
#include <qitype/functiontype.hpp>

#include "servicehelper.hpp"
#include "qicli.hpp"


ServiceHelper::ServiceHelper(const qi::ObjectPtr &service, const std::string &name)
  :_name(name),
    _service(service)

{}

ServiceHelper::ServiceHelper(const ServiceHelper &other)
  :_name(other._name),
    _service(other._service)
{}

std::ostream &operator<<(std::ostream &os, const std::vector<qi::GenericValuePtr> &gvv)
{
  for (unsigned int i = 0; i < gvv.size(); ++i)
  {
    os << qi::encodeJSON(gvv[i]);
    if (i + 1 != gvv.size())
      os << " ";
  }
  return os;
}

const ServiceHelper& ServiceHelper::operator=(const qi::ObjectPtr &service)
{
  _service = service;
  return *this;
}

const ServiceHelper& ServiceHelper::operator =(const ServiceHelper &other)
{
  _service = other._service;
  _name = other._name;
  return *this;
}

const qi::ObjectPtr& ServiceHelper::objPtr() const
{
  return _service;
}

int ServiceHelper::call(const std::string &methodName, const std::vector<std::string> &argList)
{
  qi::GenericFunctionParameters params;

  for (unsigned int i = 0; i < argList.size(); ++i)
  {
    qi::GenericValue gv = qi::decodeJSON(argList[i]);
    params.push_back(gv.clone());
  }
  qi::FutureSync<qi::GenericValuePtr> result = _service->metaCall(methodName, params);
  if (result.hasError())
  {
    std::cout << "Call error: " << result.error() << std::endl;
    return 1;
  }
  std::cout << qi::encodeJSON(result.value()) << std::endl;
  return 0;
}

int ServiceHelper::showProp(const std::string &propName)
{
  int propId = _service->metaObject().propertyId(propName);
  if (propId == -1)
  {
    std::cout << "error: can't find property: " << propName << std::endl;
    return 1;
  }
  qi::FutureSync<qi::GenericValue> result = _service->property(propId);

  if (result.hasError())
  {
    std::cout << "error: " << result.error() << std::endl;
    return 1;
  }
  std::cout << qi::encodeJSON(result.value()) << std::endl;
  return 0;
}

int ServiceHelper::setProp(const std::string &propName, const std::string &value)
{
  int propId = _service->metaObject().propertyId(propName);
  if (propId == -1)
  {
    std::cout << "error: can't find property: " << propName << std::endl;
    return 1;
  }
  qi::GenericValue gv = qi::decodeJSON(value);
  qi::FutureSync<void> result = _service->setProperty(propId, gv);

  if (result.hasError())
  {
    std::cout << "error: " << result.error() << std::endl;
    return 1;
  }
  return 0;
}

static bool bypass(const std::string &name, unsigned int uid, bool showHidden) {
  if (showHidden)
    return false;
  if (qi::MetaObject::isPrivateMember(name, uid))
    return true;
  return false;
}

std::list<qi::MetaSignal> ServiceHelper::getMetaSignalsByPattern(const std::string &pattern, bool getHidden)
{
  qi::MetaObject mo = _service->metaObject();
  qi::MetaObject::SignalMap signalMap = mo.signalMap();

  qi::MetaObject::SignalMap::const_iterator begin;
  qi::MetaObject::SignalMap::const_iterator end = signalMap.end();

  std::list<qi::MetaSignal> signalList;
  boost::basic_regex<char> reg(pattern);

  for (begin = signalMap.begin(); begin != end; ++begin)
  {
    if (bypass(begin->second.name(), begin->second.uid(), getHidden))
      continue;
    if (boost::regex_match(begin->second.name(), reg))
      signalList.push_back(begin->second);
  }
  return signalList;
}

qi::GenericValuePtr defaultWatcher(const ServiceHelper::WatchOptions &options, const std::vector<qi::GenericValuePtr> &params)
{
  static boost::mutex m;
  std::ostringstream ss;
  if (options.showTime)
    ss << getTime() << ": ";
  ss << options.serviceName << ".";
  ss << options.signalName << " : ";
  ss << params;
  ss << std::endl;
  {
    // gain ownership of std::cout to avoid text overlap on terminal
    boost::lock_guard<boost::mutex> lock(m);
    std::cout << ss.str();
  }
  return qi::GenericValuePtr();
}

int ServiceHelper::watchSignalPattern(const std::string &signalPattern, bool showTime, bool showHidden)
{
  std::list<qi::MetaSignal> toWatch = getMetaSignalsByPattern(signalPattern, showHidden);
  std::list<qi::MetaSignal>::const_iterator begin;
  std::list<qi::MetaSignal>::const_iterator end = toWatch.end();

  if (toWatch.empty())
  {
    std::cerr << "error: no signal matching the given pattern \"" << signalPattern << "\" was found in service " << _name << std::endl;
    return 1;
  }
  WatchOptions options;
  options.serviceName = _name;
  options.showTime = showTime;
  for (begin = toWatch.begin(); begin != end; ++begin)
  {
    options.signalName = begin->name();
    qi::SignalSubscriber sigSub(qi::makeDynamicGenericFunction(boost::bind(defaultWatcher, options, _1)));
    qi::FutureSync<qi::Link> futLink = _service->connect(begin->name(), sigSub);

    qi::Link link = futLink.value();

    if (link != 0)
      _links.push_back(link);
  }
  return 0;
}

void ServiceHelper::disconnectAll()
{
  std::list<qi::Link>::const_iterator begin;
  std::list<qi::Link>::const_iterator end = _links.end();

  for (begin = _links.begin(); begin != end; ++begin)
    _service->disconnect(*begin).wait();
  _links.clear();
}

int ServiceHelper::post(const std::string &signalName, const std::vector<std::string> &argList)
{
  qi::GenericFunctionParameters params;

  for (unsigned int i = 0; i < argList.size(); ++i)
    params.push_back(qi::decodeJSON(argList[i]).clone());

  qi::MetaSignal const * ms = _service->metaObject().signal(_service->metaObject().signalId(signalName));
  if (!ms)
  {
    std::cout << "error: Cannot find signal " << signalName << std::endl;
    return 1;
  }

  _service->metaPost(signalName, params);
  return 0;
}
