/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport/transport_server.hpp>
#include <qimessaging/transport/network_thread.hpp>
#include <boost/bind.hpp>

static int reqid = 500;

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


    void run(TransportSocket *client, const qi::Message &msg)
    {
      qi::Message retval;
      if (msg.path() == "services")
        services(msg, retval);
      else if (msg.path() == "service")
        service(msg, retval);
      else
      {
        qi::Object* obj;
        std::map<std::string, qi::Object*>::iterator servicesIt;
        servicesIt = _services.find(msg.destination());
        if (servicesIt == _services.end())
        {
          obj = _session->service(msg.destination());
          _services[msg.destination()] = obj;
        }
        else
        {
          obj = servicesIt->second;
        }

        qi::DataStream ds(msg.data());
        qi::DataStream rs;

        obj->metaCall(msg.path(), "", ds, rs);

        retval.setType(qi::Message::Answer);
        retval.setId(msg.id());
        retval.setSource(msg.destination());
        retval.setDestination(msg.source());
        retval.setPath(msg.path());
        retval.setData(rs.str());
      }

      client->send(retval);

    }

    virtual void onReadyRead(TransportSocket *client, const qi::Message &msg)
    {
      boost::thread thd = boost::thread(boost::bind(&GatewayPrivate::run, this, client, msg));
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


    void services(const qi::Message &msg, qi::Message &retval)
    {
      std::vector<std::string> result = _session->services();

      qi::DataStream d;
      d << result;

      retval.setType(qi::Message::Answer);
      retval.setId(msg.id());
      retval.setSource(msg.destination());
      retval.setDestination(msg.source());
      retval.setPath(msg.path());
      retval.setData(d.str());
    }


    void service(const qi::Message &msg, qi::Message &retval)
    {
      qi::DataStream d;
      d << _endpoints;

      retval.setType(qi::Message::Answer);
      retval.setId(msg.id());
      retval.setSource(msg.destination());
      retval.setDestination(msg.source());
      retval.setPath(msg.path());
      retval.setData(d.str());
    }

  public:
    std::map<std::string, qi::Object*> _services;
    std::vector<qi::EndpointInfo>      _endpoints;
    TransportServer                    _ts;
    qi::Session                       *_session;
  }; // !GatewayPrivate

  Gateway::Gateway()
    : _p(new GatewayPrivate())
  {
  }

  Gateway::~Gateway()
  {
    delete _p;
  }

  void Gateway::start(const std::string &addr, unsigned short port, qi::Session *session)
  {
    qi::EndpointInfo e;
    e.type = "tcp";
    e.ip = addr;
    e.port = port;
    _p->_endpoints.push_back(e);

    std::vector<std::string> result = session->services();

    _p->_session = session;
    _p->_ts.setDelegate(_p);
    _p->_ts.start(addr, port, _p->_session->_nthd->getEventBase());
  }

  void Gateway::advertiseService(const std::string &name, qi::Object *obj)
  {
    _p->_services[name] = obj;
  }
} // !qi
