#include <iomanip>
#include <boost/regex.hpp>
#include <qi/iocolor.hpp>
#include "sessionhelper.hpp"

SessionHelper::SessionHelper(const std::string &address)
{
  _session.connect(address);
  _session.disconnected.connect(boost::bind(&SessionHelper::onDisconnect, this, _1));
}

SessionHelper::~SessionHelper()
{
  _session.close();
}

ServiceHelper SessionHelper::getService(const std::string &serviceName)
{
  qi::FutureSync<qi::ObjectPtr> future = _session.service(serviceName);

  if (future.hasError())
    throw std::runtime_error(future.error());
  return ServiceHelper(future.value(), serviceName);
}

std::vector<ServiceHelper> SessionHelper::getServices(const std::string &pattern)
{
  std::vector<qi::ServiceInfo> servsInfos = _session.services().value();
  std::vector<ServiceHelper> services;
  services.reserve(servsInfos.size());
  boost::basic_regex<char> reg(pattern);

  for (unsigned int i = 0; i < servsInfos.size(); ++i)
  {
    if (boost::regex_match(servsInfos[i].name(), reg))
      services.push_back(getService(servsInfos[i].name()));
  }
  return services;
}

void SessionHelper::showServiceInfo(const qi::ServiceInfo &infos, bool verbose, bool showHidden, bool showDoc)
{
  std::cout << qi::StreamColor_Fuchsia
            << std::right << std::setw(3) << std::setfill('0')
            << infos.serviceId() << qi::StreamColor_Reset
            << std::left << std::setw(0) << std::setfill(' ')
            << " [" << qi::StreamColor_Red << infos.name() << qi::StreamColor_Reset << "]" << std::endl;

  if (!verbose)
    return;

  std::string firstEp;
  if (infos.endpoints().begin() != infos.endpoints().end())
    firstEp = infos.endpoints().begin()->str();
  std::cout << qi::StreamColor_Green << "  * " << qi::StreamColor_Fuchsia << "Info" << qi::StreamColor_Reset << ":"
             << std::endl;

  std::cout << "   machine   " << infos.machineId() << std::endl
            << "   process   " << infos.processId() << std::endl
            << "   endpoints " << firstEp << std::endl;

  for (qi::UrlVector::const_iterator it_urls = infos.endpoints().begin(); it_urls != infos.endpoints().end(); ++it_urls) {
    if (it_urls != infos.endpoints().begin())
      std::cout << "             " << it_urls->str() << std::endl;
  }

  try
  {
    ServiceHelper service = getService(infos.name());
    qi::details::printMetaObject(std::cout, service.objPtr()->metaObject(), true, showHidden, showDoc);
  }
  catch (...)
  {
    std::cout << "  Can't connect to service " << infos.name();
    return;
  }
}

bool isNumber(const std::string &str)
{
  for (unsigned int i = 0; i < str.length(); ++i) {
    if (!::isdigit(str[i]))
      return false;
  }
  return true;
}

void SessionHelper::showServicesInfoPattern(const std::vector<std::string> &patternVec, bool verbose, bool showHidden, bool showDoc)
{
  std::vector<qi::ServiceInfo> servs = _session.services();
  std::vector<std::string> matchServs;

  //nothing specified, display everything
  if (patternVec.empty()) {
    for (unsigned int i = 0; i < servs.size(); ++i)
      showServiceInfo(servs[i], verbose, showHidden, showDoc);
    return;
  }

  //pattern match, and display
  for (unsigned int u = 0; u < patternVec.size(); ++u)
  {
    bool displayed = false;
    if (isNumber(patternVec[u])) {
      unsigned int sid = atoi(patternVec[u].c_str());
      for (unsigned int j = 0; j < servs.size(); ++j) {
        if (servs[j].serviceId() == sid) {
          showServiceInfo(servs[j], verbose, showHidden, showDoc);
          displayed = true;
        }
      }
    } else {
      boost::basic_regex<char> reg(patternVec[u]);
      for (unsigned int i = 0; i < servs.size(); ++i) {
        if (boost::regex_match(servs[i].name(), reg)) {
          showServiceInfo(servs[i], verbose, showHidden, showDoc);
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

void SessionHelper::onDisconnect(const std::string &str)
{
  std::cout << "connection lost: " << str << std::endl;
  ::exit(1);
}
