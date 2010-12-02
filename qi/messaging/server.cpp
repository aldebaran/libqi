/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/server.hpp>
#include <qi/messaging/detail/server_impl.hpp>

namespace qi {
  Server::Server() {}
  Server::~Server() {}

  Server::Server(const std::string& serverName,
                 const std::string& masterAddress) :
    _impl(new detail::ServerImpl(serverName, masterAddress))
  {
    // TODO prevent the name "master"
  }

  void Server::xAddService(const std::string& methodSignature, qi::Functor* functor) {
    _impl->addService(methodSignature, functor);
  }

  bool Server::isInitialized() const {
    return _impl->isInitialized();
  }
}
