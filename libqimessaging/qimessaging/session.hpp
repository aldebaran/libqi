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


  class Object;
  class SessionPrivate;

  class QIMESSAGING_API SessionInterface
  {
  public:
    virtual ~SessionInterface() = 0;
    inline virtual void onSessionConnected(Session *QI_UNUSED(session))         {};
    inline virtual void onSessionConnectionError(Session *QI_UNUSED(session))   {};
    inline virtual void onSessionDisconnected(Session *QI_UNUSED(session))      {};
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

    qi::Future< std::vector<ServiceInfo> > services();

    qi::Future< qi::TransportSocket* > serviceSocket(const std::string &name,
                                                     unsigned int      *idx,
                                                     const std::string &type = std::string("any"));

    qi::Future< qi::Object* > service(const std::string &service,
                                      const std::string &type = std::string("any"));

    void setCallbacks(SessionInterface *delegate);

    qi::Url url() const;

    SessionPrivate      *_p;
  };
}



#endif  // _QIMESSAGING_SESSION_HPP_
