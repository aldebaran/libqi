/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/context.hpp>
#include <qi/messaging/src/network/uuid.hpp>

namespace qi {
  Context::Context() :
      _contextID(qi::detail::getUUID()),
      _transportContext(new transport::TransportContext()) {}

  Context::Context(const Context& rhs) {
    _contextID = rhs._contextID;
    _transportContext = rhs._transportContext;
  }

  Context::~Context() {}

  const std::string& Context::getID() const {
    return _contextID;
  }

   void Context::setID(const std::string& contextID) {
    _contextID = contextID;
  }

  void Context::setTransportContext(transport::TransportContext *ctx)
  {
    _transportContext = ctx;
  }

  transport::TransportContext &Context::getTransportContext()
  {
    return *_transportContext;
  }

  // Access to a static global context -----
  static qi::Context* gQiContextPtr;

  qi::Context* getDefaultQiContextPtr() {
    if (gQiContextPtr == NULL) {
      gQiContextPtr = new qi::Context(); // who deletes this? need a singleton
    }
    return gQiContextPtr;
  }
  // ---------------------------------------
}

