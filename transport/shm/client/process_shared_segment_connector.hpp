/*
 * process_shared_segment_connector.hpp
 *
 *  Created on: Oct 8, 2009 at 12:41:55 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_PROCESSSHAREDSEGMENTCONNECTOR_HPP_
#define LIBIPPC_PROCESSSHAREDSEGMENTCONNECTOR_HPP_

#include <alcommon-ng/transport/shm/transport_config.hpp>
#include <alcommon-ng/transport/shm/definition_type.hpp>

namespace AL {
  namespace Transport {

/**
 * @brief An abstract class used to synchronize a process with an other one's
 * before the remote procedure call.
 */
class ProcessSharedSegmentConnector {
public:
  virtual ~ProcessSharedSegmentConnector () {}

  virtual void handShake (const char invite [SEGMENT_NAME_MAX], DefinitionType type) = 0;
  virtual void notifyWrite () = 0;

  virtual void lock () = 0;
  virtual void unlock () = 0;
};

}
}

#endif /* !LIBIPPC_PROCESSSHAREDSEGMENTCONNECTOR_HPP_ */
