/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/


// THIS FILE LOOKS LIKE A DYING RELIC ..... DESTROY ?

#ifndef     _QI_MESSAGING_TRANSPORT_HPP_
# define     _QI_MESSAGING_TRANSPORT_HPP_

#include <qi/config.hpp>
#include <qi/transport/common/common.hpp>

#ifdef WITH_SHM
# include <qi/transport/shm/shm.hpp>
#endif

#ifdef WITH_ZMQ
# include <qi/transport/zeromq/zeromq.hpp>
#endif

//force Server to be a ShmServer for the moment
namespace qi {
  namespace transport {
    //typedef ShmServer Server;
    //typedef ShmClient Client;

    //typedef ZMQServer Server;
    //typedef ZMQClient Client;
  }
}

#endif      /* !TRANSPORT_PP_ */
