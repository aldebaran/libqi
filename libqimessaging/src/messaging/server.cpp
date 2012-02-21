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
#include "src/transport/url.hpp"
#include "src/transport/network_thread.hpp"

namespace qi {

  class ServerPrivate : public TransportServerInterface, public TransportSocketInterface {
  public:

    virtual void newConnection() {
      TransportSocket *socket = _ts.nextPendingConnection();
      if (!socket)
        return;
      socket->setDelegate(this);
    };

    virtual void onReadyRead(TransportSocket *client, const qi::Message &msg) {
      qi::Object *obj;

      obj = _services[msg.service()];
      qi::DataStream ds(msg.data());
      qi::DataStream rs;

      obj->metaCall(msg.function(), "", ds, rs);

      qi::Message retval;
      retval.setType(qi::Message::Reply);
      retval.setId(msg.id());
      retval.setService(msg.service());
      retval.setData(rs.str());
      client->send(retval);
    };

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
    std::string                        _url;
    qi::Session                       *_session;
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

  void Server::listen(qi::Session *session, const std::string &url) {
    _p->_url = url;
    _p->_session = session;

    qi::Url urlo(_p->_url);

    _p->_ts.setDelegate(_p);
    _p->_ts.start(urlo.host(), urlo.port(), _p->_session->_nthd->getEventBase());
  }


  void Server::registerService(const std::string &name, qi::Object *obj) {
    _p->_services[name] = obj;

    qi::Message msg;
    msg.setType(qi::Message::Call);
    msg.setService("qi.master");
    msg.setFunction("registerService");

    qi::DataStream d;
    d << name;
    d << _p->_url;
    msg.setData(d.str());

    _p->_session->tc->send(msg);
    _p->_session->tc->waitForId(msg.id());
    qi::Message ans;
    _p->_session->tc->read(msg.id(), &ans);
  };

  void Server::stop() {
  }
}
