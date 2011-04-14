#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_
#define _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_

#include <string>
#include <qimessaging/transport/buffer.hpp>

namespace qi {
  namespace transport {

    namespace detail {
      class ClientBackend;
    }

    /// <summary>Transport context. </summary>
    class TransportContext;

    /// \ingroup Transport
    class TransportClient {
    public:
      virtual ~TransportClient();

      /// <summary>Constructor. </summary>
      /// <param name="context">The context.</param>
      TransportClient(TransportContext &context);

      /// <summary>Connects to an endpoint</summary>
      /// <param name="endpoint">The fully qualified endpoint to connect to.</param>
      /// <returns>true if it succeeds, false if it fails.</returns>
      bool connect(const std::string &endpoint);

      /// <summary>Sends a message.</summary>
      /// <param name="request">The request.</param>
      /// <param name="reply">[in,out] The reply.</param>
      void send(const qi::transport::Buffer &request, qi::transport::Buffer &reply);

    protected:

      /// <summary> Context for the transport </summary>
      qi::transport::TransportContext      &_transportContext;

      /// <summary> true if is initialized </summary>
      bool                                  _isInitialized;

      /// <summary> The client </summary>
      qi::transport::detail::ClientBackend *_client;
    };

  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_
