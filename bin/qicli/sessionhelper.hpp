#ifndef QICLI_SESSIONHELPER_HPP_
# define QICLI_SESSIONHELPER_HPP_

# include <qimessaging/session.hpp>

# include "servicehelper.hpp"

class SessionHelper
{
public:
  SessionHelper(const std::string &address, bool verbose=false);
  ~SessionHelper();
  bool getServiceSync(const std::string &serviceName, ServiceHelper &out);
  void _showServiceInfo(const qi::ServiceInfo &infos, bool verbose=false, bool number=false);
  void showServiceInfo(const std::string &serviceName, bool verbose=false, bool number=false);
  void showServiceInfo(unsigned int i, bool verbose=false, bool number=false);
  void xShowServicesInfo(const std::vector<std::string> &patternVec, bool verbose=false, bool number=false);
  void showServicesInfo(const std::vector<std::string> &serviceList, bool verbose=false, bool number=false);
  void showServicesInfo(bool verbose=false, bool number=false);
  qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
  qi::FutureSync<void> close();
  qi::FutureSync< qi::ObjectPtr > service(const std::string &service, const std::string &protocol = "");
  qi::FutureSync< std::vector<qi::ServiceInfo> > services(qi::Session::ServiceLocality locality = qi::Session::ServiceLocality_All);

private:
  bool        _verbose;
  qi::Session _session;
};

#endif /* !QICLI_SESSIONHELPER_HPP_ */
