#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_OBJECT_P_HPP_
#define _SRC_OBJECT_P_HPP_

#include <iostream>
#include <string>
#include <boost/thread/recursive_mutex.hpp>

#include <qi/atomic.hpp>

#include <qitype/api.hpp>
#include <qitype/signature.hpp>
#include <qi/future.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/metamethod.hpp>
#include <qitype/signal.hpp>

namespace qi {

  class EventLoop;
  class ManageablePrivate
  {
  public:
    // Links that target us. Needed to be able to disconnect upon destruction
    std::vector<SignalSubscriber>       registrations;
    boost::mutex                        registrationsMutex;
    boost::mutex                        callbacksMutex;
    bool                                dying;
    // Event loop in which calls are made
    EventLoop                          *eventLoop;
  };

};

#endif  // _SRC_OBJECT_P_HPP_
