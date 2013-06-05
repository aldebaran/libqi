#include <boost/date_time.hpp>

#include <qi/future.hpp>
#include <qitype/functiontype.hpp>

#include "servicehelper.hpp"
#include "qicli.hpp"

std::ostream &operator<<(std::ostream &os, std::vector<qi::GenericValuePtr> const& gvv)
{
  for (unsigned int i = 0; i < gvv.size(); ++i)
  {
    os << qi::encodeJSON(gvv[i]);
    if (i + 1 != gvv.size())
      os << " ";
  }
  return os;
}

ServiceHelper const& ServiceHelper::operator=(qi::ObjectPtr const& service)
{
  _service = service;
  return *this;
}

qi::ObjectPtr const& ServiceHelper::objPtr() const
{
  return _service;
}

int ServiceHelper::call(std::string const& methodName, std::vector<std::string> const& argList)
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
    std::cout << "failed to call method: " << result.error() << std::endl;
    return 1;
  }
  std::cout << qi::encodeJSON(result.value()) << std::endl;
  return 0;
}

int ServiceHelper::showProp(std::string const& propName)
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

int ServiceHelper::setProp(std::string const& propName, std::string const& value)
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

qi::GenericValuePtr watcher(bool showTime, std::vector<qi::GenericValuePtr> const& params)
{
  if (showTime)
    std::cout << getTime() << " : ";
  std::cout << params << std::endl;
  return qi::GenericValuePtr();
}

int ServiceHelper::watchSignal(std::string const& signalName, bool showTime)
{
  qi::FutureSync<qi::Link> futLink = _service->connect(signalName,
                                                       qi::SignalSubscriber(
                                                         qi::makeDynamicGenericFunction(
                                                           boost::bind(watcher, showTime, _1))));

  if (futLink.hasError())
  {
    std::cerr << "error: " << futLink.error() << std::endl;
    return 1;
  }

  qi::Link link = futLink.value();

  if (link == 0)
  {
    std::cerr << "error: link value is 0" << std::endl;
    return 1;
  }

  ::getchar();
  _service->disconnect(link).wait();
  return 0;
}

int ServiceHelper::post(std::string const& signalName, std::vector<std::string> const& argList)
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
