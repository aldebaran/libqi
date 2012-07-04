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
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport_server.hpp>

namespace qi {

  class NetworkThread;
  class Object;
  class ServerPrivate;

  /// <summary> Used to advertise named services. Advertised Services are
  /// registered with the service directory so that clients can find them. The exact
  /// signature of your method is composed of the methods name and the
  /// return and argument types of your handler.</summary>
  /// \b Advertise a Service
  /// \include example_qi_server.cpp
  /// \ingroup Messaging
  class QIMESSAGING_API Server {
  public:

    /// <summary> Constructs a Server object that can be used to
    /// advertise services to clients. </summary>
    /// <param name="name"> The name you want to give to the server. </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    Server();
    virtual ~Server();

    /// <summary> Listen on the given set of ports. </summary>
    /// <param name="session" the service directory session on which to advertise.</param>
    /// <param name="url"> the set of urls to listen to. You can use
    /// a port value of 0 to let the system pick an available port.</param>
    bool listen(qi::Session *session, const std::vector<std::string> &url);
    void stop();

    unsigned int registerService(const std::string &name, qi::Object *obj);
    void         unregisterService(unsigned int idx);

    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::Object                   *registeredServiceObject(const std::string &service);

  private:
    ServerPrivate *_p;
  };
}

#endif  // _QIMESSAGING_SERVER_HPP_
