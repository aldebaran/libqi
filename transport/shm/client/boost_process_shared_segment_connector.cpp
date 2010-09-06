/*
 * boost_process_shared_segment_connector.cpp
 *
 *  Created on: Oct 8, 2009 at 12:42:24 PM
 *      Author: Jean-Charles DELAY
 *       Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/exceptions/exceptions.hpp>
#include <alcommon-ng/transport/shm/client/boost_process_shared_segment_connector.hpp>

#include <boost/interprocess/shared_memory_object.hpp>

namespace AL {
  namespace Transport {

BoostProcessSharedSegmentConnector::BoostProcessSharedSegmentConnector (const std::string & process_segment_name) :
  process_segment_name(process_segment_name), segment(0), mapped_region(0) {
}

BoostProcessSharedSegmentConnector::~BoostProcessSharedSegmentConnector () {
  this->disconnect();
}

void BoostProcessSharedSegmentConnector::handShake (const char invite[SEGMENT_NAME_MAX], DefinitionType type) {
  this->connect();
  assert (segment != 0);
  boost::posix_time::ptime bTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::seconds(3);
  if (!segment->op_mutex.timed_lock(bTime))
  {
    throw ServerException("timeout BoostProcessSharedSegmentConnector::op_mutex");
  }

  boi::scoped_lock<boi::interprocess_mutex> timed_lock(segment->request_mutex, bTime);
  std::memcpy(segment->invite, invite, SEGMENT_NAME_MAX);
  segment->type = type;
  segment->request_cond.notify_all();
  // dhoussin timeout
  if (!segment->request_cond.timed_wait(timed_lock, bTime))
    throw ServerException("timeout BoostProcessSharedSegmentConnector::request_cond");
}

void BoostProcessSharedSegmentConnector::notifyWrite () {
  segment->request_cond.notify_all();
}

void BoostProcessSharedSegmentConnector::connect () {
  if (mapped_region)
    return;

  try {
    boi::shared_memory_object block(boi::open_only, process_segment_name.c_str(), boi::read_write);
    mapped_region = new boi::mapped_region(block, boi::read_write);
    void * addr = mapped_region->get_address();
    segment = static_cast<ProcessSharedSegment *> (addr);
  } catch (boi::interprocess_exception & e) {
    if (mapped_region)
      delete mapped_region;
    mapped_region = 0;

    throw ConnectionException(std::string("shm_open: ") + e.what());
  }

  if (!segment) {
    delete mapped_region;
    mapped_region = 0;
    throw std::bad_alloc();
  }
}

void BoostProcessSharedSegmentConnector::lock () {
  assert (segment != 0);
  segment->request_mutex.lock();
}

void BoostProcessSharedSegmentConnector::unlock () {
  assert (segment != 0);
  segment->request_mutex.unlock();
}

void BoostProcessSharedSegmentConnector::disconnect () {
  if (mapped_region)
    delete mapped_region;

  mapped_region = 0;
}

}
}
