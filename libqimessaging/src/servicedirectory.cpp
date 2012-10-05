/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <qimessaging/genericobject.hpp>
#include <qimessaging/transportserver.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/objecttypebuilder.hpp>
#include "src/transportserver_p.hpp"
#include "src/serverresult.hpp"
#include "src/session_p.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/url.hpp>
#include "src/servicedirectory_p.hpp"
#include "src/signal_p.hpp"

namespace qi
{

  qi::ObjectPtr createSDP(ServiceDirectoryPrivate* self) {
    qi::ObjectTypeBuilder<ServiceDirectoryPrivate> ob;

    ob.advertiseMethod("service", &ServiceDirectoryPrivate::service);
    ob.advertiseMethod("services", &ServiceDirectoryPrivate::services);
    ob.advertiseMethod("registerService", &ServiceDirectoryPrivate::registerService);
    ob.advertiseMethod("unregisterService", &ServiceDirectoryPrivate::unregisterService);
    ob.advertiseMethod("serviceReady", &ServiceDirectoryPrivate::serviceReady);
    return ob.object(self);
  }

  ServiceDirectoryPrivate::ServiceDirectoryPrivate()
    : _server()
    , servicesCount(0)
    , currentSocket()
  {
    _object = createSDP(this);
    _server.newConnection.connect(boost::bind(&ServiceDirectoryPrivate::onTransportServerNewConnection, this, _1));

    ServiceInfo si;
    si.setName("serviceDirectory");
    si.setServiceId(1);
    si.setMachineId(qi::os::getMachineId());
    unsigned int regid = registerService(si);
    serviceReady(1);
    //serviceDirectory must have id '1'
    assert(regid == 1);
    // Make calls synchronous on net event loop so that the 'currentSocket' hack
    // works.
    _object->moveToEventLoop(getDefaultNetworkEventLoop());
    /*
     * Order is important. See qi::Message::ServiceDirectoryFunctions.
     */
  }

  ServiceDirectoryPrivate::~ServiceDirectoryPrivate()
  {
    {
      boost::recursive_mutex::scoped_lock sl(_clientsMutex);
      for (std::set<TransportSocketPtr>::iterator i = _clients.begin();
        i!= _clients.end(); ++i)
      {
        //TODO: do not call reset. (SD should be a server, server should handle that properly)
        (*i)->disconnected._p->reset();
        (*i)->messageReady._p->reset();
        (*i)->disconnect();
      }
      _clients.clear();
    } // Lock must not be held while deleting session, or deadlock
  }

  void ServiceDirectoryPrivate::onTransportServerNewConnection(TransportSocketPtr socket)
  {
    boost::recursive_mutex::scoped_lock sl(_clientsMutex);
    if (!socket)
      return;
    socket->disconnected.connect(boost::bind<void>(&ServiceDirectoryPrivate::onSocketDisconnected, this, _1, socket));
    socket->messageReady.connect(boost::bind<void>(&ServiceDirectoryPrivate::onMessageReady, this, _1, socket));
    socket->startReading();
    _clients.insert(socket);
  }

  void ServiceDirectoryPrivate::onMessageReady(const qi::Message &msg, qi::TransportSocketPtr socket)
  {
    currentSocket  = socket;

    if (msg.service() != 1) {
      if (msg.type() == qi::Message::Type_Call) {
        qi::Message ans(qi::Message::Type_Error, msg.address());
        qi::Buffer buf;
        qi::ODataStream od(buf);
        std::stringstream ss;
        od << "s";
        ss << "unknown request at address: " << msg.address();
        od << ss.str();
        ans.setBuffer(buf);
        socket->send(ans);
      }
      return;
    }
    qiLogDebug("ServiceDirectory") << "Processing message " << msg.id() << ' ' << msg.function() << ' ' << msg.buffer().size();
    std::string sig = signatureSplit(_object->metaObject().method(msg.function())->signature())[2];
    sig = sig.substr(1, sig.length()-2);
    qi::Future<GenericValue> res = _object->metaCall(msg.function(),
      GenericFunctionParameters::fromBuffer(sig, msg.buffer()), MetaCallType_Direct);
    res.connect(boost::bind<void>(serverResultAdapter, _1, socket, msg.address()));

    currentSocket.reset();
  }

  void ServiceDirectoryPrivate::onSocketDisconnected(int error, TransportSocketPtr socket)
  {
    boost::recursive_mutex::scoped_lock sl(_clientsMutex);
    _clients.erase(socket);
    currentSocket = socket;

    // if services were connected behind the socket
    std::map<TransportSocketPtr, std::vector<unsigned int> >::iterator it;
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
    currentSocket.reset();
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
#if 0
    std::map<TransportSocketPtr , std::vector<unsigned int> >::iterator socketIt;
    for (socketIt = socketToIdx.begin(); socketIt != socketToIdx.end(); ++socketIt)
    {
      // notify every session that the service is unregistered
      qi::Message msg;
      msg.setType(qi::Message::Type_Event);
      msg.setService(qi::Message::Service_Server);
      msg.setObject(qi::Message::GenericObject_Main);
      msg.setEvent(qi::Message::ServiceDirectoryEvent_ServiceUnregistered);

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
#endif
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

#if 0
    std::map<TransportSocketPtr, std::vector<unsigned int> >::iterator socketIt;
    for (socketIt = socketToIdx.begin(); socketIt != socketToIdx.end(); ++socketIt)
    {
      qi::Message msg;
      msg.setType(qi::Message::Type_Event);
      msg.setService(qi::Message::Service_Server);
      msg.setObject(qi::Message::GenericObject_Main);
      msg.setEvent(qi::Message::ServiceDirectoryEvent_ServiceRegistered);

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
#endif
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

  if (_p->_server.listen(address))
  {
    qiLogVerbose("qimessaging.ServiceDirectory") << "Started ServiceDirectory at " << _p->_server.listenUrl().str();

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
  _p->_server.close();
  {
    std::set<TransportSocketPtr> socks;
    {
      boost::recursive_mutex::scoped_lock sl(_p->_clientsMutex);
      socks = _p->_clients;
    }
    // Lock must not be held while disconnecting from signal
    for (std::set<TransportSocketPtr>::iterator it = socks.begin();
         it != socks.end(); ++it)
    {
      (*it)->disconnected._p->reset();
      (*it)->messageReady._p->reset();
      (*it)->disconnect();
    }
  }
  {
    boost::recursive_mutex::scoped_lock sl(_p->_clientsMutex);
    _p->_clients.clear();
  } // Lock must not be held while deleting session, or deadlock
}

qi::Url ServiceDirectory::listenUrl() const {
  return _p->_server.listenUrl();
}


}; // !qi
