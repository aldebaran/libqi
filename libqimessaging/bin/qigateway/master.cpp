#include "master.hpp"

#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/master.hpp>

namespace qi {
namespace gateway {

void Master::launch(const std::string& address)
{
  qi::Master master(address);
  master.run();

  qiLogInfo("qigateway") << "Master: Listening on " << address;

  if (master.isInitialized()) {
    while (1)
      qi::os::sleep(1);
  }
}

} // namespace gateway
} // namespace qi
