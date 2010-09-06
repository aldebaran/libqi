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

#include <alcommon-ng/tools/sleep.hpp>
#include <alcommon-ng/transport/transport.hpp>
#include <alcommon-ng/transport/zeromq/zmqsimpleserver.hpp>
#include <alcommon-ng/transport/zeromq/zmqserver.hpp>
#include <alcommon-ng/transport/zeromq/zmqserverqueue.hpp>

#include <boost/shared_ptr.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>

//#define TEST_TRANSPORT_SHM
//#define TEST_TRANSPORT_ZMQ_SINGLE
//#define TEST_TRANSPORT_ZMQ_QUEUE
//#define TEST_TRANSPORT_ZMQ_POLL




#if TEST_TRANSPORT_ZMQ_SINGLE || TEST_TRANSPORT_ZMQ_QUEUE || TEST_TRANSPORT_ZMQ_POLL
# define TEST_TRANSPORT_ZMQ
# define TEST_TRANSPORT_TCP
//#define TEST_TRANSPORT_IPC
//#define TEST_TRANSPORT_INPROC
#endif

#if not (defined(TEST_TRANSPORT_ZMQ_SINGLE) || defined(TEST_TRANSPORT_ZMQ_QUEUE) || defined(TEST_TRANSPORT_ZMQ_POLL) || defined(TEST_TRANSPORT_SHM))
# error "please define a transport"
#endif

using AL::Test::DataPerfTimer;
//using AL::ALPtr;

static const int gThreadCount = 1;
static const int gLoopCount   = 100000;


class DataPerf
{
public:
  DataPerf()
  {}
  int begin() { return 0; }
  int end() { return _count; }

  std::string &buffer(unsigned int iter, std::string &data) {
    _bytes = (unsigned long)pow(2.0f,(int)iter);
    data = std::string(_bytes, 'B');
    return data;
  }

  std::string buffer(int iter) {
    std::string data;
    return buffer(iter, data);
  }

  void start(unsigned long count) {
    _dt.start(count, _bytes);
  }
  void stop() {
    _dt.stop();
  }
  void save(const char *filename) {
    ;
  }

protected:
  unsigned long _count;
  DataPerfTimer _dt;
  unsigned int  _bytes;
};


//static const std::string gServerAddress = "tagada";
#ifdef TEST_TRANSPORT_SHM
static const std::string gServerAddress = "shmplace";
#endif
#ifdef TEST_TRANSPORT_TCP
static const std::string gServerAddress = "tcp://127.0.0.1:5555";
#endif
#ifdef TEST_TRANSPORT_IPC
static const std::string gServerAddress = "ipc:///tmp/test";
#endif
#ifdef TEST_TRANSPORT_INPROC
static const std::string gServerAddress = "inproc://workers";
#endif

static const std::string gClientAddress = gServerAddress;


//class ResultHandler;
class TestServer : public AL::Transport::IThreadable, public AL::Transport::IDataHandler
{
public:
  TestServer(AL::Transport::Server *server)
    : _server(server)
  {
    _server->setDataHandler(this);
  }

  virtual void run()
  {
    _server->run();
  }

protected:
  virtual void dataHandler(const std::string &data, std::string &result)
  {
    //simple for test
    result = data;
  }

protected:
  AL::Transport::Server *_server;
};


int main_client(AL::Transport::Client *client)
{
  DataPerf               dp;
  std::string            tosend;
  std::string            torecv;

  for (int i = 0; i < 12; ++i)
  {
    tosend = dp.buffer(i);
    dp.start(gLoopCount);
    for (int j = 0; j< gLoopCount; ++j)
    {
      torecv = "";
      client->send(tosend, torecv);
    }
    dp.stop();
    dp.save("test_transport_zmq_sinple.test");
  }
  return 0;
}

#ifdef TEST_TRANSPORT_SHM
//global result server: used by all client to receive result
//WARNING name must be unique to each process (generate a uuid?)
AL::Transport::ShmServer *gResultServer = new AL::Transport::ShmServer("clientserv");
#endif

AL::Transport::Client *makeClient(int i = 0)
{
  AL::Transport::Client *client = 0;
//TODO: generate client address

#ifdef TEST_TRANSPORT_SHM
//  std::stringstream ss;

//  ss << gClientAddress << i;
//  std::string clientAddress = ss.str();
//  return new AL::Transport::ShmClient(clientAddress, gResultServer->getResultHandler());
  return new AL::Transport::ShmClient(gClientAddress, gResultServer->getResultHandler());
#endif
#ifdef TEST_TRANSPORT_ZMQ
  return new AL::Transport::ZMQClient(gClientAddress);
#endif
  return client;
}

AL::Transport::Server *makeServer(int i = 0)
{
#ifdef TEST_TRANSPORT_ZMQ_SINGLE
  return new AL::Transport::ZMQSimpleServer(gServerAddress);
#endif
#ifdef TEST_TRANSPORT_ZMQ_QUEUE
  return new AL::Transport::ZMQServerQueue(gServerAddress);
#endif
#ifdef TEST_TRANSPORT_ZMQ_POLL
  return new AL::Transport::ZMQServer(gServerAddress);
#endif
#ifdef TEST_TRANSPORT_SHM
  return new AL::Transport::ShmServer(gClientAddress);
#endif
  return 0;

}

void start_client(int count)
{
    boost::thread thd[100];

    assert(count < 100);

    #ifdef TEST_TRANSPORT_SHM
    //start a result server
    boost::thread threadClientServer(boost::bind(&AL::Transport::ShmServer::run, gResultServer));
    sleep(1);
    #endif

    for (int i = 0; i < count; ++i)
    {
      AL::Transport::Client *client = 0;
      std::cout << "starting thread: " << i << std::endl;
      sleep(1);
      client = makeClient(i);
      thd[i] = boost::thread(boost::bind(&main_client, client));
    }

    for (int i = 0; i < count; ++i)
      thd[i].join();
    std::cout << "end client" << std::endl;
}

int main_server()
{
  AL::Transport::Server    *st;
  st = makeServer();
  TestServer                testserver(st);
  testserver.run();
  return 0;
}

//class that handle :
// - client and server args
// - loop count
// - perf saving
//
// - add multithreading support to DataPerf

int main(int argc, char **argv)
{

  if (argc > 1 && !strcmp(argv[1], "--client")) {
    start_client(1);
  } else if (argc > 1 && !strcmp(argv[1], "--server")) {
    return main_server();
  } else {
    //start the server
    boost::thread threadServer(&main_server);
    sleep(1);
    start_client(gThreadCount);
  }
  return 0;
}
