/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <vector>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <qi/messaging.hpp>
#include <boost/shared_ptr.hpp>
#include <qi/tools/dataperftimer.hpp>
#include <qi/tools/sleep.hpp>

using namespace qi::messaging;
using qi::Test::DataPerfTimer;

static const int gThreadCount = 10;
static const int gLoopCount   = 10000;

class ServiceHandler :  public IMessageHandler
{
public:
  // to call function on current process
  void messageHandler(
    const qi::messaging::CallDefinition & def,
          qi::messaging::ResultDefinition& res)
  {

    if (def.methodName() == "ping") {
      // do nothing
    } else if (def.methodName() == "size") {
      res.value((int)def.args().front().as<std::string>().size());
    } else if (def.methodName() == "echo") {
      std::string result = def.args().front().as<std::string>();
      res.value(result);
    }
  }
};

static const std::string gServerAddress = "tcp://127.0.0.1:5555";
static const std::string gClientAddress = "tcp://127.0.0.1:5555";

int main_server()
{
  ServiceHandler           module2Callback;
  boost::shared_ptr<Server>       fIppcServer  = boost::shared_ptr<Server>(new Server());
  fIppcServer->serve(gServerAddress);
  fIppcServer->setMessageHandler(&module2Callback);
  fIppcServer->run();
  return 0;
}

int main_client(int clientId)
{
  std::stringstream sstream;

  qi::messaging::Client client;
  client.connect(gClientAddress);
  qi::messaging::ResultDefinition res;

  DataPerfTimer dt("Messaging void -> ping -> void");
  dt.start(gLoopCount);
  for (int j = 0; j< gLoopCount; ++j)
  {
    client.call(CallDefinition("ping"),res);
  }
  dt.stop();

  dt.printHeader("Messaging string -> size -> int");
  for (int i = 0; i < 12; ++i)
  {
    unsigned int                  numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string                   request = std::string(numBytes, 'B');

    dt.start(gLoopCount, numBytes);
    for (int j = 0; j< gLoopCount; ++j)
    {
      client.call(CallDefinition("size", request), res);
      int size = res.value().as<int>();
      //assert(tosend == torecv);
    }
    dt.stop();
  }

  dt.printHeader("Messaging string -> echo -> string");
  for (int i = 0; i < 12; ++i)
  {
    unsigned int                  numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string                   request = std::string(numBytes, 'B');
    dt.start(gLoopCount, numBytes);
    for (int j = 0; j< gLoopCount; ++j)
    {
      qi::messaging::CallDefinition def("echo", std::string(request));
      client.call(def, res);
      std::string result = res.value().as<std::string>();
    }
    dt.stop();
  }

  return 0;
}




int main(int argc, char **argv)
{

  if (argc > 1 && !strcmp(argv[1], "--client"))
  {
    boost::thread thd[gThreadCount];

    for (int i = 0; i < gThreadCount; ++i)
    {
      std::cout << "starting thread: " << i << std::endl;
      thd[i] = boost::thread(boost::bind(&main_client, i));
    }

    for (int i = 0; i < gThreadCount; ++i)
      thd[i].join();
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server();
  }
  else
  {
    boost::thread             threadServer(&main_server);
    sleep(1);
    boost::thread             threadClient(boost::bind(&main_client, 0));
    threadClient.join();
    sleep(1);
  }
  return 0;
}
