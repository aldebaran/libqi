/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_OBJECTPRIVATE_HPP_
#define _QIMESSAGING_OBJECTPRIVATE_HPP_

#include <iostream>
#include <string>
#include <boost/thread/recursive_mutex.hpp>

#include <qi/atomic.hpp>

#include <qimessaging/api.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/metamethod.hpp>
#include <qimessaging/signal.hpp>

namespace qi {

  class EventLoop;
  class ManageablePrivate
  {
  public:
    // Links that target us. Needed to be able to disconnect upon destruction
    std::vector<SignalSubscriber>       registrations;
    boost::mutex                        registrationsMutex;
    std::map<ObjectInterface *, void *> callbacks;
    boost::mutex                        callbacksMutex;
    bool                                dying;
    // Event loop in which calls are made
    EventLoop                          *eventLoop;
  };

};

#endif
