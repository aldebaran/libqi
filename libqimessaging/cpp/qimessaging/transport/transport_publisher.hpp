#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_TRANSPORT_TRANSPORT_PUBLISHER_HPP_
#define _QIMESSAGING_TRANSPORT_TRANSPORT_PUBLISHER_HPP_

#include <string>
#include <vector>
#include <qimessaging/api.hpp>

namespace qi {
  namespace transport {

    namespace detail {
      class PublisherBackend;
    }
    class TransportContext;

    /// <summary>Transport Publisher. </summary>
    /// \ingroup Transport
    class QIMESSAGING_API TransportPublisher {
    public:

      /// <summary>Constructor. </summary>
      /// <param name="ctx">The transport context.</param>
      TransportPublisher(TransportContext &ctx);

      /// <summary>Finaliser. </summary>
      ~TransportPublisher();

      /// <summary>Initialises this object. </summary>
      void init();

      /// <summary>Connects to an endpoint</summary>
      /// <param name="endpoint">
      /// The fully qualified endpoint to connect to.
      /// </param>
      virtual void connect(const std::string& endpoint);

      /// <summary>Binds to an endpoint </summary>
      /// <param name="endpoint">
      /// The fully qualified endpoint to bind to.
      /// </param>
      virtual void bind(const std::string& endpoint);

      /// <summary>Binds to multiple endpoints </summary>
      /// <param name="endpoints">
      /// A vector of fully qualified endpoints to bind to.
      /// </param>
      virtual void bind(const std::vector<std::string>& endpoints);

      /// <summary>Publishes a message </summary>
      /// <param name="message">The message.</param>
      virtual void publish(const std::string& message);

    protected:

      /// <summary> Context for the transport </summary>
      qi::transport::TransportContext          &_transportContext;

      /// <summary> The backend publisher </summary>
      qi::transport::detail::PublisherBackend *_publisher;
    };
  }
}

#endif  // _QIMESSAGING_TRANSPORT_TRANSPORT_PUBLISHER_HPP_
