/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/server.hpp>
#include <qimessaging/transport_server.hpp>
#include <qimessaging/service_info.hpp>
#include "src/network_thread.hpp"
#include "src/session_p.hpp"
#include <qi/os.hpp>

namespace qi {

  class ServerPrivate : public TransportServerInterface, public TransportSocketInterface
  {
  public:

    virtual void newConnection();
    virtual void onSocketReadyRead(TransportSocket *client, int id);

  public:
    std::map<unsigned int, qi::Object*> _services;
    TransportServer                     _ts;
    std::vector<std::string>            _endpoints;
    qi::Session                        *_session;
  };

  void ServerPrivate::newConnection()
  {
    TransportSocket *socket = _ts.nextPendingConnection();
    if (!socket)
      return;
    socket->setDelegate(this);
  }

  void ServerPrivate::onSocketReadyRead(TransportSocket *client, int id) {
    qi::Message msg;
    client->read(id, &msg);
    qi::Object *obj;

    obj = _services[msg.service()];
    qi::FunctorParameters ds(msg.buffer());

    qi::Message retval;
    retval.buildReplyFrom(msg);
    qi::FunctorResult rs(retval.buffer());

    obj->metaCall(msg.function(), "", ds, rs);

    client->send(retval);
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

  bool Server::listen(qi::Session *session, const std::vector<std::string> &endpoints) {
    _p->_endpoints = endpoints;
    _p->_session = session;

    qi::Url urlo(_p->_endpoints[0]);

    _p->_ts.setDelegate(_p);
    return _p->_ts.start(urlo, _p->_session->_p->_networkThread->getEventBase());
  }


  unsigned int Server::registerService(const std::string& name, qi::Object *obj)
  {
    qi::Message msg;
    qi::ServiceInfo si;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

    qi::DataStream d(msg.buffer());
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId("TODO");
    si.setEndpoints(_p->_endpoints);
    d << si;

    _p->_session->_p->_serviceSocket->send(msg);
    _p->_session->_p->_serviceSocket->waitForId(msg.id());
    qi::Message ans;
    _p->_session->_p->_serviceSocket->read(msg.id(), &ans);
    qi::DataStream dout(ans.buffer());
    unsigned int idx = 0;
    dout >> idx;
    _p->_services[idx] = obj;
    return idx;
  };

  void Server::stop() {
  }
}
