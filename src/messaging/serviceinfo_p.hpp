#include <qi/messaging/serviceinfo.hpp>
#include <string>

namespace qi
{

class ServiceInfoPrivate
{
public:
  ServiceInfoPrivate();

  std::string name;
  unsigned int serviceId;
  std::string machineId;
  unsigned int processId;
  qi::UrlVector endpoints;
  std::string sessionId;
};

}
