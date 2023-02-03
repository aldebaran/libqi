#include <qi/applicationsession.hpp>
#include <qi/anymodule.hpp>
#include <qi/log.hpp>
#include <testssl/testssl.hpp>
#include "remoteperformanceservice.hpp"

qiLogCategory("RemoteServiceOwner");

struct PingPongService
{
  qi::AnyObject take()
  {
    return object;
  }

  void give(qi::AnyObject newObject)
  {
    object = newObject;
  }

private:
  qi::AnyObject object;
};

QI_REGISTER_OBJECT(PingPongService, take, give)



int main(int argc, char **argv) {
  qi::ApplicationSession::Config cfg;
  qi::Session::Config sessionCfg;
  sessionCfg.serverSslConfig = test::ssl::serverConfig(test::ssl::server(),
                                                       test::ssl::rootCA());
  cfg.setSessionConfig(sessionCfg);
  qi::ApplicationSession app(argc, argv, cfg);
  qi::log::addFilter("qi*", qi::LogLevel_Debug);

  qiLogInfo() << "Attempting connection to " << app.url().str();
  app.startSession();
  auto client = app.session();
  assert(client->isConnected());

  auto service = boost::make_shared<PingPongService>();
  auto perfService = boost::make_shared<RemotePerformanceService>();
  qiLogInfo() << "Created PingPongService & RemotePerformanceService";
  client->registerService("PingPongService", service);
  client->registerService("RemotePerformanceService", perfService);
  qiLogInfo() << "Registered PingPongService & RemotePerformanceService";
  app.run();

  return 0;
}
