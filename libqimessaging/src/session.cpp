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

  void SessionPrivate::onSocketConnected(TransportSocket *client)
  {
    // Come from connection between transport socket and service
    std::map<void *, ServiceRequest>::iterator it = _futureConnect.find(client);
    if (it != _futureConnect.end())
    {
      ServiceRequest &sr = it->second;
      qi::Message msg;
      msg.setType(qi::Message::Type_Call);
      msg.setService(sr.serviceId);
      msg.setPath(qi::Message::Path_Main);
      msg.setFunction(qi::Message::Function_MetaObject);
      _futureService[msg.id()] = sr;
      _futureConnect.erase(client);
      client->send(msg);
      return;
    }

    {
      // Comes from session connected to ServiceDirectory
      if (_callbacks)
        _callbacks->onSessionConnected(_self);
      return;
    }
  }

  void SessionPrivate::onSocketConnectionError(TransportSocket *client)
  {
    std::map<void *, ServiceRequest>::iterator it = _futureConnect.find(client);
    if (it != _futureConnect.end())
    {
      _futureConnect.erase(client);
      return;
    }

    {
      if (_callbacks)
        _callbacks->onSessionConnectionError(_self);
      return;
    }
  }

  void SessionPrivate::onSocketDisconnected(TransportSocket *client)
  {
    if (_callbacks)
      _callbacks->onSessionDisconnected(_self);
  }

  void SessionPrivate::onSocketReadyRead(qi::TransportSocket *client, int id)
  {
    qi::Message msg;
    client->read(id, &msg);

    // request for service method
    std::map<int, ServiceRequest>::iterator it;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      it = _futureService.find(id);
    }
    if (it != _futureService.end())
    {
      if (client == _serviceSocket)
        serviceEndpointEnd(id, client, &msg, it->second); // Come from ServiceDirectory with endpoints to connect
      else
        serviceMetaobjectEnd(id, client, &msg, it->second); // Come from Service to get the MetaObject
      return;
    }

    // request from services method
    std::map<int, qi::Promise<std::vector<qi::ServiceInfo> > >::iterator it2;
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      it2 = _futureServices.find(id);
    }
    if (it2 != _futureServices.end())
    {
      servicesEnd(client, &msg, it2->second);
      {
        boost::mutex::scoped_lock l(_mutexFuture);
        _futureServices.erase(it2);
      }
      return;
    }

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
    qiLogError("qimessaging") << "Session::Private: onSocketReadyRead: unknow message id";
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
      {
        qi::Buffer b;
        qi::DataStream ds(b);
        ds << "Bad message type";
        promise.setError(b);
        qiLogError("qimessaging.sessionprivate") << "Message (#" << id << ") type not handled: "
                                                 << msg->type();
        return;
      }
    }
  }

  void SessionPrivate::serviceEndpointEnd(int id,
                                          qi::TransportSocket *QI_UNUSED(client),
                                          qi::Message *msg,
                                          ServiceRequest &sr)
  {
    qi::ServiceInfo si;
    qi::DataStream  d(msg->buffer());
    d >> si;

    if (d.status() == qi::DataStream::Status_Ok)
    {
      sr.serviceId = si.serviceId();
      for (std::vector<std::string>::const_iterator it = si.endpoints().begin();
           it != si.endpoints().end();
           ++it)
      {
        qi::Url url(*it);
        if (sr.protocol == qi::Url::Protocol_Any || sr.protocol == url.protocol())
        {
          qi::TransportSocket *ts = NULL;
          ts = new qi::TransportSocket();
          ts->setCallbacks(this);
          {
            boost::mutex::scoped_lock l(_mutexFuture);
            _futureConnect[ts] = sr;
            _futureService.erase(id);
          }
          ts->connect(_self, url);
          return;
        }
      }
    }
    {
      boost::mutex::scoped_lock l(_mutexFuture);
      _futureService[id].promise.setError("No service found");
      _futureService.erase(id);
    }
    return;
  }

  void SessionPrivate::serviceMetaobjectEnd(int QI_UNUSED(id),
                                            qi::TransportSocket *client,
                                            qi::Message *msg,
                                            ServiceRequest &sr)
  {
    qi::MetaObject *mo = new qi::MetaObject();
    qi::DataStream  d(msg->buffer());
    d >> *mo;

    if (d.status() == qi::DataStream::Status_Ok)
    {
      client->setCallbacks(0);
      qi::RemoteObject *robj = new qi::RemoteObject(client, sr.serviceId, mo);
      qi::Object *obj;
      obj = robj;
      sr.promise.setValue(obj);
    } else {
      sr.promise.setError("Serialization error");
    }
  }

  void SessionPrivate::servicesEnd(qi::TransportSocket *QI_UNUSED(client),
                                   qi::Message *msg,
                                   qi::Promise<std::vector<qi::ServiceInfo> > &promise)
  {
    std::vector<qi::ServiceInfo> result;
    qi::DataStream d(msg->buffer());
    d >> result;
    if (d.status() == qi::DataStream::Status_Ok)
      promise.setValue(result);
    else
      promise.setError("Serialization error");
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
    } else {
      qi::Promise<unsigned int> prom;
      future = prom.future();
      prom.setError("serialization error");
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
    qi::Promise<std::vector<ServiceInfo> > promise;
    qi::Message msg;

    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);
    {
      boost::mutex::scoped_lock l(_p->_mutexFuture);
      _p->_futureServices[msg.id()] = promise;
    }
    _p->_serviceSocket->send(msg);
    return promise.future();
  }

  qi::Future< qi::Object * > Session::service(const std::string &service,
                                              qi::Url::Protocol  type)
  {
    ServiceRequest sr;
    qi::Message    msg;
    qi::Buffer     buf;

    msg.setBuffer(buf);
    sr.name      = service;
    sr.protocol  = type;

    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
    qi::DataStream dr(buf);
    dr << service;
    {
      boost::mutex::scoped_lock l(_p->_mutexFuture);
      _p->_futureService[msg.id()] = sr;
    }
    _p->_serviceSocket->send(msg);
    return sr.promise.future();
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
