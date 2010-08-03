/*
 * mapped_device.cpp
 *
 *  Created on: Oct 9, 2009 at 3:16:52 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alippc/transport/shm/memory/mapped_device.hpp>
#include <alippc/exceptions/exceptions.hpp>

#include <cstring>
#include <cassert>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <alippc/transport/shm/client/boost_process_shared_segment_connector.hpp>

#ifdef IPPC_DEBUG_HARD
# include <string>
# include <iostream>
#endif

namespace boi = boost::interprocess;

namespace AL {
  namespace Messaging {

MappedDevice::MappedDevice (MappedSharedSegment * segment) :
  segment(segment), bptr(0), wptr(0), rptr(0), nb_read(0), connector(0), first_time_writing(true), first_time_reading(true) {
  assert (this->segment != 0);

  bptr = wptr = rptr = this->segment->data;
  lock = new boi::scoped_lock<boi::interprocess_mutex>(segment->rw_mutex);
}

MappedDevice::MappedDevice (MappedSharedSegment * segment, const ProcessSharedSegmentConnector * connector) :
  segment(segment), bptr(0), wptr(0), rptr(0), nb_read(0), connector(connector), first_time_writing(true), first_time_reading(true) {
  assert (this->segment != 0);

  bptr = wptr = rptr = this->segment->data;
  lock = new boi::scoped_lock<boi::interprocess_mutex>(segment->rw_mutex);
}

MappedDevice::MappedDevice(const MappedDevice & device) {
#ifdef IPPC_DEBUG_HARD
  std::cout << "[debug][MappedDevice copy-contructor]" << std::endl;
#endif
  segment = device.segment;
  bptr = device.bptr;
  rptr = device.rptr;
  wptr = device.wptr;
  nb_read = device.nb_read;
  connector = device.connector;
  first_time_writing = device.first_time_writing;
  first_time_reading = device.first_time_reading;
  lock = device.lock;
}

MappedDevice::~MappedDevice () {
#ifdef IPPC_DEBUG_HARD
  std::cout << "[debug][MappedDevice destructor]" << std::endl;
#endif
  if (!first_time_writing) {
	  segment->done = true;
	  segment->rw_cond.notify_all();
	  delete lock;
  }

  if (!first_time_reading) {
    delete lock;
  }

}

std::streamsize MappedDevice::read (char* s, std::streamsize sz) {
  assert (segment != 0);

  if (!first_time_reading) {
    if (!segment->done) {
//      std::cout << "[DEBUG][READ] !first_time_reading && !segment->done so waiting for some write" << std::endl;
//      segment->rw_cond.wait(*lock);
		  boost::posix_time::ptime bTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::seconds(3);
	    if (!segment->rw_cond.timed_wait(*lock, bTime))
		    throw ReadException("wait for write from server timed out");
    }
  } else
    first_time_reading = false;

//	std::cout << "done = " << (segment->done ? "true" : "false") << " | bytes = " << segment->bytes << " | wrote = " << segment->wrote << " | nd_read = " << nb_read << std::endl;

  if(segment->done && (segment->bytes == nb_read)) {
#ifdef IPPC_DEBUG_HARD
  std::cout << "[debug][read] sz = " << sz << " && read s = `' [0]" << std::endl;
#endif
    return 0;
  }

  if (segment->done && segment->bytes <= (MAX_DATA_SIZE - (rptr - bptr))) {
    std::streamsize to_read = sz <= (segment->bytes - nb_read) ? sz : segment->bytes - nb_read;
    std::memcpy(s, rptr, to_read);
    nb_read = to_read;
    rptr += to_read;
#ifdef IPPC_DEBUG_HARD
	  std::cout << "[debug][read] sz = " << sz << " [" << to_read << "]" << std::endl;
#endif
    return segment->bytes;
  }

  std::streamsize total = 0;

#ifdef IPPC_DEBUG_HARD
  unsigned int loop_no = 0;
#endif

  while (!segment->done || (segment->wrote != total)) {
    std::streamsize to_read = (MAX_DATA_SIZE - (rptr - bptr)) < (segment->wrote - total) ? MAX_DATA_SIZE - (rptr - bptr) : segment->wrote - total;
//    std::cout << "to_read = " << to_read << std::endl;
    to_read = (sz - total) <= to_read ? sz - total : to_read;
    to_read = (segment->wrote - nb_read) <= to_read ? segment->wrote - nb_read : to_read;
//    std::cout << "to_read = " << to_read << std::endl;
//		std::cout << "sz = " << sz << " | done = " << (segment->done ? "true" : "false") << " | bytes = " << segment->bytes << " | wrote = " << segment->wrote << " | nd_read = " << nb_read << " | total = " << total << std::endl;
    std::memcpy(s + total, rptr, to_read);
    rptr += to_read;
    total += to_read;
    nb_read += to_read;

    segment->rw_cond.notify_all();

//		std::cout << "sz = " << sz << " | done = " << (segment->done ? "true" : "false") << " | bytes = " << segment->bytes << " | wrote = " << segment->wrote << " | nd_read = " << nb_read << " | total = " << total << std::endl;
#ifdef IPPC_DEBUG_HARD
	  std::cout << "Loop #" << loop_no++ << " read " << to_read << " bytes" << std::endl;
#endif

    if ((segment->done && segment->wrote == total) || (total == sz) || (segment->bytes == nb_read))
      break;

//    segment->rw_cond.wait(*lock);
	  boost::posix_time::ptime bTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::seconds(3);
    if (!segment->rw_cond.timed_wait(*lock, bTime))
	    throw ReadException("wait for write from server timed out");

    rptr = bptr;
  };

#ifdef IPPC_DEBUG_HARD
  std::cout << "[debug][read] sz = " << sz << " [" << total << "]" << std::endl;
#endif


  return total;
}

std::streamsize MappedDevice::write (const char* s, std::streamsize sz) {
  assert (segment != 0);

  if (!first_time_writing) {
//    std::cout << "[DEBUG][WRITING] !first_time_writing so notifying other side that currently writing" << std::endl;
    segment->rw_cond.notify_all();
  }

//  std::cout << "[DEBUG] Connector = " << connector << " && first_time_writing = " << (first_time_writing ? "true" : "false") << std::endl;
  if (connector && first_time_writing) {
#ifdef IPPC_DEBUG_HARD
    std::cout << "[debug] Waking up server for reading datas" << std::endl;
#endif
    const BoostProcessSharedSegmentConnector * bc = dynamic_cast<const BoostProcessSharedSegmentConnector *>(connector);
    assert (bc != 0);
    BoostProcessSharedSegmentConnector * c = const_cast<BoostProcessSharedSegmentConnector *>(bc);
    c->notifyWrite();
    first_time_writing = false;
  }

#ifdef IPPC_DEBUG_HARD
  std::cout << "[debug][write] sz = " << sz << std::endl;
#endif

  segment->bytes += sz;
  std::streamsize total = 0;

#ifdef IPPC_DEBUG_HARD
  unsigned int loop_no = 0;
#endif

  while (sz != total) {
    std::streamsize to_write = (MAX_DATA_SIZE - (wptr - bptr)) < (sz - total) ? MAX_DATA_SIZE - (wptr - bptr) : sz - total;
    std::memcpy(wptr, s + total, to_write);
    wptr += to_write;
    total += to_write;
    segment->wrote += to_write;

    segment->rw_cond.notify_all();

#ifdef IPPC_DEBUG_HARD
	  std::cout << "Loop #" << loop_no++ << " wrote " << to_write << " bytes" << std::endl;
#endif

	  boost::posix_time::ptime bTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::seconds(3);
    if (!segment->rw_cond.timed_wait(*lock, bTime))
	    throw WriteException("wait for read from server timed out");

	  if (sz == total) {
	    break;
	  }

    wptr = bptr;
  }

  return total;
}

}
}
