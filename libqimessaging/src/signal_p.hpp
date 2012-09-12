/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  QI_SIGNAL_P_HPP_
# define QI_SIGNAL_P_HPP_

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

#endif /* !QI_SIGNAL_P_PP_ */
