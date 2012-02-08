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

#include <qimessaging/transport.hpp>
#include <qimessaging/broker.hpp>
#include <qimessaging/perf/dataperftimer.hpp>

static int gLoopCount = 1000;
static const int gThreadCount = 1;

int main_client()
{
  qi::perf::DataPerfTimer dp;
  qi::Broker nc;

  nc.connect("127.0.0.1:9559");
  nc.waitForConnected();
  nc.setName("remote");
  nc.setDestination("serviceTest");

  for (int i = 5; i < 6; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);
    dp.start(gLoopCount, numBytes);
    qi::Message request;
    qi::Message reply;
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
      request.setSource("remote");
      request.setDestination("serviceTest");
      request.setPath("reply");
      request.setData(requeststr);
      nc.tc->send(request);
      nc.tc->waitForId(request.id());
      nc.tc->read(request.id(), &reply);
      if (request.id() != reply.id() || reply.id() <= 0) {
        std::cout << "error id" << std::endl;
      }

      if (request.data().size() != reply.data().size() || reply.data().size() != numBytes) {
        std::cout << "error sz" << std::endl;
      }
      if (reply.data()[2] != c)
      {
        std::cout << "error content" << std::endl;
      }

    }
    dp.stop();
    dp.print();
  }
  return 0;
}


void start_client(int count)
{
    boost::thread thd[100];

    assert(count < 100);

    for (int i = 0; i < count; ++i)
    {
      std::cout << "starting thread: " << i << std::endl;
      sleep(1);
      thd[i] = boost::thread(boost::bind(&main_client));
    }

    for (int i = 0; i < count; ++i)
      thd[i].join();

    std::cout << "end client" << std::endl;
}



class ServiceTestPerf : public qi::TransportServerDelegate
{
public:
  ServiceTestPerf()
  {
    nthd = new qi::NetworkThread();

    ts = new qi::TransportServer();
    ts->setDelegate(this);
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
    ts->send(retval);
  }

private:
  qi::NetworkThread   *nthd;
  qi::TransportServer *ts;
};

#include <qi/os.hpp>

int main_server()
{
  ServiceTestPerf stp;
  stp.start("127.0.0.1:9559");

  while(1) {
    qi::os::sleep(1);
  }

  return 0;
}


int main(int argc, char **argv)
{
  if (argc > 1 && !strcmp(argv[1], "--client"))
  {
    start_client(1);
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server();
  }
  else
  {
    //start the server
    boost::thread threadServer(boost::bind(&main_server));
    sleep(1);
    start_client(gThreadCount);
  }
  return 0;
}
