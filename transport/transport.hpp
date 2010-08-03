/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	_IPPC_ROOT_TRANSPORT_HPP_
# define   	_IPPC_ROOT_TRANSPORT_HPP_


#include <alcommon-ng/transport/common/common.hpp>
#include <alcommon-ng/transport/shm/shm.hpp>
#include <alcommon-ng/transport/zeromq/zeromq.hpp>

//force Server to be a ShmServer for the moment
namespace AL {
  namespace Messaging {
//    typedef ShmServer Server;
//    typedef ShmClient Client;

    typedef ZMQServer Server;
    typedef ZMQClient Client;
  }
}

#endif	    /* !TRANSPORT_PP_ */
