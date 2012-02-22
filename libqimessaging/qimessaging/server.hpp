#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


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
  /// registered with the master so that clients can find them. The exact
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

    void listen(qi::Session *session, const std::vector<std::string> &url);
    void stop();
    void registerService(const std::string &name, qi::Object *obj);

  private:
    ServerPrivate *_p;
  };
}

#endif  // _QIMESSAGING_SERVER_HPP_
