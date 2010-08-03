/*
 * process_shared_segment.hpp
 *
 *  Created on: Oct 8, 2009 at 10:57:04 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_PROCESSSHAREDSEGMENT_HPP_
#define LIBIPPC_PROCESSSHAREDSEGMENT_HPP_

#include <alippc/transport/shm/transport_config.hpp>
#include <alippc/serialization/definition_type.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

namespace boi = boost::interprocess;

namespace AL {
  namespace Messaging {

/**
 * @brief A process personal shared segment that allow it to listen for
 * incoming connection from client.
 * This class is directly mapped into the shared memory.
 */
struct ProcessSharedSegment {
  ProcessSharedSegment ();
  ~ProcessSharedSegment ();

  boi::interprocess_mutex request_mutex;
  boi::interprocess_condition request_cond;

  boi::interprocess_mutex op_mutex;

  char invite[SEGMENT_NAME_MAX];
  DefinitionType type;
};

}
}
//#include "ProcessSharedSegment.hxx"

#endif /* !LIBIPPC_PROCESSSHAREDSEGMENT_HPP_ */
