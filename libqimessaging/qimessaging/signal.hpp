#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SIGNAL_HPP_
#define _QIMESSAGING_SIGNAL_HPP_

#include <qimessaging/eventloop.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/functiontype.hpp>

namespace qi {

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;
  class SignalBasePrivate;
  struct SignalSubscriber;

  class QIMESSAGING_API SignalBase
  {
  public:
    explicit SignalBase(const std::string& signature);
    ~SignalBase();
    std::string signature() const;

    typedef unsigned int Link;

    template<typename FUNCTION_TYPE>
    Link connect(FUNCTION_TYPE f, EventLoop* ctx = getDefaultObjectEventLoop());

    Link connect(qi::ObjectPtr target, unsigned int slot);
    Link connect(GenericFunction callback, EventLoop* ctx = getDefaultObjectEventLoop());
    Link connect(const SignalSubscriber& s);
    bool disconnect(const Link& link);

    void trigger(const GenericFunctionParameters& params);
    void operator()(
      qi::AutoGenericValue p1 = qi::AutoGenericValue(),
      qi::AutoGenericValue p2 = qi::AutoGenericValue(),
      qi::AutoGenericValue p3 = qi::AutoGenericValue(),
      qi::AutoGenericValue p4 = qi::AutoGenericValue(),
      qi::AutoGenericValue p5 = qi::AutoGenericValue(),
      qi::AutoGenericValue p6 = qi::AutoGenericValue(),
      qi::AutoGenericValue p7 = qi::AutoGenericValue(),
      qi::AutoGenericValue p8 = qi::AutoGenericValue());

    std::vector<SignalSubscriber> subscribers();

  public:
    boost::shared_ptr<SignalBasePrivate> _p;
  };

  template<typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(FUNCTION_TYPE  callback, EventLoop* ctx)
  {
    return connect(makeGenericFunction(callback), ctx);
  }

  template<typename T>
  class Signal: public SignalBase, public boost::function<T>
  {
  public:
    Signal();
    using boost::function<T>::operator();
    inline SignalBase::Link connect(boost::function<T> f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }
    inline SignalBase::Link connect(GenericFunction f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }
  };

}

#include <qimessaging/details/signal.hxx>

QI_NO_TYPE(qi::SignalBase)

#endif  // _QIMESSAGING_SIGNAL_HPP_
