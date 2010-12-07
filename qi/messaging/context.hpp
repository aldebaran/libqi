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
  class Context {
  public:
    Context();
    virtual ~Context();
    Context(const Context& rhs);

    const std::string& getID() const;

    //helper function for Nodes, return a transportContext if one is available
    //TODO: should not be public
    static transport::TransportContext *transportContext(Context *ctx);

    transport::TransportContext *transportContext();

  protected:
    std::string                      _contextID;
    qi::transport::TransportContext *_transportContext;

  };
}

#endif  // _QI_MESSAGING_CONTEXT_HPP_
