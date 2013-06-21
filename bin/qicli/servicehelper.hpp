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
  ServiceHelper(const qi::ObjectPtr &service=qi::ObjectPtr(), const std::string &name="");
  ServiceHelper(const ServiceHelper &other);

  int                         call(const std::string &methodName, const qi::GenericFunctionParameters &gvArgList);
  int                         post(const std::string &signalName, const qi::GenericFunctionParameters &gvArgList);
  int                         showProperty(const std::string &propertyName);
  int                         setProperty(const std::string &propertyName, const qi::GenericValue &gvArg);
  int                         watch(const std::string &signalName, bool showTime=false);

  const ServiceHelper&        operator=(const ServiceHelper &other);
  const ServiceHelper&        operator=(const qi::ObjectPtr &service);
  const std::string           &name() const;
  const qi::ObjectPtr&        objPtr() const;

  template<typename T>
  std::list<std::string>   getMatchingMembersName(const std::map<unsigned int, T> &metaMemberMap, const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingSignalsName(const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingMethodsName(const std::string &pattern, bool getHidden) const;
  std::list<std::string>   getMatchingPropertiesName(const std::string &pattern, bool getHidden) const;
private:
  qi::GenericValuePtr defaultWatcher(const ServiceHelper::WatchOptions &options, const std::vector<qi::GenericValuePtr> &params);
  bool                byPassMember(const std::string &name, unsigned int uid, bool showHidden) const;

private:
  std::string         _name;
  qi::ObjectPtr       _service;
};

#endif /* !QICLI_SERVICEHELPER_HPP_ */
