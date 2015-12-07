#ifndef QICLI_SERVICEHELPER_HPP_
# define QICLI_SERVICEHELPER_HPP_

#include <qi/session.hpp>
#include <qi/jsoncodec.hpp>

struct WatchOptions;

class ServiceHelper
{
public:
  ServiceHelper(const qi::AnyObject &service=qi::AnyObject(), const std::string &name="", qi::JsonOption jsonPrintOption = qi::JsonOption_None);
  ServiceHelper(const ServiceHelper &other) = default;
  ServiceHelper& operator=(const ServiceHelper &other) = default;
  ServiceHelper& operator=(const qi::AnyObject &service);
  ~ServiceHelper();

  bool                        call(const std::string &methodName, const qi::GenericFunctionParameters &gvArgList, unsigned int callCount);
  bool                        post(const std::string &signalName, const qi::GenericFunctionParameters &gvArgList);
  bool                        showProperty(const std::string &propertyName);
  bool                        setProperty(const std::string &propertyName, const qi::AnyValue &gvArg);
  bool                        watch(const std::string &signalName, bool showTime=false);

  const std::string           &name() const;
  const qi::AnyObject&        objPtr() const;

  template<typename T>
  std::list<std::string>   getMatchingMembersName(const std::map<unsigned int, T> &metaMemberMap, const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingSignalsName(const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingMethodsName(const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingPropertiesName(const std::string &pattern, bool getHidden) const;
private:
  qi::AnyReference    defaultWatcher(const WatchOptions &options, const std::vector<qi::AnyReference> &params);
  bool                byPassMember(const std::string &name, unsigned int uid, bool showHidden) const;

private:
  std::string _name;
  std::list<qi::SignalLink> _signalLinks;
  qi::AnyObject _service;
  qi::JsonOption _jsonPrintOption;
};

#endif /* !QICLI_SERVICEHELPER_HPP_ */
