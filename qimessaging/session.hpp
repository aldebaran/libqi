#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SESSION_HPP_
#define _QIMESSAGING_SESSION_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qi/future.hpp>
#include <qitype/genericobject.hpp>

#include <vector>
#include <string>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  class SessionPrivate;
  class QIMESSAGING_API Session {
  public:
    QI_DISALLOW_COPY_AND_ASSIGN(Session);
    Session();
    virtual ~Session();

    enum ServiceLocality {
      ServiceLocality_All    = 0,
      ServiceLocality_Local  = 1,
      ServiceLocality_Remote = 2
    };

    //Client
    qi::FutureSync<bool> connect(const qi::Url &serviceDirectoryURL);
    bool isConnected() const;
    qi::Url url() const;

    qi::Future< std::vector<ServiceInfo> > services(ServiceLocality locality = ServiceLocality_All);

    qi::Future< qi::ObjectPtr > service(const std::string &service,
                                        ServiceLocality locality = ServiceLocality_All);

    //Server
    qi::Future<void> listen(const qi::Url &address);
    std::vector<qi::Url> endpoints() const;
    bool    setIdentity(const std::string& key, const std::string& crt);

    //close both client and server side
    qi::FutureSync<void>    close();

    qi::FutureSync<unsigned int> registerService(const std::string &name, ObjectPtr object);
    qi::FutureSync<void>         unregisterService(unsigned int idx);


    /// Load a module and register an instance of each declared object as a service.
    std::vector<std::string>      loadService(const std::string& name, int flags = -1);

  public:
    qi::Signal<void (unsigned int, std::string)> serviceRegistered;
    qi::Signal<void (unsigned int, std::string)> serviceUnregistered;
    // C4251
    qi::Signal<void ()>                          connected;
    // C4251
    qi::Signal<void (int error)>                 disconnected;

  public:
    SessionPrivate      *_p;
  };

  namespace details {
    //This is internal, this could be removed, do not use.
    QIMESSAGING_API void setSessionServerDefaultCallType(qi::Session *session, qi::MetaCallType callType);
  }
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QIMESSAGING_SESSION_HPP_
