/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/server.hpp>
#include <qi/messaging/detail/server_impl.hpp>

namespace qi {
  /// <summary> Default constructor. </summary>
  Server::Server() {}
  Server::~Server() {}

  /// <summary> Default constructor. </summary>
  /// <param name="serverName"> Name of the server. </param>
  /// <param name="serverAddress"> The server address. </param>
  /// <param name="masterAddress"> The master address. </param>
  Server::Server(const std::string& serverName,
                         const std::string& serverAddress,
                         const std::string& masterAddress) :
  fImp(new detail::ServerImpl(serverName, serverAddress, masterAddress)) {
  }

  void Server::xAddService(const std::string& methodSignature, qi::Functor* functor) {
    fImp->addService(methodSignature, functor);
  }
}
