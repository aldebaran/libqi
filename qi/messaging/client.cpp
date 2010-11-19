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
  Client::Client() {}

  Client::~Client() {}

  Client::Client(const std::string& clientName,
                         const std::string& masterAddress)
    : _impl(new detail::ClientImpl(clientName, masterAddress))
  {}

  void Client::callVoid(const std::string& methodName) {
    qi::serialization::BinarySerializer msg;
    qi::serialization::BinarySerializer result;

    void (*f)()  = 0;
    std::string signature = makeSignature(methodName, f);

    msg.writeString(signature);
    xCall(signature, msg, result);
  }

  void Client::xCall(const std::string& signature,
    const qi::serialization::BinarySerializer& msg,
          qi::serialization::BinarySerializer& result)
  {
    return _impl->call(signature, msg, result);
  }
}
