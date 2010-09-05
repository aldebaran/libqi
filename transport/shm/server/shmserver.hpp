/*
 * server.hpp
 *
 *  Created on: Oct 8, 2009 at 10:42:17 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_SERVER_HPP_
#define LIBIPPC_SERVER_HPP_

#include <alcommon-ng/transport/server.hpp>
#include <alcommon-ng/transport/common/handlers_pool.hpp>
#include <alcommon-ng/transport/common/i_threadable.hpp>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/transport/shm/server/boost_server_shared_segment_connector.hpp>


#include <alcommon-ng/transport/common/server_response_delegate.hpp>
#include <alcommon-ng/transport/shm/client/result_handler.hpp>
#include <alcommon-ng/transport/common/handlers_pool.hpp>
#include <alcore/alptr.h>
#include <althread/althread.h>
#include <string>


namespace AL {
  namespace Transport {


/**
 * @brief The server class. It listen for incoming connection from client
 * and push handlers for those connection to the tread pool.
 * This class need to be instantiated and run at the beginning of the process.
 */
  class ShmServer : public Server {
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

    ResultHandler *getResultHandler();

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
