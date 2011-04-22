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
#include <qimessaging/transport/transport_context.hpp>
#include <qimessaging/api.hpp>

namespace qi {

  /// <summary> Messaging context that can be used to share or separate
  /// resources used by Servers, Clients, Publishers and Subscribers
  /// </summary>
  /// \ingroup Messaging
  class QIMESSAGING_API Context {
  public:

    /// <summary>Default constructor. </summary>
    Context();

    /// <summary>Finaliser. </summary>
    virtual ~Context();

    /// <summary>Gets the context identifier. </summary>
    /// <returns>The identifier.</returns>
    const std::string& getID() const;

    /// <summary>Sets the context identifier. </summary>
    /// <param name="contextID">Identifier for the context.</param>
    void setID(const std::string& contextID);

    /// <summary>Gets the transport context. </summary>
    /// <returns>The transport context.</returns>
    transport::TransportContext &getTransportContext();

    /// <summary>Sets a transport context. </summary>
    /// <param name="ctx">The transport context.</param>
    void setTransportContext(transport::TransportContext* ctx);

  protected:
    /// <summary> Identifier for the context </summary>
    std::string                      _contextID;
    /// <summary> Context for the transport </summary>
    qi::transport::TransportContext *_transportContext;

  private:
    // no copy constructor allowed
    Context(const Context &rhs) {;}
  };

  /// <summary>Gets the default qi context pointer. This is a global context
  /// shared by all messaging components within a process unless their
  /// context has been explicitly set.
  /// </summary>
  /// <returns>The default qi context pointer.</returns>
  extern qi::Context* getDefaultQiContextPtr();
}

#endif  // _QI_MESSAGING_CONTEXT_HPP_
