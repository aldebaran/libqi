/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/server.hpp>
#include <qimessaging/transport/transport_server.hpp>
#include <qimessaging/transport/network_thread.hpp>

namespace qi {

  class ServerPrivate : public TransportServerInterface, public TransportSocketInterface {
  public:

    virtual void newConnection() {
      TransportSocket *socket = _ts.nextPendingConnection();
      if (!socket)
        return;
      socket->setDelegate(this);
    };

    virtual void onRead(TransportSocket *client, const qi::Message &msg) {
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
    };

    virtual void onWrite(TransportSocket *client, const qi::Message &msg)
    {
    }

    virtual void onConnected(TransportSocket *client, const Message &msg)
    {
    }

    virtual void onDisconnected(TransportSocket *client, const Message &msg)
    {
    }

  public:
    std::map<std::string, qi::Object*> _services;
    TransportServer                    _ts;
  };

  Server::Server()
    : _p(new ServerPrivate())
  {
//    _p->_ts.setDelegate(this);
  }

  Server::~Server()
  {
    delete _p;
  }

  void Server::start(const std::string &addr, unsigned short port, qi::NetworkThread *base) {
    _p->_ts.setDelegate(_p);
    _p->_ts.start(addr, port, base->getEventBase());
  };

  void Server::advertiseService(const std::string &name, qi::Object *obj) {
    _p->_services[name] = obj;
  };

}
