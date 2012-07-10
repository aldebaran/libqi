/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
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


  {
  }

  void SessionPrivate::onSocketReadyRead(qi::TransportSocket *client, int id)
  {
    qi::Message msg;
    client->read(id, &msg);



    // Request for register/unregister methods
    std::map<int, qi::FunctorResult>::iterator it3;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      it3 = _futureFunctor.find(id);
    }
    if (it3 != _futureFunctor.end())
    {
      serviceRegisterUnregisterEnd(id, &msg, it3->second);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor.erase(it3);
      }
      return;
    }
  }

  void SessionPrivate::serviceRegisterUnregisterEnd(int id,
                                                    qi::Message *msg,
                                                    qi::FunctorResult promise)
  {
    if (!promise.isValid())
    {
      qiLogError("qimessaging.sessionprivate") << "No promise found for req id:" << id;
      return;
    }

    switch (msg->type())
    {
      case qi::Message::Type_Reply:
        promise.setValue(msg->buffer());
        break;
      case qi::Message::Type_Event:
        promise.setValue(msg->buffer());
        break;
      default:
        qiLogError("qimessaging.sessionprivate") << "Message (#" << id << ") type not handled: "
            << msg->type();
        return;
    }
  }

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
        if (ts->waitForConnected())
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
  }

  qi::Future<unsigned int> SessionPrivate::registerService(const qi::ServiceInfo &si)
  {
    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

    qi::Buffer     buf;
    qi::DataStream d(buf);
    d << si;

    qi::Future<unsigned int> future;
    if (d.status() == qi::DataStream::Status_Ok)
    {
      msg.setBuffer(buf);

      qi::FunctorResult        ret;
      qi::makeFunctorResult<unsigned int>(&ret, &future);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor[msg.id()] = ret;
      }

      if (!_serviceSocket->send(msg))
      {
        qi::DataStream dout(msg.buffer());
        qi::ServiceInfo si;
        dout >> si;
        qiLogError("qimessaging.Session") << "Error while register service: "
                                          << si.name() << " request";
        qi::Buffer buf;
        qi::DataStream dse(buf);

        dse << "Error while register service: "
            << si.name() << " request";
        ret.setError(buf);
      }
    }
    return future;
  }

  qi::Future<void> SessionPrivate::unregisterService(unsigned int idx)
  {
    qi::Message msg;
    qi::Buffer  buf;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_UnregisterService);

    qi::DataStream d(buf);
    d << idx;

    qi::Future<void> future;
    if (d.status() == qi::DataStream::Status_Ok)
    {
      msg.setBuffer(buf);

      qi::FunctorResult ret;
      qi::makeFunctorResult<void>(&ret, &future);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureFunctor[msg.id()] = ret;
      }

      if (!_serviceSocket->send(msg))
      {
        qi::DataStream dout(msg.buffer());
        unsigned int idx;
        dout >> idx;
        qiLogError("qimessaging.Session") << "Error while unregister serviceId: "
                                          << idx << " request";

        qi::Buffer buf;
        qi::DataStream dse(buf);

        dse << "Error while unregister serviceId: "
            << idx << " request";
        ret.setError(buf);
      }
    }
    return future;
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
    return _p->_serviceSocket->connect(_p->_self, serviceDirectoryURL);
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

  qi::Url Session::url() const {
    return _p->_serviceSocket->url();
  }

}
