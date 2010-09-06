/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/


// THIS FILE LOOKS LIKE A DYING RELIC ..... DESTROY ?

#ifndef     _AL_MESSAGING_TRANSPORT_HPP_
# define     _AL_MESSAGING_TRANSPORT_HPP_

#include <alcommon-ng/config.hpp>
#include <alcommon-ng/transport/common/common.hpp>

#ifdef WITH_SHM
# include <alcommon-ng/transport/shm/shm.hpp>
#endif

#ifdef WITH_ZMQ
# include <alcommon-ng/transport/zeromq/zeromq.hpp>
#endif

//force Server to be a ShmServer for the moment
namespace AL {
  namespace Transport {
    //typedef ShmServer Server;
    //typedef ShmClient Client;

    //typedef ZMQServer Server;
    //typedef ZMQClient Client;
  }
}

#endif      /* !TRANSPORT_PP_ */
