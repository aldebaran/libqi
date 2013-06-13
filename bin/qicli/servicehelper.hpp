#ifndef QICLI_SERVICEHELPER_HPP_
# define QICLI_SERVICEHELPER_HPP_

#include <qimessaging/session.hpp>

class ServiceHelper
{
public:
  struct WatchOptions
  {
    std::string serviceName;
    std::string signalName;
    bool showTime;
    WatchOptions()
      :showTime(false)
    {}
  };

public:
  ServiceHelper(const qi::ObjectPtr &service, const std::string &name="");
  const ServiceHelper& operator =(const ServiceHelper &other);
  ServiceHelper(const ServiceHelper &other);
  const ServiceHelper& operator=(const qi::ObjectPtr &service);
  const qi::ObjectPtr& objPtr() const;
  int call(const std::string &methodName, const std::vector<std::string> &argList);
  int post(const std::string &signalName, const std::vector<std::string> &argList);
  int showProp(const std::string &propName);
  int setProp(const std::string &propName, const std::string &value);
  int watchSignalPattern(const std::string &signalPattern, bool showTime=false, bool showHidden=false);
  void disconnectAll();

  /* UTILS */
  std::list<qi::MetaSignal> getMetaSignalsByPattern(const std::string &pattern, bool getHidden);

private:
  std::string _getTime() const;

private:
  std::string         _name;
  qi::ObjectPtr       _service;
  std::list<qi::Link> _links;
};

#endif /* !QICLI_SERVICEHELPER_HPP_ */
