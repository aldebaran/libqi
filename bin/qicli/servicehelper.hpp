#ifndef QICLI_SERVICEHELPER_HPP_
# define QICLI_SERVICEHELPER_HPP_

#include <qimessaging/session.hpp>

class ServiceHelper
{
public:
  const ServiceHelper& operator=(const qi::ObjectPtr &service);
  const qi::ObjectPtr& objPtr() const;
  int call(const std::string &methodName, const std::vector<std::string> &argList);
  int post(const std::string &signalName, const std::vector<std::string> &argList);
  int showProp(const std::string &propName);
  int setProp(const std::string &propName, const std::string &value);
  int watchSignal(const std::string &signalName, bool showTime);

private:
  std::string _getTime() const;

private:
  qi::ObjectPtr _service;
};

#endif /* !QICLI_SERVICEHELPER_HPP_ */
