#ifndef QICLI_SESSIONHELPER_HPP_
# define QICLI_SESSIONHELPER_HPP_

# include <qimessaging/session.hpp>

# include "servicehelper.hpp"

class SessionHelper
{
private:
  qi::Session _session;
public:
  bool getServiceSync(std::string const& serviceName, ServiceHelper &out);
  void _showServiceInfo(qi::ServiceInfo const& infos, bool verbose=false);
  void showServiceInfo(std::string const& serviceName, bool verbose=false);
  void showServiceInfo(unsigned int i, bool verbose=false);
  void xShowServicesInfo(std::vector<std::string> const& patternVec, bool verbose=false);
  void showServicesInfo(std::vector<std::string> const& serviceList, bool verbose=false);
  void showServicesInfo(bool verbose=false);
  qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
  qi::FutureSync<void> close();
  qi::FutureSync< qi::ObjectPtr > service(const std::string &service, const std::string &protocol = "");
  qi::FutureSync< std::vector<qi::ServiceInfo> > services(qi::Session::ServiceLocality locality = qi::Session::ServiceLocality_All);
};

#endif /* !QICLI_SESSIONHELPER_HPP_ */
