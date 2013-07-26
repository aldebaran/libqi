#ifndef QICLI_SERVICEHELPER_HPP_
# define QICLI_SERVICEHELPER_HPP_

#include <qimessaging/session.hpp>

class ServiceHelper
{
public:
  struct WatchOptions
  {
    std::string signalName;
    bool showTime;
    WatchOptions()
      :showTime(false)
    {}
  };

public:
  ServiceHelper(const qi::AnyObject &service=qi::AnyObject(), const std::string &name="");
  ServiceHelper(const ServiceHelper &other);
  ~ServiceHelper();

  bool                        call(const std::string &methodName, const qi::GenericFunctionParameters &gvArgList);
  bool                        post(const std::string &signalName, const qi::GenericFunctionParameters &gvArgList);
  bool                        showProperty(const std::string &propertyName);
  bool                        setProperty(const std::string &propertyName, const qi::AnyValue &gvArg);
  bool                        watch(const std::string &signalName, bool showTime=false);

  const ServiceHelper&        operator=(const ServiceHelper &other);
  const ServiceHelper&        operator=(const qi::AnyObject &service);
  const std::string           &name() const;
  const qi::AnyObject&        objPtr() const;

  template<typename T>
  std::list<std::string>   getMatchingMembersName(const std::map<unsigned int, T> &metaMemberMap, const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingSignalsName(const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingMethodsName(const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingPropertiesName(const std::string &pattern, bool getHidden) const;
private:
  qi::AnyReference    defaultWatcher(const ServiceHelper::WatchOptions &options, const std::vector<qi::AnyReference> &params);
  bool                byPassMember(const std::string &name, unsigned int uid, bool showHidden) const;

private:
  std::string         _name;
  std::list<qi::SignalLink> _signalLinks;
  qi::AnyObject       _service;
};

#endif /* !QICLI_SERVICEHELPER_HPP_ */
