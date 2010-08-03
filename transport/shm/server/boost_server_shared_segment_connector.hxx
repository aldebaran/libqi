/*
 * boost_server_shared_segment_connector.hxx
 *
 *  Created on: Oct 13, 2009 at 3:30:10 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alippc/transport/shm/server/boost_server_shared_segment_connector.hpp>
#include <alippc/transport/shm/memory/process_shared_segment.hpp>

#include <cassert>
#include <cstring>

namespace AL {
  namespace Messaging {

template <typename L>
L & BoostServerSharedSegmentConnector::getMutex () {
  assert (segment != 0);

  return segment->request_mutex;
}

template <typename L>
std::string BoostServerSharedSegmentConnector::waitForRequest (L & lock, bool & server_running) {
  assert (segment != 0);

  segment->op_mutex.unlock();
  segment->request_cond.wait(lock);

  if (!server_running)
    return "";

  char buf [SEGMENT_NAME_MAX];
  std::memcpy(buf, segment->invite, SEGMENT_NAME_MAX);
  return std::string(buf, SEGMENT_NAME_MAX);
}

template <typename L>
void BoostServerSharedSegmentConnector::resync (L & lock) {
  assert (segment != 0);

  boost::posix_time::ptime bTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::seconds(3000); // dhoussin time
  if (!segment->request_cond.timed_wait(lock, bTime))
    throw ServerException("resync with client timed out");
}

}
}