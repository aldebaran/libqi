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

#include <alcommon-ng/ippc.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/timer.hpp>

using namespace AL::Messaging;

#ifdef _WIN32
    // CK 28/7/2010 dodgy hack so it compiles
    #define sleep(x) Sleep(x)
#else
    #include <unistd.h>
#endif

//using AL::ALPtr;

static const int gThreadCount = 10;
static const int gLoopCount   = 10000;


class ServiceHandler :  public DefaultMessageHandler
{
public:
  // to call function on current process
  boost::shared_ptr<AL::Messaging::ResultDefinition> onMessage(const AL::Messaging::CallDefinition & def)
  {
    boost::shared_ptr<AL::Messaging::ResultDefinition> res(new AL::Messaging::ResultDefinition(def));

    if (def.getMethodName() == "test2")
    {
      //printf("Method: test2\n");
      AL::Messaging::VariablesList    params = def.getParameters();
      res->value((int)params.front().as<std::string>().size());
    }

    else if (def.getMethodName() == "echo")
    {
        printf("Method: Echo\n");
        AL::Messaging::VariablesList    params = def.getParameters();
        res->value(params.convertToALValue());
    }
    return res;
  }
};


static const std::string gServerAddress = "tcp://127.0.0.1:5555";
static const std::string gClientAddress = "tcp://127.0.0.1:5555";


int main_server()
{
  ServiceHandler           module2Callback;
  boost::shared_ptr<DefaultServer >       fIppcServer  = boost::shared_ptr<DefaultServer >(new DefaultServer(gServerAddress));
  fIppcServer->setMessageHandler(&module2Callback);
  fIppcServer->run();
  //boost::thread             threadServer(boost::bind(&DefaultServer::run, fIppcServer.get()));
  //while(1)
  //  sleep(5);
  return 0;
}

int main_client(int clientId)
{
  std::stringstream sstream;

  AL::Messaging::DefaultClient       client(gClientAddress);
  boost::shared_ptr<AL::Messaging::ResultDefinition> res;

  //for (int i = 0; i< gLoopCount; ++i)
  //{
  //  res = client.send(def);
  //  printf("result is: %d\n", res->value().as<int>());
  //}


  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  for (int i = 0; i < 12; ++i)
  {
    unsigned int numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string  request = std::string(numBytes, 'B');
    boost::timer t;
    double       elapsed;
    AL::Messaging::CallDefinition      def;

    def.setMethodName("test2");
    def.setSender("toto");
    def.push(request);

    t.restart();
    for (int j = 0; j< gLoopCount; ++j)
    {
      res = client.send(def);
      //assert(tosend == torecv);
    }

    //
    elapsed = t.elapsed();
    float msgPs = 1.0f / ((float)elapsed / (1.0f * gLoopCount) );
    float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
    std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
  }




  /* Couldn't get echo to work ... VariableValue and VariableList beurk
  // test types
  // can't clear the underlying list, shame
  def = AL::Messaging::CallDefinition();
  def.setSender("test-module2");
  def.setMethodName("echo");

  // string
  def = AL::Messaging::CallDefinition();
  def.setSender("test-module2");
  def.push("Text:"); def.push("some text");
  res = client.send(def);
  std::cout << "Result is " << res->value() << std::endl;

  // int
  def = AL::Messaging::CallDefinition();
  def.setSender("test-module2");
  def.push("Int:"); def.push(1);
  res = client.send(def);
  std::cout << "Result is " << res->value() << std::endl;


  // bool
  def = AL::Messaging::CallDefinition();
  def.setSender("test-module2");
  def.push("bool:"); def.push(true);
  res = client.send(def);
  std::cout << "Result is " << res->value() << std::endl;

  // float
  def = AL::Messaging::CallDefinition();
  def.setSender("test-module2");
  def.push("float:"); def.push(0.42f);
  res = client.send(def);
  std::cout << "Result is " << res->value() << std::endl;
 */

  //sleep(1);

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
