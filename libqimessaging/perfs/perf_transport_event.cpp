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

#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>

#include <qimessaging/session.hpp>
#include "dataperftimer.hpp"

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>

#include <qimessaging/transport_server.hpp>
#include <qimessaging/url.hpp>
#include "../src/session_p.hpp"

static int gLoopCount = 10000;
static const int gThreadCount = 1;
static bool clientDone = false;
#include <iostream>


class ServerEventPrivate : public qi::TransportServerInterface, public qi::TransportSocketInterface {
public:
  ServerEventPrivate()
  {
    _ts = NULL;
    _session = NULL;
    _dp = new qi::perf::DataPerfTimer("Transport synchronous call");
    _msgRecv = 0;
    _numBytes = 0;
  }

  virtual void onTransportServerNewConnection(qi::TransportServer* server, qi::TransportSocket *socket, void *data)
  {
    if (!socket)
      return;
    socket->addCallbacks(this);
  }

  virtual void onSocketReadyRead(qi::TransportSocket *client, int id, void *data)
  {
    qi::Message msg;
    client->read(id, &msg);
    _msgRecv++;
    size_t s = msg.buffer().size();
    if (s != _numBytes)
    {
      _dp->stop(1);
      if (_msgRecv != gLoopCount)
        std::cout << "Drop " << gLoopCount - _msgRecv << " messages!" << std::endl;

      _numBytes = s;
       _msgRecv = 0;
      _dp->start(gLoopCount, _numBytes);
    }

  }

  virtual void onSocketWriteDone(qi::TransportSocket *QI_UNUSED(client), void *data)
  {
  }

  virtual void onSocketConnected(qi::TransportSocket *QI_UNUSED(client), void *data)
  {
  }

  virtual void onSocketDisconnected(qi::TransportSocket *QI_UNUSED(client), void *data)
  {
  }

public:
  std::map<unsigned int, qi::Object*> _services;
  qi::TransportServer                *_ts;
  std::vector<std::string>            _endpoints;
  qi::Session                        *_session;

  qi::perf::DataPerfTimer            *_dp;
  int                                 _msgRecv;
  unsigned int                        _numBytes;
};

class ServerEvent
{
public:
  ServerEvent()
    : _p(new ServerEventPrivate())
  {
  }

  ~ServerEvent()
  {
    delete _p;
  }

  void listen(qi::Session *session, const std::vector<std::string> &endpoints)
  {
    _p->_endpoints = endpoints;
    _p->_session = session;

    qi::Url urlo(_p->_endpoints[0]);
    _p->_ts = new qi::TransportServer(urlo);

    _p->_ts->addCallbacks(_p);
    _p->_ts->listen();
  }


  unsigned int registerService(const std::string &name, qi::Object *obj)
  {
    qi::Message     msg;
    qi::ServiceInfo si;
    msg.setType(qi::Message::Type_Event);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

    qi::Buffer b;
    qi::ODataStream d(b);
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId("TODO");
    si.setEndpoints(_p->_endpoints);
    d << si;
    msg.setBuffer(b);
    _p->_session->_p->_serviceSocket.send(msg);
    _p->_session->_p->_serviceSocket.waitForId(msg.id());
    qi::Message ans;
    _p->_session->_p->_serviceSocket.read(msg.id(), &ans);
    qi::IDataStream dout(ans.buffer());
    unsigned int idx = 0;
    dout >> idx;
    _p->_services[idx] = obj;
    return idx;
  }


  void stop() {
  }

private:
  ServerEventPrivate *_p;
};


int main_client(std::string QI_UNUSED(str))
{
  qi::Session  session;
  session.connect("tcp://127.0.0.1:5555");
  session.waitForConnected();

  qi::Object *sock = session.service("serviceTest");

  for (int i = 0; i < 12; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);

    for (int j = 0; j < gLoopCount; ++j)
    {
      sock->emitEvent("New event");
    }
  }
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
  clientDone = true;
}


#include <qi/os.hpp>


int main_gateway()
{
  qi::Gateway       gate;

  gate.attachToServiceDirectory("tcp://127.0.0.1:5555");
  gate.listen("tcp://127.0.0.1:12345");
  std::cout << "ready." << std::endl;
  while (true)
    qi::os::sleep(60);

  return 0;
}

std::string reply(const std::string &msg)
{
  return msg;
}

int main_server()
{
  qi::ServiceDirectory sd;
  sd.listen("tcp://127.0.0.1:5555");
  std::cout << "Service Directory ready." << std::endl;

  qi::Session session;
  qi::Object  obj;
  ServerEvent srv;
  obj.advertiseMethod("reply", &reply);

  session.connect("tcp://127.0.0.1:5555");
  session.waitForConnected();

  std::vector<std::string> endpoints;
  endpoints.push_back("tcp://127.0.0.1:9559");
  srv.listen(&session, endpoints);
  srv.registerService("serviceTest", &obj);
  std::cout << "serviceTest ready." << std::endl;

  while (!clientDone)
    qi::os::sleep(60);

  srv.stop();
  session.close();
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
  else
  {
    //start the server
    boost::thread threadServer1(boost::bind(&main_server));
    qi::os::sleep(1);
    start_client(gThreadCount);
  }
  return 0;
}
