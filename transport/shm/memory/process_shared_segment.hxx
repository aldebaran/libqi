/*
 * process_shared_segment.hxx
 *
 *  Created on: Oct 8, 2009 at 11:57:23 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/transport/shm/memory/process_shared_segment.hpp>

namespace AL {
  namespace Messaging {

template <typename L>
void ProcessSharedSegment::request (const char * invite, L & init_lock) {
  std::memcpy(this->invite, invite, SEGMENT_NAME_MAX);
  init_cond.wait(init_lock);
}

}
}

