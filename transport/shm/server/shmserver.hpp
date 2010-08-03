/*
 * server.hpp
 *
 *  Created on: Oct 8, 2009 at 10:42:17 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_SERVER_HPP_
#define LIBIPPC_SERVER_HPP_

#include <alippc/transport/serverbase.hpp>
#include <alippc/transport/common/handlers_pool.hpp>
#include <alippc/transport/common/threadable.hpp>
#include <alippc/serialization/call_definition.hpp>
#include <alippc/transport/shm/server/boost_server_shared_segment_connector.hpp>

#include <alippc/transport/common/server_command_delegate.hpp>
#include <alippc/transport/common/server_response_delegate.hpp>
#include <alippc/transport/shm/client/result_handler.hpp>
#include <alippc/transport/common/handlers_pool.hpp>
#include <alcore/alptr.h>
#include <althread/althread.h>
#include <string>


namespace AL {
  namespace Messaging {

/**
 * @brief The server class. It listen for incoming connection from client
 * and push handlers for those connection to the tread pool.
 * This class need to be instantiated and run at the beginning of the process.
 */
  class ShmServer : public ServerBase {
  public:
    /**
     * @brief The Server class constructor.
     * @param server_name The name given to the server, id for clients to connect.
     */
    ShmServer (const std::string & server_name);

    /**
     * @brief The Server class destructor.
     */
    virtual ~ShmServer ();

    /**
     * @brief Run the server thread.
     */
    virtual void run ();

    /**
     * @brief Wait for the server thread to complete its task.
     */
    void wait ();

    /**
     * @brief Force the server to stop and wait for complete stop.
     */
    void stop ();

    ResultHandler *getResultHandler ();

    virtual void sendResponse(const CallDefinition &def, AL::ALPtr<ResultDefinition> result, void *data = 0);

  private:
    /**
     * @brief The method called binded into the thread.
     */
    // void _run ();

    /**
     * @brief A simple boolean used to stop the server properly.
     */
    bool server_running;

    /**
     * @brief The server thread (Boost.Thread).
     */
    AL::ALPtr<AL::ALThread> fThread;

    /**
     * @brief The connector to create the public shared zone and synchronize with clients.
     */
    BoostServerSharedSegmentConnector connector;

    ResultHandler resultHandler;

    HandlersPool handlersPool;
  };

}
}

#endif /* !LIBIPPC_SERVER_HPP_ */
