#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SERVER_IMPL_HPP_
#define _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SERVER_IMPL_HPP_

#include <zmq.hpp>
#include <qi/core/handlers_pool.hpp>
#include <qi/transport/src/server_backend.hpp>
#include <qi/transport/src/server_response_handler.hpp>
#include <qi/transport/src/zmq/zmq_server_impl.hpp>
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
      //class ResultHandler;
      class ZMQServerImpl : public detail::ServerBackend, public detail::ServerResponseHandler {
      public:
        /// <summary> The Server class constructor. </summary>
        /// <param name="server_name">
        /// The addresses given to the server
        /// </param>
        ZMQServerImpl(const std::vector<std::string> & serverAddresses);

        /// <summary> The Server class destructor </summary>
        virtual ~ZMQServerImpl();

        /// <summary> Run the server thread. </summary>
        virtual void run();

        /// <summary> Wait for the server thread to complete its task. </summary>
        void wait();

        zmq::message_t *recv(zmq::message_t &msg);

        /// <summary> Force the server to stop and wait for complete stop. </summary>
        void stop();

        void poll();
        void serverResponseHandler(const std::string &result, void *data = 0);

        //ResultHandler *getResultHandler() { return 0; }

        friend void *worker_routine(void *arg);

      private:
        bool                     server_running;
        zmq::context_t           zctx;
        zmq::socket_t            zsocket;
        boost::mutex             socketMutex;
        qi::detail::HandlersPool handlersPool;
      };
    }
  }
}
#endif  // _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SERVER_IMPL_HPP_
