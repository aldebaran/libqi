#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_PUBLISHER_HPP_
#define _QI_TRANSPORT_TRANSPORT_PUBLISHER_HPP_

#include <string>
#include <vector>

namespace qi {
  namespace transport {

    namespace detail {
      class PublisherBackend;
    }

    class TransportContext;

    class TransportPublisher {
    public:
      TransportPublisher(TransportContext &ctx);
      ~TransportPublisher();

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
      /// <param name="endpoint">
      /// A vector of fully qualified endpoints to bind to.
      /// </param>
      virtual void bind(const std::vector<std::string>& endpoints);

      /// <summary>Publishes a message </summary>
      /// <param name="message">The message.</param>
      virtual void publish(const std::string& message);

    protected:
      qi::transport::detail::PublisherBackend *_publisher;
    };
  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_PUBLISHER_HPP_
