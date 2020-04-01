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
    std::vector<ServiceInfo> services(RelativeEndpointsUriEnabled relativeEndpointsUri);
    ServiceInfo              service(const std::string &name);
    ServiceInfo              service(const std::string &name,
                                     RelativeEndpointsUriEnabled relativeEndpointsUri);
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

    // The state of the relative endpoints URI feature.
    //
    // To be effective, this function must be called within one of the advertised methods, i.e.
    // when the service bound object is treating an incoming call request, as this function will
    // attempt to get the current socket on which the message was received.
    //
    // If the bound object cannot be accessed or has no current socket associated with, returns
    // an empty value.
    ka::opt_t<RelativeEndpointsUriEnabled> relativeEndpointsUriEnabled() const;

    // Finalizes a service info, first by removing any existing relative endpoints and removing
    // duplicate endpoints. Then, if the feature is enabled, adds relative endpoints according to
    // existing registered services. Finally, sorts the endpoints by preference
    // (see spec:/sbre/framework/2020/b).
    ServiceInfo finalize(ServiceInfo info, RelativeEndpointsUriEnabled relativeEndpointsUri) const;

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
