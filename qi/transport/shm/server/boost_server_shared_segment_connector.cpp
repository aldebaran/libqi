/*
 * boost_server_shared_segment_connector.cpp
 *
 *  Created on: Oct 13, 2009 at 3:26:29 PM
 *      Author: Jean-Charles DELAY
 *       Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/exceptions/exceptions.hpp>
#include <alcommon-ng/transport/shm/server/boost_server_shared_segment_connector.hpp>

#include <boost/interprocess/shared_memory_object.hpp>

namespace boi = boost::interprocess;

namespace AL {
  namespace Transport {

BoostServerSharedSegmentConnector::BoostServerSharedSegmentConnector (const std::string & segment_name) :
  ServerSharedSegmentConnector(segment_name), segment(0), mapped_region(0) {
}

BoostServerSharedSegmentConnector::~BoostServerSharedSegmentConnector () {
  this->disconnect();
}

void BoostServerSharedSegmentConnector::connect () {
  if (mapped_region)
    return;

  boi::shared_memory_object::remove(segment_name.c_str());

  try {
    boi::shared_memory_object block(boi::create_only, segment_name.c_str(), boi::read_write);
    block.truncate(sizeof(ProcessSharedSegment));
    mapped_region = new boi::mapped_region(block, boi::read_write);
    void * addr = mapped_region->get_address();
    segment = new (addr) ProcessSharedSegment;
  } catch (boi::interprocess_exception & e) {
    if (mapped_region)
      delete mapped_region;
    mapped_region = 0;

    throw ServerException(std::string("shm_create: ") + e.what());
  }
}

void BoostServerSharedSegmentConnector::disconnect () {
  if (mapped_region)
    delete mapped_region;

  mapped_region = 0;
  // segment freed with the region

  boi::shared_memory_object::remove(segment_name.c_str());
}

void BoostServerSharedSegmentConnector::notifyInit () {
  assert (segment != 0);
  segment->request_cond.notify_all();
}

void BoostServerSharedSegmentConnector::broadcast () {
  assert (segment != 0);
  segment->request_cond.notify_all();
}

DefinitionType BoostServerSharedSegmentConnector::getType () const {
  assert (segment != 0);
  return segment->type;
}

}
}
