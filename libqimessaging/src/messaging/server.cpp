/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/server.hpp>
#include <qimessaging/context.hpp>
#include "src/messaging/server_impl.hpp"

namespace qi {
  Server::~Server()
  {
  }

  Server::Server(const std::string& serverName, Context *ctx)
    : _impl(new detail::ServerImpl(serverName, ctx))
  {
    // TODO prevent the name "master"
  }

  void Server::connect(const std::string& masterAddress) {
    _impl->connect(masterAddress);
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
