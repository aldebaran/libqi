/*
 * connection_handler.hpp
 *
 *  Created on: Oct 13, 2009 at 2:26:49 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_CONNECTIONHANDLER_HPP_
#define LIBIPPC_CONNECTIONHANDLER_HPP_

#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/transport/common/runnable.hpp>

#include <alcommon-ng/transport/common/server_command_delegate.hpp>
#include <alcommon-ng/transport/common/server_response_delegate.hpp>

#include <string>

namespace AL {
  namespace Messaging {

/**
 * @brief A connection handler created for each new incoming connection and pushed to
 * the thread pool.
 */
class ConnectionHandler : public Runnable {
public:
  /**
   * @brief The ConnectionHandler class constructor
   * @param rdv_name The shared memory name used to share data between the client and server.
   */
  ConnectionHandler (const std::string & rdv_name, ServerCommandDelegate *callback, internal::ServerResponseDelegate *responsedelegate);

  /**
   * @brief The ConnectionHandler class destructor.
   */
  virtual ~ConnectionHandler ();

  /**
   * @brief The method used by the Thread Pool to run the handler.
   * It syncs with the client, retrieve data, and send result back.
   */
  virtual void run ();

private:
  /**
   * @brief The shared memory name for sharing data.
   */
  std::string rdv_name;

  /**
   * The callback to call after recieving a CallDefinition
   */
  ServerCommandDelegate * callback;

  /**
   * The delegate used to send the response
   */
  internal::ServerResponseDelegate * response_delegate;
};

}
}
#endif /* !LIBIPPC_CONNECTIONHANDLER_HPP_ */
