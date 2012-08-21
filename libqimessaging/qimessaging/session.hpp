/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_SESSION_HPP_
#define _QIMESSAGING_SESSION_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/service_info.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/object.hpp>

#include <vector>
#include <string>

namespace qi {

  class SessionPrivate;

  class QIMESSAGING_API SessionInterface
  {
  public:
    virtual ~SessionInterface() = 0;
    inline virtual void onSessionConnected(Session *QI_UNUSED(session))
    {
      qiLogVerbose("session.hpp") << "onSessionConnected not implemented";
    }

    inline virtual void onSessionConnectionError(Session *QI_UNUSED(session))
    {
      qiLogVerbose("session.hpp") << "onSessionConnectionError not implemented";
    }

    inline virtual void onSessionDisconnected(Session *QI_UNUSED(session))
    {
      qiLogVerbose("session.hpp") << "onSessionDisconnected not implemented";
    }

    inline virtual void onServiceRegistered(Session *QI_UNUSED(session),
                                            const std::string &QI_UNUSED(serviceName))
    {
      qiLogVerbose("session.hpp") << "onServiceRegistered not implemented";
    }

    inline virtual void onServiceUnregistered(Session *QI_UNUSED(session),
                                              const std::string &QI_UNUSED(serviceName))
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

    bool connect(const qi::Url &serviceDirectoryURL);
    bool isConnected() const;

    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);
    bool waitForServiceReady(const std::string &service, int msecs = 30000);

    qi::Future< std::vector<ServiceInfo> > services(ServiceLocality locality = ServiceLocality_All);

    qi::Future< qi::Object* > service(const std::string &service,
                                      ServiceLocality locality = ServiceLocality_All,
                                      const std::string &protocol  = std::string("any"));

    void addCallbacks(SessionInterface *delegate);
    void removeCallbacks(SessionInterface *delegate);

    qi::Url url() const;

    bool listen(const std::string &address);
    void close();

    qi::Future<unsigned int> registerService(const std::string &name,
                                             qi::Object *obj);
    qi::Future<void>         unregisterService(unsigned int idx);

    qi::Url                       listenUrl() const;

    /// Load a module and register an instance of each declared object as a service.
    std::vector<std::string>      loadService(const std::string& name, int flags = -1);

    SessionPrivate      *_p;
  };
}



#endif  // _QIMESSAGING_SESSION_HPP_
