/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/server.hpp>
#include <qi/messaging/context.hpp>
#include <qi/messaging/src/server_impl.hpp>

namespace qi {
  Server::~Server()
  {
  }

  Server::Server(const std::string& serverName, Context *ctx)
    : _impl(new detail::ServerImpl(serverName, ctx))
  {
    // TODO prevent the name "master"
  }

  void Server::xAdvertiseService(const std::string& methodSignature, qi::Functor* functor) {
    _impl->advertiseService(methodSignature, functor);
  }

  void Server::xUnadvertiseService(const std::string& methodSignature) {
    _impl->unadvertiseService(methodSignature);
  }

  bool Server::isInitialized() const {
    return _impl->isInitialized();
  }
}
