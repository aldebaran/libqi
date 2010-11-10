/*
 * transport.hpp
 *
 *  Created on: Oct 1, 2009 at 10:40:56 AM
 *      Author: Jean-Charles DELAY
 *       Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_TRANSPORT_HPP_
#define LIBIPPC_TRANSPORT_HPP_

#include <alcommon-ng/transport/shm/transport_config.hpp>
#include <alcommon-ng/transport/shm/client/shmclient.hpp>
#include <alcommon-ng/transport/shm/client/shmconnection.hpp>
#include <alcommon-ng/transport/shm/server/shmserver.hpp>
#include <alcommon-ng/transport/common/handlers_pool.hpp>

#ifndef static_assert
# include <boost/static_assert.hpp>
# define static_assert   BOOST_STATIC_ASSERT
#endif

#include <alcommon-ng/transport/shm/memory/mapped_shared_segment.hpp>

namespace AL {
  namespace Transport {
    static_assert (sizeof(MappedSharedSegment) == SHARED_SEGMENT_SIZE);
  }
}

#endif /* !LIBIPPC_TRANSPORT_HPP_ */
