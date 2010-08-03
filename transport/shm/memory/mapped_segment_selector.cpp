/*
 * mapped_segment_selector.cpp
 *
 *  Created on: Oct 12, 2009 at 10:27:06 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/exceptions/exceptions.hpp>
#include <alcommon-ng/transport/shm/memory/mapped_segment_selector.hpp>

#include <boost/interprocess/shared_memory_object.hpp>

#ifndef foreach
# include <boost/foreach.hpp>
# define foreach    BOOST_FOREACH
#endif

namespace AL {
  namespace Messaging {

MappedSegmentSelector::MappedSegmentSelector () {
}

MappedSegmentSelector::~MappedSegmentSelector () {
  {
    boi::scoped_lock<boi::interprocess_mutex> lock(map_use);

    foreach (segments_map_elt elt, segments)
      this->closeSharedSegment(elt.first, elt.second);
  }
}

MappedSegmentSelector & MappedSegmentSelector::instance () {
  static MappedSegmentSelector instance;

  return instance;
}

MappedSharedSegment * MappedSegmentSelector::get (const char segment_name [SEGMENT_NAME_MAX], unsigned int flags) {
  {
    boi::scoped_lock<boi::interprocess_mutex> lock(map_use);
//    std::cout << ">>> Map size = " << segments.size() << std::endl;
	  segments_map::iterator it = segments.find(segment_name);

	  if (it != segments.end())
	    return (*it).second.segment;
  }

  SegmentDefinition sd;
  sd.flags = flags;

  if ((flags & MappedSegmentSelector::MS_CREATE) && (flags & MappedSegmentSelector::MS_OPEN))
    this->createOrOpenSharedSegment(segment_name, sd);
  else if (flags & MappedSegmentSelector::MS_CREATE)
    this->createSharedSegment(segment_name, sd);
  else if (flags & MappedSegmentSelector::MS_OPEN)
    this->openSharedSegment(segment_name, sd);
  else {
    throw SharedSegmentInitializationException("please specify open mode for streambuf (create/open)");
  }

	boi::scoped_lock<boi::interprocess_mutex> lock(map_use);
	segments[segment_name] = sd;
	return sd.segment;
}

void MappedSegmentSelector::free (const char segment_name [SEGMENT_NAME_MAX]) {
  {
    boi::scoped_lock<boi::interprocess_mutex> lock(map_use);
	  segments_map::iterator it = segments.find(segment_name);

	  if (it == segments.end())
	    return;

	  this->closeSharedSegment(segment_name, (*it).second);
	  segments.erase(it);
  }
}

void MappedSegmentSelector::createSharedSegment (const char segment_name [SEGMENT_NAME_MAX], SegmentDefinition & sd) {
  boi::shared_memory_object::remove(segment_name);

  boi::mapped_region * mapped_region = 0;
  MappedSharedSegment * segment = 0;

  try {
//    boi::shared_memory_object block(boi::create_only, segment_name, boi::read_write);
    boi::shared_memory_object block(boi::create_only, segment_name, boi::read_write); // CHANGES
    block.truncate(sizeof(MappedSharedSegment));
    mapped_region = new boi::mapped_region(block, boi::read_write);
    void * addr = mapped_region->get_address();
    std::memset(addr, 0, sizeof(MappedSharedSegment));
    segment = new (addr) MappedSharedSegment;
  } catch (const boi::interprocess_exception & e) {
    if (mapped_region)
      delete mapped_region;
    throw SharedSegmentInitializationException(std::string("shm_create: ") + e.what());
  }

  if (!segment) {
    delete mapped_region;
    throw std::bad_alloc();
  }

  sd.region = mapped_region;
  sd.segment = segment;
}

void MappedSegmentSelector::openSharedSegment (const char segment_name [SEGMENT_NAME_MAX], SegmentDefinition & sd) {
  boi::mapped_region * mapped_region = 0;
  MappedSharedSegment * segment = 0;

  try {
    boi::shared_memory_object block(boi::open_only, segment_name, boi::read_write);
    mapped_region = new boi::mapped_region(block, boi::read_write);
    void * addr = mapped_region->get_address();
    segment = static_cast<MappedSharedSegment *> (addr);
  } catch (const boi::interprocess_exception & e) {
    if (mapped_region)
      delete mapped_region;
    throw SharedSegmentInitializationException(std::string("shm_open: ") + e.what());
  }

  if (!segment) {
    delete mapped_region;
    throw std::bad_alloc();
  }

  sd.region = mapped_region;
  sd.segment = segment;
}

void MappedSegmentSelector::createOrOpenSharedSegment (const char segment_name [SEGMENT_NAME_MAX], SegmentDefinition & sd) {
  boi::mapped_region * mapped_region = 0;
  MappedSharedSegment * segment = 0;

  try {
	  try {
	    boi::shared_memory_object block(boi::create_only, segment_name, boi::read_write);
	    block.truncate(sizeof(MappedSharedSegment));
	    mapped_region = new boi::mapped_region(block, boi::read_write);
	    void * addr = mapped_region->get_address();
	    std::memset(segment, 0, sizeof(MappedSharedSegment));
	    segment = new (addr) MappedSharedSegment;
	  } catch (const boi::interprocess_exception & /*e*/) {
	    boi::shared_memory_object block(boi::open_only, segment_name, boi::read_write);
	    mapped_region = new boi::mapped_region(block, boi::read_write);
	    void * addr = mapped_region->get_address();
	    segment = static_cast<MappedSharedSegment *> (addr);
	  }
  }
  catch (const boi::interprocess_exception & e) {
    if (mapped_region)
      delete mapped_region;
    throw SharedSegmentInitializationException(std::string("shm_create_or_open: ") + e.what());
  }

  if (!segment) {
    delete mapped_region;
    throw std::bad_alloc();
  }

  sd.region = mapped_region;
  sd.segment = segment;
}

void MappedSegmentSelector::closeSharedSegment (const SegmentName segment_name, SegmentDefinition & sd) {
  if (sd.flags & MappedSegmentSelector::MS_REMOVE) {
    boi::shared_memory_object::remove(segment_name.value);
  }

  delete sd.region;
}

}
}
