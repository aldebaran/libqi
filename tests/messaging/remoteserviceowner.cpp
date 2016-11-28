#include <qi/applicationsession.hpp>
#include <qi/anymodule.hpp>
#include <qi/log.hpp>

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
  qi::ApplicationSession app(argc, argv);
  qi::log::addFilter("qi*", qi::LogLevel_Debug);

  qiLogInfo() << "Attempting connection to " << app.url().str();
  app.startSession();
  auto client = app.session();
  assert(client->isConnected());

  auto service = boost::make_shared<PingPongService>();
  qiLogInfo() << "Created PingPongService";
  client->registerService("PingPongService", service);
  qiLogInfo() << "Registered PingPongService";
  app.run();

  return 0;
}
