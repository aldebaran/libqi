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

  using SignalSubscriberMap = std::map<SignalLink, SignalSubscriber>;
  using TrackMap = std::map<int, SignalLink>;

  class SignalBasePrivate
  {
  public:
    SignalBasePrivate()
      : execContext(nullptr)
      , defaultCallType(MetaCallType_Auto)
    {}

    ~SignalBasePrivate();
    Future<bool> disconnect(const SignalLink& l);
    Future<bool> disconnectAll();
    void disconnectTrackLink(int id);

  private:
    friend class SignalBase;
    Future<bool> disconnectAllStep(bool overallSuccess);

    SignalBase::OnSubscribers      onSubscribers;
    ExecutionContext*              execContext;
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
