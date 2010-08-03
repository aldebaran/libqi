/*
 * transport_config.hpp
 *
 *  Created on: Oct 1, 2009 at 12:11:13 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_TRANSPORTCONFIG_HPP_
#define LIBIPPC_TRANSPORTCONFIG_HPP_

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

/**
 * Size of a shared segment. 4Ko is the size of a memory page.
 */
#ifndef SHARED_SEGMENT_SIZE
# include <iostream>
# define SHARED_SEGMENT_SIZE    (std::streamsize) 4096
#endif

/**
 * Size of the sync layer in the shared segment i.e. size of a SharedSegment without the Packet.
 */
#ifndef SYNC_LAYER_SIZE
# define SYNC_LAYER_SIZE        (sizeof(boost::interprocess::interprocess_condition) + \
                                 sizeof(boost::interprocess::interprocess_mutex))
#endif

/**
 * Size max for the string defining a shared segment name.
 */
#ifndef SEGMENT_NAME_MAX
# include <iostream>
# define SEGMENT_NAME_MAX       (std::streamsize) 64
#endif


#endif /* !LIBIPPC_TRANSPORTCONFIG_HPP_ */
