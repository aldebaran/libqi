/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/transport/transport_server.hpp>
#include <qimessaging/transport/network_thread.hpp>

namespace qi
{

  class GatewayPrivate : public TransportServerInterface, public TransportSocketInterface
  {
  public:

    virtual void newConnection()
    {
      TransportSocket *socket = _ts.nextPendingConnection();
      if (!socket)
        return;
      socket->setDelegate(this);
    }

    virtual void onReadyRead(TransportSocket *client, const qi::Message &msg)
    {
      qi::Object *obj;

      obj = _services[msg.destination()];
      qi::DataStream ds(msg.data());
      qi::DataStream rs;

      obj->metaCall(msg.path(), "", ds, rs);

      qi::Message retval;
      retval.setType(qi::Message::Answer);
      retval.setId(msg.id());
      retval.setSource(msg.destination());
      retval.setDestination(msg.source());
      retval.setPath(msg.path());
      retval.setData(rs.str());
      client->send(retval);
    }

    virtual void onWriteDone(TransportSocket *client)
    {
    }

    virtual void onConnected(TransportSocket *client)
    {
    }

    virtual void onDisconnected(TransportSocket *client)
    {
    }


  public:
    std::map<std::string, qi::Object*> _services;
    TransportServer                    _ts;
  };

  Gateway::Gateway()
    : _p(new GatewayPrivate())
  {
  }

  Gateway::~Gateway()
  {
    delete _p;
  }

  void Gateway::start(const std::string &addr, unsigned short port, qi::NetworkThread *base)
  {
    _p->_ts.setDelegate(_p);
    _p->_ts.start(addr, port, base->getEventBase());
  }

  void Gateway::advertiseService(const std::string &name, qi::Object *obj)
  {
    _p->_services[name] = obj;
  }
}

//static int id = 300;

//namespace qi
//{
//  Gateway::Gateway()
//  {
//    _nthd = new qi::NetworkThread();
//    _ts = new qi::TransportServer();
////    _ts->setDelegate(this);
//  }

//  Gateway::~Gateway()
//  {
//    delete _ts;
//    delete _nthd;
//  }

//  void Gateway::start(const std::string &address,
//                      unsigned short port,
//                      struct event_base *base)
//  {
//    qi::EndpointInfo e;

//    e.type = "tcp";
//    e.ip = address;
//    e.port = port;


//    _endpoints.push_back(e);
//    _ts->start(e.ip, e.port, _nthd->getEventBase());
//  }

//  void Gateway::onConnected(TransportSocket *client, const qi::Message &msg)
//  {
//  }

//  void Gateway::onWrite(TransportSocket *client, const qi::Message &msg)
//  {
//  }

//  void Gateway::onRead(TransportSocket *client, const qi::Message &msg)
//  {
//    qi::Message ans;

//    if (msg.path() == "services")
//    {
//      services(msg, ans);
//    }
//    else if (msg.path() == "service")
//    {
//      service(msg, ans);
//    }
//    else
//    {
//      qi::TransportSocket* ts;
//      std::map<std::string, qi::TransportSocket*>::iterator it = _serviceConnection.find(msg.destination());
//      // no connection to service
//      if (it == _serviceConnection.end())
//      {
//        ts = _session.serviceSocket(msg.destination());
//        _serviceConnection[msg.destination()] = ts;
//      }
//      else
//      {
//        ts = it->second;
//      }

//      qi::Message fwd(msg);
//      fwd.setId(id++);
//      fwd.setSource("gateway");
//      ts->send(fwd);
//      ts->waitForId(fwd.id());
//      qi::Message ans;
//      ts->read(fwd.id(), &ans);
//      ans.setId(msg.id());
//      ans.setDestination(msg.source());

////      _ts->send(ans);
//    }
//  }

//  void Gateway::services(const qi::Message &msg, qi::Message &retval)
//  {
//    std::vector<std::string> result = _session.services();

//    qi::DataStream d;
//    d << result;

//    qi::Message retval;
//    retval.setType(qi::Message::Answer);
//    retval.setId(msg.id());
//    retval.setSource(msg.destination());
//    retval.setDestination(msg.source());
//    retval.setPath(msg.path());
//    retval.setData(d.str());

////    _ts->send(retval);
//  }


//  void Gateway::service(const qi::Message &msg, qi::Message &retval)
//  {
//    qi::DataStream d;
//    d << _endpoints;

//    qi::Message retval;
//    retval.setType(qi::Message::Answer);
//    retval.setId(msg.id());
//    retval.setSource(msg.destination());
//    retval.setDestination(msg.source());
//    retval.setPath(msg.path());
//    retval.setData(d.str());

////    _ts->send(retval);
//  }

//  void Gateway::registerGateway(const std::string &masterAddress,
//                                const std::string &gatewayAddress)
//  {
//    qi::EndpointInfo e;

//    size_t begin = 0;
//    size_t end = 0;
//    end = gatewayAddress.find(":");

//    e.ip = gatewayAddress.substr(begin, end);
//    begin = end + 1;

//    std::stringstream ss(gatewayAddress.substr(begin));
//    ss >> e.port;
//    e.type = "tcp";

//    _session.connect(masterAddress);
//    _session.waitForConnected();
//    _session.setName("gateway");
//    _session.setDestination("qi.master");
//    _session.registerEndpoint(e);
//  }

//  void Gateway::unregisterGateway(const std::string &gatewayAddress)
//  {
//    qi::EndpointInfo e;

//    size_t begin = 0;
//    size_t end = 0;
//    end = gatewayAddress.find(":");

//    e.ip = gatewayAddress.substr(begin, end);
//    begin = end + 1;

//    std::stringstream ss(gatewayAddress.substr(begin));
//    ss >> e.port;
//    e.type = "tcp";

//    _session.unregisterEndpoint(e);
//  }

//} // !qi
