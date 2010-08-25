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

#include <alcommon-ng/transport/transport.hpp>
#include <alcommon-ng/transport/zeromq/zmqsimpleserver.hpp>
#include <boost/shared_ptr.hpp>

#ifdef _WIN32
    // CK 28/7/2010 dodgy hack so it compiles
    #define sleep(x) Sleep(x)
#else
    #include <unistd.h>
#endif

//using AL::ALPtr;

static const int gThreadCount = 1;
static const int gLoopCount   = 100000;

//TCP: 6.6
//IPC: 6sec => 100000  => 16000 msg/sec

//static const std::string gServerAddress = "tcp://127.0.0.1:5555";
static const std::string gServerAddress = "ipc:///tmp/test";
//static const std::string gServerAddress = "inproc://workers";
static const std::string gClientAddress = gServerAddress;

//class ResultHandler;
class TestServer : public AL::Transport::Threadable, public AL::Transport::DataHandler
{
public:
  TestServer(const std::string &address)
  {
    _server = new AL::Transport::ZMQSimpleServer(address);
    _server->setDataHandler(this);
  }

  virtual void run()
  {
    _server->run();
  }

protected:
  virtual void onData(const std::string &data, std::string &result)
  {
    //simple for test
    result = data;
  }

protected:
  std::string           _serverAddress;
  AL::Transport::Server *_server;
};


int main_server()
{
  TestServer                server(gServerAddress);
  server.run();
//  boost::thread             threadServer(boost::bind(&TestServer::run, &server));
//  while(1)
//    sleep(5);
  return 0;
}

int main_client(int clientId)
{
  (void) clientId;
  AL::Transport::Client *client = new AL::Transport::ZMQClient(gClientAddress);
  std::string            tosend = "bim";
  std::string            torecv;

  for (int i = 0; i< gLoopCount; ++i)
  {
    tosend = "bim";
    torecv = "";
    client->send(tosend, torecv);
    assert(tosend == torecv);
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
    //sleep(1);
    boost::thread             threadClient(boost::bind(&main_client, 0));
    threadClient.join();
    //sleep(1);
  }
  return 0;
}
