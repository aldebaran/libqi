#ifndef QICLI_SERVICEHELPER_HPP_
# define QICLI_SERVICEHELPER_HPP_

#include <qimessaging/session.hpp>

class ServiceHelper
{
private:
  qi::ObjectPtr _service;

public:
  ServiceHelper const& operator=(qi::ObjectPtr const& service);
  qi::ObjectPtr const& objPtr() const;
  int call(std::string const& methodName, std::vector<std::string> const& argList);
  int post(std::string const& signalName, std::vector<std::string> const& argList);
  int showProp(std::string const& propName);
  int setProp(std::string const& propName, std::string const& value);
  int watchSignal(std::string const& signalName, bool showTime);

private:
  std::string _getTime() const;
};

#endif /* !QICLI_SERVICEHELPER_HPP_ */
