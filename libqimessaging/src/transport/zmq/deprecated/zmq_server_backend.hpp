#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_ZMQ_ZMQ_SERVER_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_ZMQ_ZMQ_SERVER_BACKEND_HPP_

#include <zmq.hpp>
#include <qi/core/handlers_pool.hpp>
#include "src/transport/server_backend.hpp"
#include "src/transport/server_response_handler.hpp"
#include "src/transport/zmq/zmq_server_backend.hpp"
#include <string>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      /// <summary>
      /// The server class. It listen for incoming connection from client
      /// and push handlers for those connection to the tread pool.
      /// This class need to be instantiated and run at the beginning of the process.
      /// </summary>
      /// \ingroup Transport
      class ZMQServerBackend : public detail::ServerBackend, public detail::ServerResponseHandler {
      public:
        /// <summary> The Server class constructor. </summary>
        /// <param name="server_name">
        /// The addresses given to the server
        /// </param>
        ZMQServerBackend(const std::vector<std::string> &serverAddresses, zmq::context_t &context);

        /// <summary> The Server class destructor </summary>
        virtual ~ZMQServerBackend();

        /// <summary> Run the server thread. </summary>
        virtual void run();

        /// <summary> Wait for the server thread to complete its task. </summary>
        void wait();

        zmq::message_t *recv(zmq::message_t &msg);

        /// <summary> Force the server to stop and wait for complete stop. </summary>
        void stop();

        bool poll(long timeout);
        void serverResponseHandler(const std::string &result, void *data = 0);

        //ResultHandler *getResultHandler() { return 0; }

        friend void *worker_routine(void *arg);

      private:
        bool                     _running;
        zmq::context_t          &_zcontext;
        zmq::socket_t            _zsocket;
        boost::mutex             _socketMutex;
        qi::detail::HandlersPool _handlersPool;
      };
    }
  }
}
#endif  // _QI_TRANSPORT_SRC_ZMQ_ZMQ_SERVER_BACKEND_HPP_
