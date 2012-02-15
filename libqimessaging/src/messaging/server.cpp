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

    virtual void onReadyRead(TransportSocket *client, const qi::Message &msg) {
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
    qi::DataStream d;
    qi::EndpointInfo endpoint;
    size_t begin = 0;
    size_t end = 0;
    end = url.find(":");
    endpoint.type = url.substr(begin, end);
    begin = end + 3;
    end = url.find(":", begin);
    endpoint.ip = url.substr(begin, end - begin);
    begin = end + 1;
    std::stringstream ss(url.substr(begin));
    ss >> endpoint.port;
    d << endpoint;

    _p->_ts.setDelegate(_p);
    _p->_ts.start(endpoint.ip, endpoint.port, session->_nthd->getEventBase());

    qi::Message msg;
    msg.setId(0);
    msg.setSource(session->name());
    msg.setPath("registerEndpoint");
    msg.setData(d.str());

    session->tc->send(msg);
    session->tc->waitForId(msg.id());
    qi::Message ans;
    session->tc->read(msg.id(), &ans);
  };

  void Server::stop() {
    /*
    qi::DataStream d;
    qi::EndpointInfo endpoint;
    size_t begin = 0;
    size_t end = 0;
    end = e.find(":");
    endpoint.type = e.substr(begin, end);
    begin = end + 3;
    end = e.find(":", begin);
    endpoint.ip = e.substr(begin, end - begin);
    begin = end + 1;
    std::stringstream ss(e.substr(begin));
    ss >> endpoint.port;
    d << endpoint;

    qi::Message msg;
    msg.setId(uniqueRequestId++);
    msg.setSource(_name);
    msg.setPath("unregisterEndpoint");
    msg.setData(d.str());

    tc->send(msg);
    tc->waitForId(msg.id());
    qi::Message ans;
    tc->read(msg.id(), &ans);
    */
  }

  void Server::advertiseService(const std::string &name, qi::Object *obj) {
    _p->_services[name] = obj;
  };

}
