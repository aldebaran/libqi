#ifndef QICLI_SESSIONHELPER_HPP_
# define QICLI_SESSIONHELPER_HPP_

# include <qimessaging/session.hpp>

# include "servicehelper.hpp"

class SessionHelper
{
private:
  bool        _verbose;
  qi::Session _session;
public:
  SessionHelper(std::string const& address, bool verbose=false);
  ~SessionHelper();
  bool getServiceSync(std::string const& serviceName, ServiceHelper &out);
  void _showServiceInfo(qi::ServiceInfo const& infos, bool verbose=false, bool number=false);
  void showServiceInfo(std::string const& serviceName, bool verbose=false, bool number=false);
  void showServiceInfo(unsigned int i, bool verbose=false, bool number=false);
  void xShowServicesInfo(std::vector<std::string> const& patternVec, bool verbose=false, bool number=false);
  void showServicesInfo(std::vector<std::string> const& serviceList, bool verbose=false, bool number=false);
  void showServicesInfo(bool verbose=false, bool number=false);
  qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
  qi::FutureSync<void> close();
  qi::FutureSync< qi::ObjectPtr > service(const std::string &service, const std::string &protocol = "");
  qi::FutureSync< std::vector<qi::ServiceInfo> > services(qi::Session::ServiceLocality locality = qi::Session::ServiceLocality_All);
};

#endif /* !QICLI_SESSIONHELPER_HPP_ */
