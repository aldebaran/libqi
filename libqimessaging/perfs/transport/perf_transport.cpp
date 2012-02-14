/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <qimessaging/transport.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/perf/dataperftimer.hpp>

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>

static int gLoopCount = 10000;
static const int gThreadCount = 1;

int main_client(std::string src)
{
  qi::perf::DataPerfTimer dp ("Transport synchronous call");
  qi::Session session;

  static int sessionId = 0;

  std::stringstream ss;
  ss << src << sessionId++;
  session.setName(ss.str());
  session.setDestination("qi.master");
  session.connect("127.0.0.1:12345");
  session.waitForConnected();
  qi::TransportSocket* transport = session.serviceSocket("serviceTest");

  for (int i = 0; i < 12; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);
    qi::Message request;
    qi::Message reply;

    dp.start(gLoopCount, numBytes);
    for (int j = 0; j < gLoopCount; ++j)
    {
      static int id = 1;
      id++;
      char c = 1;
      c++;
      if (c == 0)
        c++;
      requeststr[2] = c;

      request.setId(id);
      request.setSource(src);
      request.setDestination("serviceTest");
      request.setPath("reply");
      request.setData(requeststr);

      transport->send(request);
      transport->waitForId(request.id(), -1);
      transport->read(request.id(), &reply);

      if (request.id() != reply.id() || reply.id() <= 0) {
        std::cout << "error id" << std::endl;
      }

      if (request.data().size() != reply.data().size() || reply.data().size() != numBytes) {
        std::cout << "error sz" << std::endl;
      }
      if (numBytes > 2 && reply.data()[2] != c)
      {
        std::cout << "error content" << std::endl;
      }
    }
    dp.stop(1);
  }
  delete transport;
  return 0;
}

void start_client(int count)
{
    boost::thread thd[100];

    assert(count < 100);

    for (int i = 0; i < count; ++i)
    {
      std::stringstream ss;
      ss << "remote" << i;
      std::cout << "starting thread: " << ss.str() << std::endl;
      thd[i] = boost::thread(boost::bind(&main_client, ss.str()));
    }

    for (int i = 0; i < count; ++i)
      thd[i].join();
}



class ServiceTestPerf : public qi::TransportServer
{
public:
  ServiceTestPerf()
  {
    nthd = new qi::NetworkThread();

    ts = new qi::TransportServer();
//    ts->setDelegate(this);
  }

  ~ServiceTestPerf()
  {
    delete ts;
  }

  void start(const std::string &address)
  {
    size_t begin = 0;
    size_t end = 0;
    end = address.find(":");

    std::string ip = address.substr(begin, end);
    begin = end + 1;

    unsigned int port;
    std::stringstream ss(address.substr(begin));
    ss >> port;

    ts->start(ip, port, nthd->getEventBase());
  }

  virtual void onConnected(const qi::Message &msg)
  {
  }

  virtual void onWrite(const qi::Message &msg)
  {
  }

  virtual void onRead(const qi::Message &msg)
  {
    if (msg.path() == "reply")
      reply(msg);
  }

  void reply(const qi::Message &msg)
  {
    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(msg.data());
//    ts->send(retval);
  }

private:
  qi::NetworkThread   *nthd;
  qi::TransportServer *ts;
};

#include <qi/os.hpp>

int main_gateway()
{
  qi::Gateway gate;
//  gate.start("127.0.0.1:12345");
  std::cout << "ready." << std::endl;

//  gate.registerGateway("127.0.0.1:5555", "127.0.0.1:12345");

  while (1)
    qi::os::sleep(1);

//  gate.unregisterGateway("127.0.0.1:12345");
}

int main_server()
{
  qi::ServiceDirectory sd;
  sd.start("127.0.0.1:5555");


  qi::Session session;
  ServiceTestPerf stp;
  stp.start("127.0.0.1:9559");

  std::string e = "tcp://127.0.0.1:9559";

  session.setName("serviceTest");
  session.setDestination("qi.master");
  session.connect("127.0.0.1:5555");
  session.waitForConnected();
  session.registerEndpoint(e);

  while(1) {
    qi::os::sleep(1);
  }

  return 0;
}


int main(int argc, char **argv)
{
  if (argc > 1 && !strcmp(argv[1], "--client"))
  {
    int threadc = 1;
    if (argc > 2)
      threadc = atoi(argv[2]);
    start_client(threadc);
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server();
  }
  else if (argc > 1 && !strcmp(argv[1], "--gateway"))
  {
    return main_gateway();
  }
  else
  {
    //start the server
    boost::thread threadServer1(boost::bind(&main_server));
    boost::thread threadServer2(boost::bind(&main_gateway));
    sleep(1);
    start_client(gThreadCount);
  }
  return 0;
}
