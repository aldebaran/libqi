/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_ZEROMQSIMPLESERVER_HPP_
#define QI_TRANSPORT_ZEROMQSIMPLESERVER_HPP_

#include <qi/transport/detail/serverimpl.hpp>
#include <qi/core/handlers_pool.hpp>
#include <string>
#include <boost/thread/mutex.hpp>

#include <zmq.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      class ResultHandler;

      /// <summary>
      /// The server class. It listen for incoming connection from client
      /// and push handlers for those connection to the tread pool.
      /// This class need to be instantiated and run at the beginning of the process.
      /// </summary>
      class ZMQSimpleServerImpl : public detail::ServerImpl, public detail::ServerResponseHandler {
      public:
        /// <summary>The Server class constructor.</summary>
        /// <param name="serverAddress">
        /// The address of the server e.g. tcp://127.0.0.1:5555
        /// </param>
        ZMQSimpleServerImpl(const std::string & serverAddress);

        /// <summary>The Server class destructor.</summary>
        virtual ~ZMQSimpleServerImpl();

        /// <summary>Run the server thread.</summary>
        virtual void run();

        /// <summary>Wait for the server thread to complete its task.</summary>
        void wait();

        zmq::message_t *recv(zmq::message_t &msg);

        /// <summary>Force the server to stop and wait for complete stop.</summary>
        void stop();

        void poll();
        void serverResponseHandler(const std::string &result, void *data = 0);

        ResultHandler *getResultHandler() { return 0; }

        friend void *worker_routine(void *arg);

      private:
        bool                server_running;
        zmq::context_t      zctx;
        zmq::socket_t       zsocket;
        boost::mutex        socketMutex;
        HandlersPool        handlersPool;
      };
    }
  }
}
#endif  // QI_TRANSPORT_ZEROMQSIMPLESERVER_HPP_
