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

static int uniqueRequestId = 0;

namespace qi {

  SessionPrivate::SessionPrivate() {
    _networkThread = new qi::NetworkThread();
    _serviceSocket = new qi::TransportSocket();
    _serviceSocket->setDelegate(this);
  }

  SessionPrivate::~SessionPrivate() {
    _serviceSocket->disconnect();
    delete _networkThread;
    delete _serviceSocket;
  }

  void SessionPrivate::onWriteDone(TransportSocket *client)
  {
  }

  void SessionPrivate::onReadyRead(TransportSocket *client, Message &msg)
  {
  }

  void SessionPrivate::onConnected(TransportSocket *client)
  {
  }

  void SessionPrivate::onDisconnected(TransportSocket *client)
  {
  }

  void SessionPrivate::connect(const std::string &url)
  {
    _serviceSocket->connect(url, _networkThread->getEventBase());
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

      _serviceSocket->waitForId(msg.id());
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
    qi::Message msg;
    qi::DataStream dr(msg.buffer());
    dr << name;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
    _serviceSocket->send(msg);

    _serviceSocket->waitForId(msg.id());
    qi::Message ans;
    _serviceSocket->read(msg.id(), &ans);

    qi::ServiceInfo si;
    qi::DataStream d(ans.buffer());
    d >> si;
    *idx = si.serviceId();

    qi::TransportSocket* ts = NULL;

    //TODO: choose a good endpoint
    qi::Url url(si.endpoints()[0]);

    ts = new qi::TransportSocket();
    ts->setDelegate(this);
    ts->connect(url, _networkThread->getEventBase());
    ts->waitForConnected();

    return ts;
  }


  qi::Object* SessionPrivate::service(const std::string &service,
                                      qi::Url::Protocol  type)
  {
    qi::Object          *obj;
    unsigned int serviceId = 0;
    qi::TransportSocket *ts = serviceSocket(service, &serviceId, type);

    if (ts == 0)
    {
      return 0;
    }

    qi::Message msg;
    msg.setType(qi::Message::Type_Call);
    msg.setService(serviceId);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::Function_MetaObject);

    ts->send(msg);
    ts->waitForId(msg.id());

    qi::Message ret;
    ts->read(msg.id(), &ret);

    qi::MetaObject *mo = new qi::MetaObject;

    qi::DataStream ds(ret.buffer());

    ds >> *mo;

    qi::RemoteObject *robj = new qi::RemoteObject(ts, serviceId, mo);
    obj = robj;
    return obj;
  }


  // ###### Session

  Session::Session()
    : _p(new SessionPrivate())
  {
  }

  Session::~Session()
  {
    delete _p;
  }


  void Session::connect(const std::string &masterAddress)
  {
    _p->connect(masterAddress);
  }

  bool Session::disconnect()
  {
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

  std::vector<ServiceInfo> Session::services()
  {
    return _p->services();
  }

  qi::TransportSocket* Session::serviceSocket(const std::string &name,
                                              unsigned int      *idx,
                                              qi::Url::Protocol  type)
  {
    return _p->serviceSocket(name, idx, type);
  }


  qi::Object* Session::service(const std::string &service,
                               qi::Url::Protocol  type)
  {
    return _p->service(service, type);
  }

  void Session::join()
  {
    _p->_networkThread->join();
  }



}
