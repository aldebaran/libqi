#include <iomanip>

#include <boost/regex.hpp>

#include "sessionhelper.hpp"

SessionHelper::SessionHelper(const std::string &address)
{
  _session.connect(address);
}

SessionHelper::~SessionHelper()
{
  _session.close();
}

bool SessionHelper::getServiceSync(const std::string &serviceName, ServiceHelper &out)
{
  qi::FutureSync<qi::ObjectPtr> future = _session.service(serviceName);

  if (future.hasError())
  {
    std::cout << "\terror : " << future.error() << std::endl;
    return false;
  }

  out = future.value();
  return true;
}

void SessionHelper::showServiceInfo(const qi::ServiceInfo &infos, bool verbose, bool show_hidden)
{
  std::cout << "[" << std::setw(3) << infos.serviceId() << "] ";
  std::cout << infos.name() << std::endl;

  if (!verbose)
    return;

  std::string firstEp;
  if (infos.endpoints().begin() != infos.endpoints().end())
    firstEp = infos.endpoints().begin()->str();
  std::cout << "  Info:" << std::endl
            << "    machine   " << infos.machineId() << std::endl
            << "    process   " << infos.processId() << std::endl
            << "    endpoints " << firstEp << std::endl;

  for (qi::UrlVector::const_iterator it_urls = infos.endpoints().begin(); it_urls != infos.endpoints().end(); ++it_urls) {
    if (it_urls != infos.endpoints().begin())
      std::cout << "              " << it_urls->str() << std::endl;
  }

  ServiceHelper service;
  if (!getServiceSync(infos.name(), service)) {
    std::cout << "  Can't connect to service " << infos.name();
    return;
  }
  qi::details::printMetaObject(std::cout, service.objPtr()->metaObject());
}

bool isNumber(const std::string &str)
{
  for (unsigned int i = 0; i < str.length(); ++i) {
    if (!::isdigit(str[i]))
      return false;
  }
  return true;
}

void SessionHelper::showServicesInfoPattern(const std::vector<std::string> &patternVec, bool verbose, bool showHidden)
{
  std::vector<qi::ServiceInfo> servs = _session.services();
  std::vector<std::string> matchServs;

  //nothing specified, display everything
  if (patternVec.empty()) {
    for (unsigned int i = 0; i < servs.size(); ++i)
      showServiceInfo(servs[i], verbose, showHidden);
    return;
  }

  //pattern match, and display
  for (unsigned int u = 0; u < patternVec.size(); ++u)
  {
    bool displayed = false;
    if (isNumber(patternVec[u])) {
      int sid = atoi(patternVec[u].c_str());
      for (unsigned int j = 0; j < servs.size(); ++j) {
        if (servs[j].serviceId() == sid) {
          showServiceInfo(servs[j], verbose, showHidden);
          displayed = true;
        }
      }
    } else {
      boost::basic_regex<char> reg(patternVec[u]);
      for (unsigned int i = 0; i < servs.size(); ++i) {
        if (boost::regex_match(servs[i].name(), reg)) {
          showServiceInfo(servs[i], verbose, showHidden);
          displayed = true;
        }
      }
    }
    //no service displayed...
    if (!displayed)
      std::cout << "Service not found: " << patternVec[u] << std::endl;
  }
}

qi::FutureSync<void> SessionHelper::connect(const qi::Url &serviceDirectoryURL)
{
  return _session.connect(serviceDirectoryURL);
}

qi::FutureSync<void> SessionHelper::close()
{
  return _session.close();
}

qi::FutureSync< qi::ObjectPtr > SessionHelper::service(const std::string &service, const std::string &protocol)
{
  return _session.service(service, protocol);
}

qi::FutureSync< std::vector<qi::ServiceInfo> > SessionHelper::services(qi::Session::ServiceLocality locality)
{
  return _session.services(locality);
}
