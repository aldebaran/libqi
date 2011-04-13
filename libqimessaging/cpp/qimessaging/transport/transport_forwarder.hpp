#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_FORWARDER_HPP_
#define _QI_TRANSPORT_TRANSPORT_FORWARDER_HPP_

#include <string>
#include <vector>
#include <qi/transport/transport_context.hpp>

namespace qi {
  namespace transport {

    namespace detail {
      class ForwarderBackend;
    };

    /// <summary> TransportForwarder</summary>
    /// \ingroup Transport
    class TransportForwarder
    {
    public:
      virtual ~TransportForwarder();

      TransportForwarder(const TransportForwarder& rhs);

      /// <summary>Constructor. </summary>
      /// <param name="context"> The context.</param>
      TransportForwarder(TransportContext &context);

      /// <summary>Binds to the in and out endpoints</summary>
      /// <param name="inEndpoints">The in endpoints.</param>
      /// <param name="outEndpoints">The out endpoints.</param>
      void bind(const std::vector<std::string>& inEndpoints, const std::vector<std::string>& outEndpoints);

      /// <summary>Runs this object. </summary>
      void run();

      /// <summary>Query if this object is initialized. </summary>
      /// <returns>true if initialized, false if not.</returns>
      bool isInitialized();

    protected:
      /// <summary> true if is initialized </summary>
      bool                                  _isInitialized;

      /// <summary> Context for the transport </summary>
      qi::transport::TransportContext      &_transportContext;

      /// <summary> The transport forwarder back end </summary>
      qi::transport::detail::ForwarderBackend *_backend;
    };
  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_FORWARDER_HPP_
