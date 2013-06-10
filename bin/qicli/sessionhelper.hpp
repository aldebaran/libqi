#ifndef QICLI_SESSIONHELPER_HPP_
# define QICLI_SESSIONHELPER_HPP_

# include <qimessaging/session.hpp>

# include "servicehelper.hpp"

class SessionHelper
{
public:
  SessionHelper(const std::string &address);
  ~SessionHelper();
  void showServicesInfoPattern(const std::vector<std::string> &patternVec, bool verbose, bool showHidden, bool showDoc);

  qi::FutureSync< qi::ObjectPtr > service(const std::string &service, const std::string &protocol = "");
  qi::FutureSync< std::vector<qi::ServiceInfo> > services(qi::Session::ServiceLocality locality = qi::Session::ServiceLocality_All);
  bool getServiceSync(const std::string &serviceName, ServiceHelper &out);

private:
  void showServiceInfo(const qi::ServiceInfo &infos, bool verbose, bool showHidden, bool showDoc);
  qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
  qi::FutureSync<void> close();

private:
  qi::Session _session;
};

#endif /* !QICLI_SESSIONHELPER_HPP_ */
