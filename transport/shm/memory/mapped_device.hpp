/*
 * mapped_device.hpp
 *
 *  Created on: Oct 9, 2009 at 3:16:52 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_MAPPEDDEVICE_HPP_
#define LIBIPPC_MAPPEDDEVICE_HPP_

#include <alcommon-ng/transport/shm/memory/mapped_shared_segment.hpp>
#include <alcommon-ng/transport/shm/client/process_shared_segment_connector.hpp>

#include <boost/iostreams/categories.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace io = boost::iostreams;

namespace AL {
  namespace Messaging {

/**
 * @brief A device used by Boost.iostreams directly mapped to a shared memory
 * segment which allow real time serialization/de-serialization.
 */
class MappedDevice {
public:
  /**
   * typedefs used by Boost to recognize this class as a Device for its Boost.iostreams.
   */
  typedef char                          char_type;
  typedef io::bidirectional_device_tag  category;

public:
  /**
   * @brief Copy constructor used by Boost.iostreams. Be careful to correctly copy the data :
   * Boost copy this class twice before using it !
   */
  MappedDevice (const MappedDevice & device);

public:
  /**
   * @brief The MappedDevice class constructor.
   * @param segment Segment to use to communicate and send/receive the data.
   */
  MappedDevice (MappedSharedSegment * segment);

  /**
   * @brief The MappedDevice class constructor.
   * @param segment Segment to use to communicate and send/receive the data.
   * @param connector The connector used by the client to notify the server that it's
   * writing for the first time. This is need for synchonizing purpose.
   */
  MappedDevice (MappedSharedSegment * segment, const ProcessSharedSegmentConnector * connector);

  /**
   * @brief The MappedDevice destructor.
   */
  virtual ~MappedDevice ();

  /**
   * @brief The read method inherited from the Boost.BidirectionalDevice class. Read at most sz
   * bytes and save them into s. Then return the number of read bytes.
   * @param s The buffer used to store the read bytes.
   * @param sz The maximum number of bytes to read.
   * @return The effective number of read bytes.
   */
  std::streamsize read (char* s, std::streamsize sz);

  /**
   * @brief The write method inherited from the Boost.BidirectionalDevice class. write sz
   * bytes from s. Then return the number of wrote bytes.
   * @param s The buffer used to store the bytes to write.
   * @param sz The number of bytes to write.
   * @return The effective number of wrote bytes.
   */
  std::streamsize write (const char* s, std::streamsize sz);

protected:
  /**
   * @brief The shared memory segment to used to send/receive data.
   */
  MappedSharedSegment * segment;

  /**
   * Some pointers used to manipulate the shared memory buffer.
   * @brief The pointer marking the beginning of the buffer.
   */
  byte * bptr;

  /**
   * @brief The pointer marking the writing head in the buffer.
   */
  byte * wptr;

  /**
   * @brief The pointer marking the reading head in the buffer.
   */
  byte * rptr;

  /**
   * @brief The number of bytes read since the creation of the device.
   */
  std::streamsize nb_read;

  /**
   * @brief The connector to the server shared memory used by the client to send notifications.
   */
  const ProcessSharedSegmentConnector * connector;

  /**
   * @brief A simple boolean true if this->write is called for the first_time.
   */
  bool first_time_writing;

  /**
   * @brief A simple boolean true if this->read is called for the first_time.
   */
  bool first_time_reading;

  /**
   * @brief Lock used for synchronize purpose.
   */
  boi::scoped_lock<boi::interprocess_mutex> * lock;
};

}
}

#endif /* !LIBIPPC_MAPPEDDEVICE_HPP_ */
