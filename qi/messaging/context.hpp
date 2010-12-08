#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_CONTEXT_HPP_
#define _QI_MESSAGING_CONTEXT_HPP_

#include <string>
#include <qi/transport/transport_context.hpp>

namespace qi {

  /// \ingroup Messaging
  class Context {
  public:
    Context();
    virtual ~Context();
    Context(const Context& rhs);

    const std::string& getID() const;
    void setID(const std::string& contextID);

    transport::TransportContext &getTransportContext();
    void setTransportContext(transport::TransportContext* ctx);

  protected:
    std::string                      _contextID;
    qi::transport::TransportContext *_transportContext;

  };

  extern qi::Context* getDefaultQiContextPtr();
}

#endif  // _QI_MESSAGING_CONTEXT_HPP_
