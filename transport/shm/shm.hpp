/*
 * transport.hpp
 *
 *  Created on: Oct 1, 2009 at 10:40:56 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_TRANSPORT_HPP_
#define LIBIPPC_TRANSPORT_HPP_

#include <alippc/transport/shm/transport_config.hpp>
#include <alippc/transport/shm/client/shmclient.hpp>
#include <alippc/transport/shm/client/shmconnection.hpp>
#include <alippc/transport/shm/server/shmserver.hpp>
#include <alippc/transport/common/handlers_pool.hpp>

#ifndef static_assert
# include <boost/static_assert.hpp>
# define static_assert   BOOST_STATIC_ASSERT
#endif

#include <alippc/transport/shm/memory/mapped_shared_segment.hpp>

namespace AL {
  namespace Messaging {
    static_assert (sizeof(MappedSharedSegment) == SHARED_SEGMENT_SIZE);
  }
}

#endif /* !LIBIPPC_TRANSPORT_HPP_ */
