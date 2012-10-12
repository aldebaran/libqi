#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SIGNAL_P_HPP_
#define _SRC_SIGNAL_P_HPP_

#include <qimessaging/signal.hpp>
namespace qi {

  typedef std::map<SignalBase::Link, SignalSubscriber> SignalSubscriberMap;

  class SignalBasePrivate
  {
  public:
    bool disconnect(const SignalBase::Link& l);
    void reset();

  public:
    SignalSubscriberMap        subscriberMap;
    std::string                signature;
    boost::recursive_mutex     mutex;
  };

}

#endif  // _SRC_SIGNAL_P_HPP_
