/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <qimessaging/object.hpp>
#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/service_directory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/service_info.hpp>
#include "src/transport_server_p.hpp"
#include "src/server_functor_result_future_p.hpp"
#include "src/session_p.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/url.hpp>
#include "src/service_directory_p.hpp"

namespace qi
{

  ServiceDirectoryPrivate::ServiceDirectoryPrivate()
  :  session(new Session)
    , ts(0)
    , servicesCount(0)
    , currentSocket(0)
  {
    ServiceInfo si;
    si.setName("serviceDirectory");
    si.setServiceId(1);
    unsigned int regid = registerService(si);
    serviceReady(1);
    //serviceDirectory must have id '1'
    assert(regid == 1);
    // Make calls synchronous on net event loop so that the 'currentSocket' hack
    // works.
    moveToEventLoop(getDefaultNetworkEventLoop());
    /*
     * Order is important. See qi::Message::ServiceDirectoryFunctions.
     */
    advertiseMethod("service", this, &ServiceDirectoryPrivate::service);
    advertiseMethod("services", this, &ServiceDirectoryPrivate::services);
    advertiseMethod("registerService", this, &ServiceDirectoryPrivate::registerService);
    advertiseMethod("unregisterService", this, &ServiceDirectoryPrivate::unregisterService);
    advertiseMethod("serviceReady", this, &ServiceDirectoryPrivate::serviceReady);
  }

  ServiceDirectoryPrivate::~ServiceDirectoryPrivate()
  {
    {
      boost::recursive_mutex::scoped_lock sl(_clientsMutex);
      for (std::set<TransportSocket*>::iterator i = _clients.begin();
        i!= _clients.end(); ++i)
      {
        (*i)->removeCallbacks(this);
        delete *i;
      }
      _clients.clear();
    } // Lock must not be held while deleting session, or deadlock
    delete ts;
    delete session;
  }

  void ServiceDirectoryPrivate::newConnection(TransportServer* server, TransportSocket *socket)
  {
    boost::recursive_mutex::scoped_lock sl(_clientsMutex);
    if (!socket)
      return;
    socket->addCallbacks(this);
    _clients.insert(socket);
  }

  void ServiceDirectoryPrivate::onSocketReadyRead(TransportSocket *socket, int id, void *data)
  {
    currentSocket  = socket;

    qi::Message msg;
    socket->read(id, &msg);
    FunctorParameters din(msg.buffer());

    ServerFunctorResult fr(socket, msg);
    metaCall(msg.function(), din, fr);

    currentSocket  = 0;
  }

  void ServiceDirectoryPrivate::onSocketWriteDone(TransportSocket *socket, void *data)
  {
    currentSocket = socket;

    currentSocket = 0;
  }

  void ServiceDirectoryPrivate::onSocketConnected(TransportSocket *socket, void *data)
  {
    currentSocket = socket;

    currentSocket = 0;
  }

  void ServiceDirectoryPrivate::onSocketDisconnected(TransportSocket *socket, void *data)
  {
    boost::recursive_mutex::scoped_lock sl(_clientsMutex);
    _clients.erase(socket);
    currentSocket = socket;

    // if services were connected behind the socket
    std::map<TransportSocket*, std::vector<unsigned int> >::iterator it;
    if ((it = socketToIdx.find(socket)) == socketToIdx.end())
    {
      return;
    }
    // Copy the vector, iterators will be invalidated.
    std::vector<unsigned int> ids = it->second;
    // Always start at the beginning since we erase elements on unregisterService
    // and mess up the iterator
    for (std::vector<unsigned int>::iterator it2 = ids.begin();
         it2 != ids.end();
         ++it2)
    {
      qiLogInfo("qimessaging.ServiceDirectory") << "service "
                                                << connectedServices[*it2].name()
                                                << " (#" << *it2 << ") disconnected"
                                                << std::endl;
      unregisterService(*it2);
    }
    socketToIdx.erase(it);
    currentSocket = 0;
    delete socket;
  }

  std::vector<ServiceInfo> ServiceDirectoryPrivate::services()
  {
    std::vector<ServiceInfo> result;
    std::map<unsigned int, ServiceInfo>::const_iterator it;

    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      result.push_back(it->second);

    return result;
  }

  ServiceInfo ServiceDirectoryPrivate::service(const std::string &name)
  {
    std::map<unsigned int, ServiceInfo>::const_iterator servicesIt;
    std::map<std::string, unsigned int>::const_iterator it;

    it = nameToIdx.find(name);
    if (it == nameToIdx.end())
      return ServiceInfo();

    unsigned int idx = it->second;

    servicesIt = connectedServices.find(idx);
    if (servicesIt == connectedServices.end())
      return ServiceInfo();

    return servicesIt->second;
  }

  unsigned int ServiceDirectoryPrivate::registerService(const ServiceInfo &svcinfo)
  {
    std::map<std::string, unsigned int>::iterator it;
    it = nameToIdx.find(svcinfo.name());
    if (it != nameToIdx.end())
    {
      qiLogWarning("qimessaging.ServiceDirectory")  << "service " << svcinfo.name()
                                                    << " is already registered (#" << it->second << ")" << std::endl;
      return 0;
    }

    unsigned int idx = ++servicesCount;
    nameToIdx[svcinfo.name()] = idx;
    // Do not add serviceDirectory on the map (socket() == null)
    if (idx != qi::Message::Service_ServiceDirectory)
    {
      socketToIdx[socket()].push_back(idx);
    }
    pendingServices[idx] = svcinfo;
    pendingServices[idx].setServiceId(idx);

    qiLogInfo("qimessaging.ServiceDirectory")  << "service " << svcinfo.name() << " registered (#" << idx << ")" << std::endl;
    for (std::vector<std::string>::const_iterator it = svcinfo.endpoints().begin();
         it != svcinfo.endpoints().end();
         ++it)
    {
      qiLogDebug("qimessaging.ServiceDirectory") << svcinfo.name() << " is now on " << *it << std::endl;
    }

    return idx;
  }

  void ServiceDirectoryPrivate::unregisterService(const unsigned int &idx)
  {
    // search the id before accessing it
    // otherwise operator[] create a empty entry
    std::map<unsigned int, ServiceInfo>::iterator it2;
    it2 = connectedServices.find(idx);
    if (it2 == connectedServices.end())
    {
      qiLogError("qimessaging.ServiceDirectory") << "Can't find service #" << idx;
      return;
    }

    std::map<std::string, unsigned int>::iterator it;
    it = nameToIdx.find(connectedServices[idx].name());
    if (it == nameToIdx.end())
    {
      qiLogError("Mapping error, service not in nameToIdx");
      return;
    }
    std::string serviceName = it2->second.name();
    qiLogInfo("qimessaging.ServiceDirectory") << "service "
      << serviceName
      << " (#" << idx << ") unregistered"
      << std::endl;
    nameToIdx.erase(it);
    connectedServices.erase(it2);

    // Find and remove serviceId into socketToIdx map
    std::map<TransportSocket *, std::vector<unsigned int> >::iterator socketIt;
    for (socketIt = socketToIdx.begin(); socketIt != socketToIdx.end(); ++socketIt)
    {
      // notify every session that the service is unregistered
      qi::Message msg;
      msg.setType(qi::Message::Type_Event);
      msg.setService(idx);
      msg.setObject(qi::Message::Object_Main);
      msg.setFunction(qi::Message::ServiceDirectoryFunction_UnregisterService);

      qi::Buffer     buf;
      qi::ODataStream d(buf);
      d << serviceName;

      if (d.status() == qi::ODataStream::Status_Ok)
      {
        msg.setBuffer(buf);
        if (!socketIt->first->send(msg))
        {
          qiLogError("qimessaging.Session") << "Error while unregister service, cannot send event.";
        }
      }

      std::vector<unsigned int>::iterator serviceIdxIt;
      for (serviceIdxIt = socketIt->second.begin();
        serviceIdxIt != socketIt->second.end();
        ++serviceIdxIt)
      {
        if (*serviceIdxIt == idx)
        {
          socketIt->second.erase(serviceIdxIt);
          break;
        }
      }
    }
  }

  void ServiceDirectoryPrivate::serviceReady(const unsigned int &idx)
  {
    // search the id before accessing it
    // otherwise operator[] create a empty entry
    std::map<unsigned int, ServiceInfo>::iterator itService;
    itService = pendingServices.find(idx);
    if (itService == pendingServices.end())
    {
      qiLogError("qimessaging.ServiceDirectory") << "Can't find pending service #" << idx;
      return;
    }

    std::string serviceName = itService->second.name();
    connectedServices[idx] = itService->second;
    pendingServices.erase(itService);

    std::map<TransportSocket*, std::vector<unsigned int> >::iterator socketIt;
    for (socketIt = socketToIdx.begin(); socketIt != socketToIdx.end(); ++socketIt)
    {
      qi::Message msg;
      msg.setType(qi::Message::Type_Event);
      msg.setService(idx);
      msg.setObject(qi::Message::Object_Main);
      msg.setFunction(qi::Message::ServiceDirectoryFunction_RegisterService);

      qi::Buffer     buf;
      qi::ODataStream d(buf);
      d << serviceName;

      if (d.status() == qi::ODataStream::Status_Ok)
      {
        msg.setBuffer(buf);
        if (!socketIt->first->send(msg))
        {
          qiLogError("qimessaging.Session") << "Error while register service, cannot send event.";
        }
      }
    }
  }

ServiceDirectory::ServiceDirectory()
  : _p(new ServiceDirectoryPrivate())
{
}

ServiceDirectory::~ServiceDirectory()
{
  close();
  delete _p;
}

bool ServiceDirectory::listen(const qi::Url &address)
{
  std::vector<std::string> eps;
  ServiceInfo             &si = _p->connectedServices[1];

  eps.push_back(address.str());
  si.setEndpoints(eps);
  _p->ts = new qi::TransportServer(address);
  _p->ts->addCallbacks(_p);

  if (_p->ts->listen())
  {
    qiLogVerbose("qimessaging.ServiceDirectory") << "Started ServiceDirectory at " << _p->ts->listenUrl().str();

    return true;
  }
  else
  {
    qiLogError("qimessaging.ServiceDirectory") << "Could not listen on "
                                               << address.str() << std::endl;
    return false;
  }
}

void ServiceDirectory::close() {
  _p->ts->close();
}

qi::Url ServiceDirectory::listenUrl() const {
  return _p->ts->listenUrl();
}


}; // !qi
