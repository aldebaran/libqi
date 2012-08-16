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

    bool connect(const qi::Url &serviceDirectoryURL);
    bool disconnect();
    bool join();
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

    bool waitForServiceReady(const std::string &service, int msecs = 30000);

    bool isConnected() const;
    qi::Future< std::vector<ServiceInfo> > services();

    qi::Future< qi::TransportSocket* > serviceSocket(const std::string &name,
                                                     unsigned int      *idx,
                                                     const std::string &type = std::string("any"));

    qi::Future< qi::Object* > service(const std::string &service,
                                      const std::string &type = std::string("any"));

    void addCallbacks(SessionInterface *delegate);
    void removeCallbacks(SessionInterface *delegate);

    qi::Url url() const;

    bool listen(const std::string &address);
    void close();

    qi::Future<unsigned int> registerService(const std::string &name,
                                             qi::Object *obj);
    qi::Future<void>         unregisterService(unsigned int idx);

    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::Object                   *registeredServiceObject(const std::string &service);

    qi::Url                       listenUrl() const;

    SessionPrivate      *_p;
  };
}



#endif  // _QIMESSAGING_SESSION_HPP_
