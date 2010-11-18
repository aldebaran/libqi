/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/client.hpp>
#include <string>
#include <qi/messaging/detail/client_impl.hpp>
#include <qi/serialization/serializer.hpp>

namespace qi {

  /// <summary>
  /// Used to call services that have been added to a server. If the service
  /// is unknown, the master is interogated to find the appropriate server.
  /// </summary>
  Client::Client() {}

  /// <summary> Destructor. </summary>
  Client::~Client() {}

  /// <summary>
  /// DefaultConstructor Used to call services that have been added to a
  /// server. If the service is unknown, the master is interogated
  /// to find the appropriate server.
  /// </summary>
  /// <param name="clientName"> Name of the client. </param>
  /// <param name="masterAddress"> The master address. </param>
  Client::Client(const std::string& clientName,
                         const std::string& masterAddress)
    : fImp(new detail::ClientImpl(clientName, masterAddress))
  {}

  void Client::callVoid(const std::string& methodName) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)()  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    xCall(hash, calldef, resultdef);
  }

  void Client::xCall(const std::string &signature,
    const qi::serialization::BinarySerializer& callDef,
    qi::serialization::BinarySerializer &result) {
    return fImp->call(signature, callDef, result);
  }
}
