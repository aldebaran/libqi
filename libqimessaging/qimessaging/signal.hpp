/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_SIGNAL_HPP_
#define _QIMESSAGING_SIGNAL_HPP_

#include <qimessaging/metafunction.hpp>
#include <qimessaging/event_loop.hpp>
#include <qimessaging/signature.hpp>

namespace qi {

  class Object;
  class SignalBasePrivate;
  struct SignalSubscriber;

  class QIMESSAGING_API SignalBase
  {
  public:
    explicit SignalBase(const std::string& signature);
    ~SignalBase();

    typedef unsigned int Link;

    template<typename FUNCTION_TYPE> Link connect(FUNCTION_TYPE f,
      EventLoop* ctx = getDefaultObjectEventLoop());

    template<typename OBJECT_TYPE, typename FUNCTION_TYPE>
    Link connect(OBJECT_TYPE c, FUNCTION_TYPE f, EventLoop* ctx = getDefaultObjectEventLoop());

    Link connect(qi::Object target, unsigned int slot);
    Link connect(FunctionValue callback, EventLoop* ctx = getDefaultObjectEventLoop());
    Link connect(MetaCallable callback, EventLoop* ctx = getDefaultObjectEventLoop());
    Link connect(const SignalSubscriber& s);
    bool disconnect(const Link& link);

    void trigger(const MetaFunctionParameters& params);
    void operator()(
      qi::AutoValue p1 = qi::AutoValue(),
      qi::AutoValue p2 = qi::AutoValue(),
      qi::AutoValue p3 = qi::AutoValue(),
      qi::AutoValue p4 = qi::AutoValue(),
      qi::AutoValue p5 = qi::AutoValue(),
      qi::AutoValue p6 = qi::AutoValue(),
      qi::AutoValue p7 = qi::AutoValue(),
      qi::AutoValue p8 = qi::AutoValue());

    std::vector<SignalSubscriber> subscribers();

  public:
    boost::shared_ptr<SignalBasePrivate> _p;
  };

  template<typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(FUNCTION_TYPE  callback, EventLoop* ctx)
  {
    return connect(makeFunctionValue(callback), ctx);
  }

  template<typename OBJECT_TYPE, typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(OBJECT_TYPE inst, FUNCTION_TYPE fun, EventLoop* ctx)
  {
    return connect(makeFunctionValue(inst, fun), ctx);
  }

  template<typename T>
  class Signal: public SignalBase
  {
  public:
    inline Signal()
      : SignalBase(detail::functionArgumentsSignature<T>())
    {
    }

    inline SignalBase::Link connect(boost::function<T> f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }

    template<typename OBJECT_TYPE, typename FUNCTION_TYPE>
    inline SignalBase::Link connect(OBJECT_TYPE inst, FUNCTION_TYPE fun, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(inst, fun, ctx);
    }
  };

}

QI_NO_TYPE(qi::SignalBase)

#endif
