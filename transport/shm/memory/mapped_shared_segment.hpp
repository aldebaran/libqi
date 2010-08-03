/*
 * mapped_shared_segment.hpp
 *
 *  Created on: Oct 8, 2009 at 11:19:32 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_MAPPEDSHAREDSEGMENT_HPP_
#define LIBIPPC_MAPPEDSHAREDSEGMENT_HPP_

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

namespace boi = boost::interprocess;

#ifndef MAX_DATA_SIZE
# include <alippc/transport/shm/transport_config.hpp>
# define MAX_DATA_SIZE (int) (SHARED_SEGMENT_SIZE - (2 * SYNC_LAYER_SIZE) - (2 * sizeof(std::streamsize)) - sizeof(uint32_t))
#endif

#ifdef WIN32
# include <alippc/win_stdint.hpp>
#endif

typedef unsigned char byte;

namespace AL {
  namespace Messaging {


#ifndef _WIN32
#  ifndef PACKED_STRUCT
#    define PACKED_STRUCT __attribute__((__packed__))
#  endif
#else
#  ifndef PACKED_STRUCT
#    define PACKED_STRUCT
#  endif
#endif
/**
 * @brief A shared segment as it's represented into the shared memory.
 * It's the way two process use to communicate and send data.
 * Its size is fixed to 4Ko (a memory page size).
 * This class is directly mapped into the shared memory.
 */
struct MappedSharedSegment {
  MappedSharedSegment ();
  ~MappedSharedSegment ();

  boi::interprocess_mutex     rw_mutex;
  boi::interprocess_condition rw_cond;

  boi::interprocess_mutex     r_mutex; // TODO see if need to be deleted
  boi::interprocess_condition r_cond; // TODO see if need to be deleted

  byte data[MAX_DATA_SIZE];
  std::streamsize wrote    PACKED_STRUCT;
  std::streamsize bytes    PACKED_STRUCT;
  uint32_t done            PACKED_STRUCT;
};

}
}

#endif /* !LIBIPPC_MAPPEDSHAREDSEGMENT_HPP_ */
