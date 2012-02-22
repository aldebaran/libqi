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
#include <qimessaging/details/dataperftimer.hpp>

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>

#include <qimessaging/transport_server.hpp>
#include "../src/url.hpp"
#include "../src/network_thread.hpp"

static int gLoopCount = 10000;
static const int gThreadCount = 1;



class ServerEventPrivate : public qi::TransportServerInterface, public qi::TransportSocketInterface {
public:
  ServerEventPrivate()
  {
    _dp = new qi::perf::DataPerfTimer("Transport synchronous call");
    _msgRecv = 0;
    _sizeM = 0;
  }

  virtual void newConnection() {
    qi::TransportSocket *socket = _ts.nextPendingConnection();
    if (!socket)
      return;
    socket->setDelegate(this);
  }

  virtual void onReadyRead(qi::TransportSocket *client, qi::Message &msg)  {

    if (_msgRecv == 0)
    {
      unsigned int numBytes = (unsigned int)pow(2.0f, (int)_sizeM);
      _dp->start(gLoopCount, numBytes);
      _msgRecv++;
    }
    else if (_msgRecv == gLoopCount)
    {
      _dp->stop(1);
      _msgRecv = 0;
      _sizeM++;
    }
    else
    {
      _msgRecv++;
    }

//    qi::Object *obj;

//    obj = _services[msg.service()];

//    obj->metaCall(msg.function(), "", ds, rs);

//    qi::Message retval;
//    retval.buildReplyFrom(msg);
//    client->send(retval);
  };

  virtual void onWriteDone(qi::TransportSocket *client)
  {
  }

  virtual void onConnected(qi::TransportSocket *client)
  {
  }

  virtual void onDisconnected(qi::TransportSocket *client)
  {
  }

public:
  qi::perf::DataPerfTimer           *_dp;
  std::map<std::string, qi::Object*> _services;
  qi::TransportServer                _ts;
  std::string                        _url;
  qi::Session                       *_session;
  int                                _msgRecv;
  int                                _sizeM;
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

  void listen(qi::Session *session, const std::string &url) {
    _p->_url = url;
    _p->_session = session;

    qi::Url urlo(_p->_url);

    _p->_ts.setDelegate(_p);
    _p->_ts.start(urlo.host(), urlo.port(), _p->_session->_nthd->getEventBase());
  }


  void registerService(const std::string &name, qi::Object *obj) {
    _p->_services[name] = obj;

    qi::Message msg;
    msg.setType(qi::Message::Call);
    msg.setService(qi::Message::ServiceDirectory);
    msg.setPath(0);
    msg.setFunction(qi::Message::RegisterService);

    qi::DataStream d(msg.buffer());
    d << name;
    d << _p->_url;

    _p->_session->tc->send(msg);
    _p->_session->tc->waitForId(msg.id());
    qi::Message ans;
    _p->_session->tc->read(msg.id(), &ans);
  };

  void stop() {
    }

  private:
    ServerEventPrivate *_p;
};


int main_client(std::string src)
{
  unsigned int serviceId;
  qi::Session session;
  session.connect("tcp://127.0.0.1:5555");
  session.waitForConnected();

  qi::TransportSocket *sock = session.serviceSocket("serviceTest", &serviceId);

  for (int i = 0; i < 12; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);

    for (int j = 0; j < gLoopCount; ++j)
    {
      static int id = 1;
      id++;
      char c = 1;
      c++;
      if (c == 0)
        c++;
      requeststr[2] = c;

      qi::Message msg;
      qi::DataStream d(msg.buffer());
      d << requeststr;

      /* FIXME: what are we trying to do here? */
      msg.setType(qi::Message::Call);
      msg.setService(1);
      msg.setPath(0);
      msg.setFunction(1);
      sock->send(msg);
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
}


#include <qi/os.hpp>


int main_gateway()
{
  qi::Session       session;
  qi::Object        obj;
  qi::Gateway       gate;

  session.connect("tcp://127.0.0.1:5555");
  session.waitForConnected();

  gate.listen(&session, "tcp://127.0.0.1:12345");
  std::cout << "ready." << std::endl;

  while (1)
    qi::os::sleep(1);
}

#include <qimessaging/server.hpp>

std::string reply(const std::string &msg)
{
  return msg;
}

int main_server()
{
  qi::ServiceDirectory sd;
  sd.listen("tcp://127.0.0.1:5555");
  std::cout << "Service Directory ready." << std::endl;

  qi::Session       session;
  qi::Object        obj;
  ServerEvent       srv;
  obj.advertiseMethod("reply", &reply);

  session.connect("tcp://127.0.0.1:5555");
  session.waitForConnected();

  srv.listen(&session, "tcp://127.0.0.1:9559");
  srv.registerService("serviceTest", &obj);
  std::cout << "serviceTest ready." << std::endl;

  while (1)
    qi::os::sleep(1);

  srv.stop();
  session.disconnect();
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
//    boost::thread threadServer2(boost::bind(&main_gateway));
    sleep(1);
    start_client(gThreadCount);
  }
  return 0;
}
