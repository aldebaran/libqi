/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/service_info.hpp>
#include "src/remoteobject_p.hpp"
#include "src/network_thread.hpp"
#include "src/session_p.hpp"

namespace qi {

  SessionInterface::~SessionInterface() {
  }

  SessionPrivate::SessionPrivate(qi::Session *session)
    : _serviceSocket(new qi::TransportSocket()),
      _networkThread(new qi::NetworkThread()),
      _self(session),
      _callbacks(0)
  {
    _serviceSocket->setCallbacks(this);
  }

  SessionPrivate::~SessionPrivate() {
    _networkThread->stop();
    delete _networkThread;
    delete _serviceSocket;
  }

  void SessionPrivate::onSocketConnected(TransportSocket *client) {
    if (_callbacks)
      _callbacks->onSessionConnected(_self);
  }

  void SessionPrivate::onSocketConnectionError(TransportSocket *client) {
    if (_callbacks)
      _callbacks->onSessionConnectionError(_self);
  }

  void SessionPrivate::onSocketDisconnected(TransportSocket *client) {
    if (_callbacks)
      _callbacks->onSessionDisconnected(_self);
  }


  bool SessionPrivate::connect(const qi::Url &serviceDirectoryURL)
  {
    return _serviceSocket->connect(_self, serviceDirectoryURL);
  }

  std::vector<ServiceInfo> SessionPrivate::services()
  {
    std::vector<ServiceInfo> result;

    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);

    _serviceSocket->send(msg);

    if (!_serviceSocket->waitForId(msg.id()))
      return result;
    qi::Message ans;
    _serviceSocket->read(msg.id(), &ans);

    qi::DataStream d(ans.buffer());
    d >> result;

    return result;
  }

  qi::TransportSocket* SessionPrivate::serviceSocket(const std::string &name,
                                                     unsigned int      *idx,
                                                     qi::Url::Protocol  type)
  {
    qi::Message    msg;
    qi::Buffer     buf;
    qi::DataStream dr(buf);
    msg.setBuffer(buf);
    dr << name;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
    _serviceSocket->send(msg);

    if (!_serviceSocket->waitForId(msg.id()))
      return 0;
    qi::Message ans;
    _serviceSocket->read(msg.id(), &ans);

    qi::ServiceInfo si;
    qi::DataStream d(ans.buffer());
    d >> si;
    *idx = si.serviceId();

    for (std::vector<std::string>::const_iterator it = si.endpoints().begin();
         it != si.endpoints().end();
         ++it)
    {
      qi::Url url(*it);

      if (url.host().compare("0.0.0.0") == 0)
        qiLogWarning("qimessaging.sessionprivate.servicesocket")
          << "Service directory return non-valid address for "
          << name << " : " << url.host() << std::endl;

      if (type == qi::Url::Protocol_Any ||
          type == url.protocol())
      {
        qi::TransportSocket *ts = NULL;
        ts = new qi::TransportSocket();
        ts->connect(_self, url);
        if (ts->waitForConnected(3))
          return ts;
        else
        {
          qiLogVerbose("qimessaging.sessionprivate.servicesocket")
          << "Fail to connect to " << url.host() << ":" << url.port() << std::endl;
          delete ts;
        }
      }
    }

    return 0;
  }


  qi::Object* SessionPrivate::service(const std::string &service,
                                      qi::Url::Protocol  type)
  {
    qi::Object          *obj;
    unsigned int serviceId = 0;
    qi::TransportSocket *ts = serviceSocket(service, &serviceId, type);

    if (ts == 0)
    {
      qiLogError("qi::Session") << "service not found: " << service;
      return 0;
    }

    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(serviceId);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::Function_MetaObject);

    ts->send(msg);
    if (ts->waitForId(msg.id()) == false)
      return (NULL);

    qi::Message ret;
    if (ts->read(msg.id(), &ret) == false)
      return (NULL);

    qi::MetaObject *mo = new qi::MetaObject;

    qi::DataStream ds(ret.buffer());

    ds >> *mo;

    qi::RemoteObject *robj = new qi::RemoteObject(ts, serviceId, mo);
    if (robj == NULL)
    {
      qiLogWarning("qimessaging.SessionPrivate.Service") << "No object related to service" << std::endl;
      return (0);
    }
    obj = robj;
    return obj;
  }


  // ###### Session

  Session::Session()
    : _p(new SessionPrivate(this))
  {
  }

  Session::~Session()
  {
    delete _p;
  }

  bool Session::connect(const qi::Url &serviceDirectoryURL)
  {
    return _p->connect(serviceDirectoryURL);
  }

  bool Session::disconnect()
  {
    _p->_serviceSocket->disconnect();
    return true;
  }

  bool Session::waitForConnected(int msecs)
  {
    return _p->_serviceSocket->waitForConnected(msecs);
  }

  bool Session::waitForDisconnected(int msecs)
  {
    return _p->_serviceSocket->waitForDisconnected(msecs);
  }

  qi::Future< std::vector<ServiceInfo> > Session::services()
  {
    qi::Promise< std::vector<ServiceInfo> > promise;
    promise.setValue(_p->services());
    return promise.future();
  }

  qi::Future< qi::TransportSocket* > Session::serviceSocket(const std::string &name,
                                              unsigned int      *idx,
                                              qi::Url::Protocol  type)
  {
    qi::Promise< qi::TransportSocket* > promise;
    promise.setValue(_p->serviceSocket(name, idx, type));
    return promise.future();
  }


  qi::Future< qi::Object* > Session::service(const std::string &service,
                                             qi::Url::Protocol  type)
  {
    qi::Promise< qi::Object * > promise;
    promise.setValue(_p->service(service, type));
    return promise.future();
  }

  bool Session::join()
  {
    _p->_networkThread->join();
    return true;
  }

  void Session::setCallbacks(SessionInterface *delegate) {
    _p->_callbacks = delegate;
  }

}
