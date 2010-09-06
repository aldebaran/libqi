/*
 * mapped_shared_segment.cpp
 *
 *  Created on: Oct 8, 2009 at 11:19:32 AM
 *      Author: Jean-Charles DELAY
 *       Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/transport/shm/memory/mapped_shared_segment.hpp>
#include <iostream>

namespace AL {
  namespace Transport {

MappedSharedSegment::MappedSharedSegment () :
  wrote(0), bytes(0), done(false) {
  std::memset(data, 0, MAX_DATA_SIZE);
}

MappedSharedSegment::~MappedSharedSegment () {
}

}
}
