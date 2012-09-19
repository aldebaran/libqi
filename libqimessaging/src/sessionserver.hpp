/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_SERVER_HPP_
#define _QIMESSAGING_SERVER_HPP_

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transportserver.hpp>
#include <qi/atomic.hpp>

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;

  // (service, linkId)
  struct RemoteLink
  {
    RemoteLink()
      : localLinkId(0)
      , event(0)
    {}
    RemoteLink(unsigned int localLinkId, unsigned int event)
    : localLinkId(localLinkId)
    , event(event) {}
    unsigned int localLinkId;
    unsigned int event;
  };

  // remote link id -> local link id
  typedef std::map<unsigned int, RemoteLink> ServiceLinks;
  // service id -> links
  typedef std::map<unsigned int, ServiceLinks> PerServiceLinks;
  // socket -> all links
  // Keep track of links setup by clients to disconnect when the client exits.
  typedef std::map<TransportSocketPtr, PerServiceLinks > Links;


  class QIMESSAGING_API Session_Server : public FutureInterface<unsigned int> {
  public:
    Session_Server(ServiceDirectoryClient *_sdClient);
    virtual ~Session_Server();

    bool listen(const std::string &address);
    void close();

    qi::Future<unsigned int> registerService(const std::string &name, const qi::GenericObject &obj);
    qi::Future<void>         unregisterService(unsigned int idx);

    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::GenericObject                    registeredServiceObject(const std::string &service);

    qi::Url                       listenUrl() const;

  private:
    //TransportServer
    void onTransportServerNewConnection(TransportSocketPtr client);

    //TransportSocket
    void onMessageReady(const qi::Message &msg, qi::TransportSocketPtr socket);
    void onSocketDisconnected(TransportSocketPtr socket, int error);

    //Future
    virtual void onFutureFinished(const unsigned int &future, void *data);
    virtual void onFutureFailed(const std::string &error, void *data);

  private:
    Links                                   _links;

    std::set<TransportSocketPtr>            _clients;
    std::map<unsigned int, qi::GenericObject>      _services;
    std::map<std::string, qi::GenericObject>       _servicesByName;
    std::map<std::string, qi::ServiceInfo>  _servicesInfo;
    //used by registerService
    typedef std::map<long, std::pair<qi::GenericObject, qi::ServiceInfo> > RegisterServiceMap;
    RegisterServiceMap _servicesObject;
    qi::atomic<long>                        _servicesObjectIndex;

    std::map<unsigned int, std::string>     _servicesIndex;
    TransportServer                         _server;
    boost::mutex                            _mutexServices;
    boost::recursive_mutex                  _mutexOthers;
    bool                                    _dying;
    ServiceDirectoryClient                 *_sdClient;
  };
}

#endif  // _QIMESSAGING_SERVER_HPP_
