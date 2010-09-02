/*
 * server_shared_segment_connector.hpp
 *
 *  Created on: Oct 13, 2009 at 3:24:35 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_SERVERSHAREDSEGMENTCONNECTOR_HPP_
#define LIBIPPC_SERVERSHAREDSEGMENTCONNECTOR_HPP_

#include <string>

namespace AL {
  namespace Transport {

/**
 * @brief An abstract class used to synchronize a server with an other process
 * during the remote procedure call.
 */
class ServerSharedSegmentConnector {
public:
  ServerSharedSegmentConnector (const std::string & segment_name) { this->segment_name = segment_name; }
  virtual ~ServerSharedSegmentConnector () {}

  virtual void connect ()    = 0;
  virtual void disconnect () = 0;

  virtual void notifyInit () = 0;
  virtual void broadcast ()  = 0;

  // missing waitForRequest, but cannot be virtual because of template
  // std::string waitForRequest ();

protected:
  std::string segment_name;
};

}
}

#endif /* !LIBIPPC_SERVERSHAREDSEGMENTCONNECTOR_HPP_ */
