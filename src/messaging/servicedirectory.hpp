#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEDIRECTORY_HPP_
#define _QIMESSAGING_SERVICEDIRECTORY_HPP_

# include <qi/url.hpp>
# include <qi/future.hpp>
# include "messagesocket.hpp"
# include <boost/thread/recursive_mutex.hpp>
# include "boundobject.hpp"
# include "server.hpp"
# include "objectregistrar.hpp"

namespace qi
{
  class ServiceDirectory {
  public:
    ServiceDirectory();
    virtual ~ServiceDirectory();

  public:
    //Public Bound API
    std::vector<ServiceInfo> services();
    ServiceInfo              service(const std::string &name);
    unsigned int             registerService(const ServiceInfo &svcinfo);
    void                     unregisterService(const unsigned int &idx);
    void                     serviceReady(const unsigned int &idx);
    void                     updateServiceInfo(const ServiceInfo &svcinfo);
    std::string              machineId();
    qi::MessageSocketPtr   _socketOfService(unsigned int id);
    void                     _setServiceBoundObject(BoundObjectPtr bo);

    qi::Signal<unsigned int, std::string>  serviceAdded;
    qi::Signal<unsigned int, std::string>  serviceRemoved;

  private:
    void removeClientSocket(MessageSocketPtr socket);

  public:
    std::map<unsigned int, ServiceInfo>                       pendingServices;
    std::map<unsigned int, ServiceInfo>                       connectedServices;
    std::map<std::string, unsigned int>                       nameToIdx;
    std::map<MessageSocketPtr, std::vector<unsigned int> >  socketToIdx;
    std::map<unsigned int, MessageSocketPtr>                idxToSocket;
    unsigned int                                              servicesCount;
    boost::weak_ptr<BoundObject>                              serviceBoundObject;
    /* Our methods can be invoked from remote, and from socket callbacks,
    * so thread-safety is required.
    */
    boost::recursive_mutex                                    mutex;
  }; // !ServiceDirectoryPrivate


  class ObjectRegistrar;
  class QI_API Session_SD
  {
  public:
    Session_SD(ObjectRegistrar* server);
    ~Session_SD();

    qi::Future<void> listenStandalone(const std::vector<Url>& listenAddresses);
    void             updateServiceInfo();

  private:
    friend class SessionPrivate;
    ObjectRegistrar*             _server;
    BoundObjectPtr               _serviceBoundObject;
    ServiceDirectory*            _sdObject;
    bool                         _init;
  }; // !ServiceDirectory
} // !qi

#endif  // _QIMESSAGING_SERVICEDIRECTORY_HPP_
