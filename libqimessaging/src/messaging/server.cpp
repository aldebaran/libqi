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

  class ServerPrivate {
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

    _p->_ts.start(addr, port, base->getEventBase());
  };

  void Server::advertiseService(const std::string &name, qi::Object *obj) {
    _p->_services[name] = obj;
  };

  void Server::onConnected(const qi::Message &msg) {
  };

  void Server::onWrite(const qi::Message &msg) {

  };

  void Server::onRead(const qi::Message &msg) {
    qi::Object *obj;

    obj = _p->_services[msg.destination()];
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
    //_p->_ts.send(retval);
  };

}
