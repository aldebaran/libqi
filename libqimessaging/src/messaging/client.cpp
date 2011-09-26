/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/client.hpp>
#include <string>
#include "src/messaging/client_impl.hpp"
#include <qimessaging/serialization.hpp>


namespace qi {
  Client::~Client() {}

  Client::Client(const std::string& clientName, Context *ctx)
    : _impl(new detail::ClientImpl(clientName, ctx))
  {
  }

  void Client::connect(const std::string &masterAddress)
  {
    _impl->connect(masterAddress);
  }

  void Client::callVoid(const std::string& methodName) {
    qi::Message msg;
    qi::Message result;

    void (*f)()  = 0;
    std::string signature = makeFunctionSignature(methodName, f);

    msg.writeString(signature);
    xCall(signature, msg, result);
  }

  void Client::xCall(const std::string& signature,
    const qi::Message& msg,
          qi::Message& result)
  {
    return _impl->call(signature, msg, result);
  }

  bool Client::isInitialized() const {
    return _impl->isInitialized();
  }
}
