#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_TRANSPORT_SRC_ZMQ_ZMQ_FORWARDER_HPP_
#define _QI_TRANSPORT_SRC_ZMQ_ZMQ_FORWARDER_HPP_

#include <string>
#include <vector>
#include <zmq.hpp>
#include <boost/shared_ptr.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      /// \ingroup Transport
      class ZMQForwarder {
      public:
        /// <summary> Constructor that allows an existing zmq context to be used </summary>
        /// <param name="context">An existing zmq context</param>
        ZMQForwarder(zmq::context_t &context);

        /// <summary> Destructor </summary>
        virtual ~ZMQForwarder();

        /// <summary>
        /// Runs the forwarder with a single in endpoint and a single out endpoint</summary>
        /// NOTE: This will block the calling thread and can only be released with termination of the process.
        /// <summary>
        /// <param name="inEndpoint"> The input endpoint </param>
        /// <param name="outEndpoint"> The output endpoint </param>
        void run(const std::string& inEndpoint, const std::string& outEndpoint);

        /// <summary>
        /// Runs the forwarder with multiple in and out endpoints
        /// NOTE: This will block the calling thread and can only be released with termination of the process.
        /// </summary>
        /// <param name="inEndpoints"> The input endpoints </param>
        /// <param name="outEndpoints"> The output endpoints </param>
        void run(const std::vector<std::string>& inEndpoints, const std::vector<std::string>& outEndpoints);

        boost::shared_ptr<zmq::context_t> getContext() const;

      protected:
        zmq::context_t &_context;
        zmq::socket_t   _in_socket;
        zmq::socket_t   _out_socket;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_ZMQ_ZMQ_FORWARDER_HPP_
