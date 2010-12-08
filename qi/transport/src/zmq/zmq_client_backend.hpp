#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_ZMQ_ZMQ_CLIENT_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_ZMQ_ZMQ_CLIENT_BACKEND_HPP_

# include <qi/transport/buffer.hpp>
# include <qi/transport/src/client_backend.hpp>
# include <zmq.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      class ZMQClientBackend : public ClientBackend {
      public:
        /// <summary>
        /// Creates a ZMQClientBackend for a server
        /// </summary>
        /// <param name="serverAddress">
        /// The protocol-qualified address of the server
        /// e.g. ipc:///tmp/naoqi/paf
        //. or tcp://127.0.0.1:5555
        /// </param>
        /// <param name="context"> a zmq context </param>
        ZMQClientBackend(const std::string &servername, zmq::context_t &context);

        virtual void send(const std::string &tosend, std::string &result);
        void pollRecv(long timeout);


      protected:
        void connect();

      protected:
        zmq::context_t &_zcontext;
        zmq::socket_t   _zsocket;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_ZMQ_ZMQ_CLIENT_BACKEND_HPP_
