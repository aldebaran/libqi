/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <vector>
#include <iostream>

//#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <alcommon-ng/ippc.hpp>
#include <alcore/alptr.h>

#ifdef _WIN32
    // CK 28/7/2010 dodgy hack so it compiles
    #define sleep(x) Sleep(x)
#else
    #include <unistd.h>
#endif

using AL::ALPtr;

static const int threadCount = 10;
static const int loopCount   = 10000;

int test1(const std::string &color)
{
  printf("my string color is : %s\n", color.c_str());
  //sleep(1);
  if (color == "yellow")
    return 42;
  return 0;
}

class Module1CallBack :  public AL::Messaging::OnMessageDelegate
{
public:

  void setServer(ALPtr<AL::Messaging::Server> server)
  {
    fIppcServer = server;
  }

  // to call function on current process
  AL::ALPtr<AL::Messaging::ResultDefinition> onMessage(const AL::Messaging::CallDefinition &def)
  {
    AL::ALPtr<AL::Messaging::ResultDefinition> res(new AL::Messaging::ResultDefinition(def));

    printf("Module1 CallBack\n");
    // receive fonctionQuiPoutre
    if (def.getMethodName() == "test1")
    {
      AL::Messaging::VariablesList    params = def.getParameters();
      std::string            color = params.front().as<std::string>();
      res->value(test1(color));
    }
    return res;
  }

protected:
  ALPtr<AL::Messaging::Server> fIppcServer;
};

int test2(int cowCount)
{
  printf("fonction qui poutre %d cows\n", cowCount);
  //sleep(1);
  return 42;
}

class Module2CallBack :  public AL::Messaging::OnMessageDelegate
{
public:
  void setServer(ALPtr<AL::Messaging::Server> server)
  {
    fIppcServer = server;
  }
  // to call function on current process
  AL::ALPtr<AL::Messaging::ResultDefinition> onMessage(const AL::Messaging::CallDefinition & def)
  {
    AL::ALPtr<AL::Messaging::ResultDefinition> res(new AL::Messaging::ResultDefinition(def));

    //printf("Module2 CallBack\n");
    // receive fonctionQuiPoutre
    if (def.getMethodName() == "test2")
    {
        printf("Method: test2\n");
        AL::Messaging::VariablesList    params = def.getParameters();
        int cowCount = params.front().as<int>();
        res->value(test2(cowCount));
        //sleep(1);
    }

    if (def.getMethodName() == "echo")
    {
        printf("Method: Echo\n");
        AL::Messaging::VariablesList    params = def.getParameters();
        res->value(params.convertToALValue());
    }
    return res;
  }
protected:
  ALPtr<AL::Messaging::Server> fIppcServer;
};


static const std::string gServerAddress = "tcp://127.0.0.1:5555";
static const std::string gClientAddress = "tcp://127.0.0.1:5555";


int main_server()
{
  Module2CallBack           module2Callback;
  ALPtr<AL::Messaging::Server>       fIppcServer  = ALPtr<AL::Messaging::Server>(new AL::Messaging::Server(gServerAddress));
  fIppcServer->setOnMessageDelegate(&module2Callback);
  boost::thread             threadServer(boost::bind(&AL::Messaging::Server::run, fIppcServer.get()));
  module2Callback.setServer(fIppcServer);
  while(1)
    sleep(5);
  return 0;
}

int main_client(int clientId)
{
  std::stringstream sstream;

  AL::Messaging::Client              client(gClientAddress);
  AL::Messaging::CallDefinition      def;

  def.setMethodName("test2");
  def.setSender("toto");
  def.push(41);
  AL::ALPtr<AL::Messaging::ResultDefinition> res;

  for (int i = 0; i< loopCount; ++i)
  {
    res = client.send(def);
    printf("result is: %d\n", res->value().as<int>());
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
    boost::thread thd[threadCount];

    for (int i = 0; i < threadCount; ++i)
    {
      std::cout << "starting thread: " << i << std::endl;
      thd[i] = boost::thread(boost::bind(&main_client, i));
    }

    for (int i = 0; i < threadCount; ++i)
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
