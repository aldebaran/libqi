/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SERVER_QUEUE_IMPL_HPP__
#define   __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SERVER_QUEUE_IMPL_HPP__

#include <zmq.hpp>
#include <qi/transport/server.hpp>
#include <qi/core/handlers_pool.hpp>
#include <qi/transport/detail/zmq/zmq_server_impl.hpp>
#include <string>
#include <boost/thread/mutex.hpp>
#include <qi/transport/detail/server_impl.hpp>


namespace qi {
  namespace transport {
    namespace detail {

      /// <summary>
      /// The server class. It listen for incoming connection from client
      /// and push handlers for those connection to the tread pool.
      /// This class need to be instantiated and run at the beginning of the process.
      /// </summary>
      class ResultHandler;
      class ZMQServerQueueImpl : public detail::ServerImpl, public detail::ServerResponseHandler {
      public:
        /// <summary>The Server class constructor.</summary>
        /// <param name="serverAddresses">
        /// The addresses to serve
        /// </param>
        ZMQServerQueueImpl(const std::vector<std::string> & serverAddresses);

        /// <summary>The Server class destructor.
        virtual ~ZMQServerQueueImpl();

        /// <summary>Run the server thread.</summary>
        virtual void run();

        /// <summary>Wait for the server thread to complete its task.</summary>
        void wait();

        /// <summary>Force the server to stop and wait for complete stop.</summary>
        void stop();

        void serverResponseHandler(const std::string &result, void *data = 0);

        ResultHandler *getResultHandler() { return 0; }

        friend void *worker_routine(void *arg);

      private:
        bool                server_running;
        std::string         server_path;
        zmq::context_t      zctx;
        zmq::socket_t       zsocketworkers;
        zmq::socket_t       zsocket;
        boost::mutex        socketMutex;
        HandlersPool        handlersPool;
      };

    }
  }
}
#endif // __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SERVER_QUEUE_IMPL_HPP__
