#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SESSION_HPP_
#define _QIMESSAGING_SESSION_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/genericobject.hpp>

#include <vector>
#include <string>

namespace qi {

  class SessionPrivate;

  class QIMESSAGING_API SessionInterface
  {
  public:
    virtual ~SessionInterface() = 0;
    inline virtual void onSessionConnected(Session *QI_UNUSED(session), void *data)
    {
      qiLogVerbose("session.hpp") << "onSessionConnected not implemented";
    }

    inline virtual void onSessionConnectionError(Session *QI_UNUSED(session), void *data)
    {
      qiLogVerbose("session.hpp") << "onSessionConnectionError not implemented";
    }

    inline virtual void onSessionDisconnected(Session *QI_UNUSED(session), void *data)
    {
      qiLogVerbose("session.hpp") << "onSessionDisconnected not implemented";
    }

    inline virtual void onServiceRegistered(Session *QI_UNUSED(session),
                                            const std::string &QI_UNUSED(serviceName), void *data)
    {
      qiLogVerbose("session.hpp") << "onServiceRegistered not implemented";
    }

    inline virtual void onServiceUnregistered(Session *QI_UNUSED(session),
                                              const std::string &QI_UNUSED(serviceName), void *data)
    {
      qiLogVerbose("session.hpp") << "onServiceUnregistered not implemented";
    }
  };

  class QIMESSAGING_API Session {
  public:
    Session();
    virtual ~Session();

    enum ServiceLocality {
      ServiceLocality_All    = 0,
      ServiceLocality_Local  = 1,
      ServiceLocality_Remote = 2
    };

    void addCallbacks(SessionInterface *delegate, void *data = 0);
    void removeCallbacks(SessionInterface *delegate);

    //Client
    qi::FutureSync<bool> connect(const qi::Url &serviceDirectoryURL);
    bool isConnected() const;
    qi::Url url() const;

    bool waitForServiceReady(const std::string &service, int msecs = 30000);

    qi::Future< std::vector<ServiceInfo> > services(ServiceLocality locality = ServiceLocality_All);

    qi::Future< qi::GenericObject > service(const std::string &service,
                                     ServiceLocality locality = ServiceLocality_All,
                                     const std::string &protocol  = std::string("any"));

    //Server
    bool    listen(const std::string &address);
    qi::Url listenUrl() const;

    //close both client and server side
    qi::FutureSync<void>    close();

    qi::FutureSync<unsigned int> registerService(const std::string &name, const qi::GenericObject &object);
    qi::Future<void>         unregisterService(unsigned int idx);


    /// Load a module and register an instance of each declared object as a service.
    std::vector<std::string>      loadService(const std::string& name, int flags = -1);

    SessionPrivate      *_p;
  };
}



#endif  // _QIMESSAGING_SESSION_HPP_
