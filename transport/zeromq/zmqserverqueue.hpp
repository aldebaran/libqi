/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_TRANSPORT_ZEROMQ_SERVERQUEUE_HPP_
#define AL_MESSAGING_TRANSPORT_ZEROMQ_SERVERQUEUE_HPP_

#include <zmq.hpp>
#include <alcommon-ng/transport/server.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/transport/common/handlers_pool.hpp>
#include <alcommon-ng/transport/zeromq/zmqserver.hpp>
#include <alcore/alptr.h>
#include <althread/althread.h>
#include <string>
#include <boost/thread/mutex.hpp>

namespace AL {
  namespace Transport {

/**
 * @brief The server class. It listen for incoming connection from client
 * and push handlers for those connection to the tread pool.
 * This class need to be instantiated and run at the beginning of the process.
 */
  class ResultHandler;
  class ZMQServerQueue : public Server {
  public:
    /**
     * @brief The Server class constructor.
     * @param server_name The name given to the server, id for clients to connect.
     */
    ZMQServerQueue(const std::string & server_name);

    /**
     * @brief The Server class destructor.
     */
    virtual ~ZMQServerQueue();

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

#endif /* !AL_MESSAGING_TRANSPORT_ZEROMQ_SERVERQUEUE_HPP_ */
