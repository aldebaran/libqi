/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
*/

#ifndef   __QI_TRANSPORT_DETAIL_THRIFT_THRIFT_SERVER_HPP__
#define   __QI_TRANSPORT_DETAIL_THRIFT_THRIFT_SERVER_HPP__

# include <qi/transport/server.hpp>
# include <qi/transport/common/handlers_pool.hpp>

namespace qi {
  namespace transport {
    class ResultHandler;
    class ThriftServer : public Server {
    public:
      /**
         * @brief The Server class constructor.
         * @param server_name The name given to the server, id for clients to connect.
         */
      ThriftServer(const std::string & server_name);

      /**
         * @brief The Server class destructor.
         */
      virtual ~ThriftServer();

      /**
         * @brief Run the server thread.
         */
      virtual void run();

      /**
         * @brief Wait for the server thread to complete its task.
         */
      void wait();

      /**
         * @brief Force the server to stop and wait for complete stop.
         */
      void stop();

      void sendResponse(const std::string &result, void *data = 0);

      ResultHandler *getResultHandler() { return 0; }

      friend void *worker_routine(void *arg);

    private:
      HandlersPool        handlersPool;
    };
  }
}



#endif // __QI_TRANSPORT_DETAIL_THRIFT_THRIFT_SERVER_HPP__
