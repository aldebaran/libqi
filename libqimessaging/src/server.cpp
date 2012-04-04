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
#include "src/server_functor_result_future_p.hpp"
#include "src/transport_server_p.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {

  class ServerPrivate : public TransportServerInterface, public TransportSocketInterface
  {
  public:

    virtual void newConnection();
    virtual void onSocketReadyRead(TransportSocket *client, int id);

  public:
    std::map<unsigned int, qi::Object*>     _services;
    std::map<std::string, qi::Object*>      _servicesByName;
    std::map<std::string, qi::ServiceInfo>  _servicesInfo;
    std::map<unsigned int, std::string>     _servicesIndex;
    TransportServer                         _ts;
    std::vector<std::string>                _endpoints;
    qi::Session                            *_session;
    boost::mutex                            _mutexServices;
    boost::mutex                            _mutexOthers;
  };

  void ServerPrivate::newConnection()
  {
    TransportSocket *socket = _ts.nextPendingConnection();
    if (!socket)
      return;
    socket->setCallbacks(this);
  }

  void ServerPrivate::onSocketReadyRead(TransportSocket *client, int id) {
    qi::Message msg;
    client->read(id, &msg);
    qi::Object *obj;

    {
      boost::mutex::scoped_lock sl(_mutexServices);
      std::map<unsigned int, qi::Object*>::iterator it;
      it = _services.find(msg.service());
      obj = it->second;
      if (it == _services.end() || !obj)
      {
        qiLogError("qi::Server") << "Can't find service: " << msg.service();
        return;
      }
    }
    qi::FunctorParameters ds(msg.buffer());

    ServerFunctorResult promise(client, msg);
    obj->metaCall(msg.function(), ds, promise);
  };


  Server::Server()
    : _p(new ServerPrivate())
  {
//    _p->_ts.setCallbacks(this);
  }

  Server::~Server()
  {
    delete _p;
  }

  bool Server::listen(qi::Session *session, const std::vector<std::string> &endpoints)
  {
    bool success = true;

    _p->_session = session;
    _p->_ts.setCallbacks(_p);

    for (std::vector<std::string>::const_iterator it = endpoints.begin();
         it != endpoints.end();
         it++)
    {
      qi::Url url(*it);

      switch (url.protocol())
      {
      case Url::Protocol_Tcp:
        if (_p->_ts.start(session, url))
        {
          _p->_endpoints.push_back(_p->_ts._p->listenUrl.str());
        }
        else
        {
          success = false;
        }
        break;
      default:
        success = false;
        break;
      }
    }

    return success;
  }


  unsigned int Server::registerService(const std::string& name, qi::Object *obj)
  {
    if (!_p->_session)
    {
      qiLogError("qimessaging.Server") << "no session attached to the server.";
      return 0;
    }

    qi::Message msg;
    qi::Buffer  buf;
    qi::ServiceInfo si;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

    qi::DataStream d(buf);
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId("TODO");
    si.setEndpoints(_p->_endpoints);
    d << si;

    msg.setBuffer(buf);
    _p->_session->_p->_serviceSocket->send(msg);
    _p->_session->_p->_serviceSocket->waitForId(msg.id());
    qi::Message ans;
    _p->_session->_p->_serviceSocket->read(msg.id(), &ans);
    qi::DataStream dout(ans.buffer());
    unsigned int idx = 0;
    dout >> idx;
    si.setServiceId(idx);
    {
      boost::mutex::scoped_lock sl(_p->_mutexServices);
      _p->_services[idx] = obj;
    }
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      _p->_servicesInfo[name] = si;
      _p->_servicesByName[name] = obj;
      _p->_servicesIndex[idx] = name;
    }
    return idx;
  };

  void Server::unregisterService(unsigned int idx)
  {
    if (!_p->_session)
    {
      qiLogError("qimessaging.Server") << "no session attached to the server.";
      return;
    }

    qi::Message msg;
    qi::Buffer  buf;
    msg.setType(qi::Message::Type_Call);
    msg.setService(qi::Message::Service_ServiceDirectory);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(qi::Message::ServiceDirectoryFunction_UnregisterService);

    qi::DataStream d(buf);
    d << idx;

    msg.setBuffer(buf);
    _p->_session->_p->_serviceSocket->send(msg);
    _p->_session->_p->_serviceSocket->waitForId(msg.id());
    qi::Message ans;
    _p->_session->_p->_serviceSocket->read(msg.id(), &ans);
    {
      boost::mutex::scoped_lock sl(_p->_mutexServices);
      _p->_services.erase(idx);
    }
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      std::map<unsigned int, std::string>::iterator it;
      it = _p->_servicesIndex.find(idx);
      if (it == _p->_servicesIndex.end()) {
        qiLogError("qimessaging.Server") << "Can't find name associated to id:" << idx;
      }
      else {
        _p->_servicesByName.erase(it->second);
        _p->_servicesInfo.erase(it->second);
      }
      _p->_servicesIndex.erase(idx);
    }
  };

  void Server::stop() {
    qiLogError("qimessaging.Server") << "stop is not implemented";
  }

  std::vector<qi::ServiceInfo> Server::registeredServices() {
    std::vector<qi::ServiceInfo> ssi;
    std::map<std::string, qi::ServiceInfo>::iterator it;

    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      for (it = _p->_servicesInfo.begin(); it != _p->_servicesInfo.end(); ++it) {
        ssi.push_back(it->second);
      }
    }
    return ssi;
  }

  qi::ServiceInfo Server::registeredService(const std::string &service) {
    std::map<std::string, qi::ServiceInfo>::iterator it;
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      it = _p->_servicesInfo.find(service);
      if (it != _p->_servicesInfo.end())
        return it->second;
    }
    return qi::ServiceInfo();
  }

  qi::Object *Server::registeredServiceObject(const std::string &service) {
    std::map<std::string, qi::Object *>::iterator it;
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      it = _p->_servicesByName.find(service);
      if (it != _p->_servicesByName.end())
        return it->second;
    }
    return 0;
  }

}
