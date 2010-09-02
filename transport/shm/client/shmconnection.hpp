/*
 * connection.hpp
 *
 *  Created on: Oct 8, 2009 at 12:26:40 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_CONNECTION_HPP_
#define LIBIPPC_CONNECTION_HPP_

#include <alcommon-ng/transport/shm/client/boost_process_shared_segment_connector.hpp>
#include <alcommon-ng/transport/shm/transport_config.hpp>
#include <alcommon-ng/transport/shm/definition_type.hpp>

#include <alcommon-ng/transport/shm/client/result_handler.hpp>

#include <string>

namespace AL {
  namespace Transport {

class CallDefinition;
class ResultDefinition;


/**
 * @brief The main client class when sending any data to the server.
 * It connects to the server, sync with it, and send a definition in
 * order to ask for a procedure call.
 */
class ShmConnection {
public:
  /**
   * @brief The ShmConnection class constructor.
   * @param server_name The server id name in order to connect to it.
   */
  ShmConnection (const std::string & server_name, ResultHandler & resultHandler);

  /**
   * @brief The ShmConnection class destructor.
   */
  virtual ~ShmConnection ();



  virtual void send(const std::string &tosend, std::string &result);
  virtual void send(const std::string &result);

private:

  void init (DefinitionType type);

  /**
   * @brief Initialize the connection with the server.
   */
  void handShake (DefinitionType type);

  /**
   * @brief The target server name.
   */
  std::string server_name;

  /**
   * @brief The shared memory segment name used to transmit data between client and server.
   */
  char invite[SEGMENT_NAME_MAX];

  /**
   * @brief The connector to the target process shared segment.
   */
  BoostProcessSharedSegmentConnector connector;

  ResultHandler & resultHandler;
};

}
}

#endif /* !LIBIPPC_CONNECTION_HPP_ */
