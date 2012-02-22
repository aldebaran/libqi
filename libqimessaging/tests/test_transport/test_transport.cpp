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


#include <qi/transport.hpp>
//#include "src/zmq/zmq_server_backend.hpp"
//#include "src/zmq/zmq_server_queue_backend.hpp"
#include "src/zmq/zmq_simple_server_backend.hpp"
#include "src/zmq/zmq_client_backend.hpp"

#include <boost/shared_ptr.hpp>
#include <qimessaging/perf/dataperftimer.hpp>


//#define TEST_TRANSPORT_ZMQ_SINGLE
//#define TEST_TRANSPORT_ZMQ_QUEUE
//#define TEST_TRANSPORT_ZMQ_POLL




#if TEST_TRANSPORT_ZMQ_SINGLE || TEST_TRANSPORT_ZMQ_QUEUE || TEST_TRANSPORT_ZMQ_POLL
# define TEST_TRANSPORT_ZMQ
//# define TEST_TRANSPORT_TCP
//#define TEST_TRANSPORT_IPC
#define TEST_TRANSPORT_INPROC
#endif

#if not (defined(TEST_TRANSPORT_ZMQ_SINGLE) || defined(TEST_TRANSPORT_ZMQ_QUEUE) || defined(TEST_TRANSPORT_ZMQ_POLL))
# error "please define a transport"
#endif

using qi::perf::DataPerfTimer;
//using AL::ALPtr;

static const int gThreadCount = 1;
static const int gLoopCount   = 100000;


class DataPerf
{
public:
  DataPerf()
  {} : _count(0), _bytes(0)

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

#ifdef TEST_TRANSPORT_TCP
static const std::string gServerAddress = "tcp://127.0.0.1:5555";
#endif
#ifdef TEST_TRANSPORT_IPC
static const std::string gServerAddress = "ipc:///tmp/toto-test";
#endif
#ifdef TEST_TRANSPORT_INPROC
static const std::string gServerAddress = "inproc:///toto-workers";
#endif


static const std::string gClientAddress = gServerAddress;

class TestServer : public qi::transport::TransportMessageHandler
{
public:
  TestServer(qi::transport::detail::ServerBackend *server)
    : _server(server)
  {
    _server->setMessageHandler(this);
  }

  virtual void run()
  {
    _server->run();
  }

protected:
  void messageHandler(qi::transport::Buffer &request, qi::transport::Buffer &reply)
  {
    //simple for test
    reply = request;
  }

protected:
  qi::transport::detail::ServerBackend *_server;
};


int main_client(qi::transport::detail::ClientBackend *client)
{
  DataPerf               dp;
  std::string            tosend;
  std::string            torecv;

  for (int i = 4; i < 5; ++i)
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


qi::transport::detail::ClientBackend *makeClient(int i, zmq::context_t &ctx)
{
  qi::transport::detail::ClientBackend *client = 0;
//TODO: generate client address

#ifdef TEST_TRANSPORT_ZMQ
  return new qi::transport::detail::ZMQClientBackend(gClientAddress, ctx);
#endif
  return client;
}

qi::transport::detail::ServerBackend *makeServer(zmq::context_t &ctx)
{
  std::vector<std::string> addresses;
  addresses.push_back(gServerAddress);

#ifdef TEST_TRANSPORT_ZMQ_SINGLE
  return new qi::transport::detail::ZMQSimpleServerBackend(addresses, ctx);
#endif
#ifdef TEST_TRANSPORT_ZMQ_QUEUE
  return new qi::transport::detail::ZMQServerQueueBackend(addresses, ctx);
#endif
#ifdef TEST_TRANSPORT_ZMQ_POLL
  return new qi::transport::detail::ZMQServerBackend(addresses, ctx);
#endif
  return 0;

}

void start_client(int count, zmq::context_t *ctx)
{
    boost::thread thd[100];

    assert(count < 100);

    for (int i = 0; i < count; ++i)
    {
      qi::transport::detail::ClientBackend *client = 0;
      std::cout << "starting thread: " << i << std::endl;
      sleep(1);
      client = makeClient(i, *ctx);
      thd[i] = boost::thread(boost::bind(&main_client, client));
    }

    for (int i = 0; i < count; ++i)
      thd[i].join();
    std::cout << "end client" << std::endl;
}

int main_server(zmq::context_t *ctx)
{
  qi::transport::detail::ServerBackend    *st;
  st = makeServer(*ctx);
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
  zmq::context_t *ctx = new zmq::context_t(1);
  if (argc > 1 && !strcmp(argv[1], "--client")) {
    start_client(1, ctx);
  } else if (argc > 1 && !strcmp(argv[1], "--server")) {
    return main_server(ctx);
  } else {
    //start the server
    boost::thread threadServer(boost::bind(&main_server, ctx));
    sleep(1);
    start_client(gThreadCount, ctx);
  }
  return 0;
}
