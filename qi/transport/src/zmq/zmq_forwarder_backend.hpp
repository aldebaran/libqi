#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_TRANSPORT_SRC_ZMQ_ZMQ_FORWARDER_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_ZMQ_ZMQ_FORWARDER_BACKEND_HPP_

#include <qi/transport/src/forwarder_backend.hpp>
#include <zmq.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      /// \ingroup Transport
      class ZMQForwarderBackend : public ForwarderBackend {
      public:
        /// <summary> Constructor that allows an existing zmq context to be used </summary>
        /// <param name="context">An existing zmq context</param>
        /// <param name="inEndpoints"> The input endpoints </param>
        /// <param name="outEndpoints"> The output endpoints </param>
        ZMQForwarderBackend(
          const std::vector<std::string>& inEndpoints,
          const std::vector<std::string>& outEndpoints,
          zmq::context_t& context);

        /// <summary> Destructor </summary>
        virtual ~ZMQForwarderBackend();

        /// <summary>Binds the forwarder to the in and out addresses</summary>
        void bind();

        /// <summary>
        /// Runs the forwarder</summary>
        /// NOTE: This will block the calling thread and can only be released with termination of the process.
        /// <summary>
        void run();

      protected:
        zmq::context_t &_context;
        zmq::socket_t   _in_socket;
        zmq::socket_t   _out_socket;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_ZMQ_ZMQ_FORWARDER_BACKEND_HPP_
