#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SIGNAL_P_HPP_
#define _SRC_SIGNAL_P_HPP_

#include <qi/signal.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace qi {

  typedef std::map<SignalLink, SignalSubscriberPtr> SignalSubscriberMap;
  typedef std::map<int, SignalLink> TrackMap;

  class SignalBasePrivate
  {
  public:
    SignalBasePrivate()
      : defaultCallType(MetaCallType_Auto)
    {}

    ~SignalBasePrivate();
    bool disconnectAll(bool wait = true);
    bool disconnect(const SignalLink& l, bool wait = true);
    bool disconnectTrackLink(const SignalLink& l);

  public:
    SignalBase::OnSubscribers      onSubscribers;
    SignalSubscriberMap            subscriberMap;
    TrackMap                       trackMap;
    qi::Atomic<int>                trackId;
    qi::Signature                  signature;
    boost::recursive_mutex         mutex;
    MetaCallType                   defaultCallType;
    SignalBase::Trigger            triggerOverride;
  };

}

#endif  // _SRC_SIGNAL_P_HPP_
