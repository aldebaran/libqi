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
    SignalBase(const std::string& signature);
    ~SignalBase();

    typedef unsigned int Link;

    template<typename FUNCTION_TYPE> Link connect(FUNCTION_TYPE f,
      EventLoop* ctx = getDefaultObjectEventLoop());

    template<typename OBJECT_TYPE, typename FUNCTION_TYPE>
    Link connect(OBJECT_TYPE c, FUNCTION_TYPE f, EventLoop* ctx = getDefaultObjectEventLoop());

    Link connect(qi::Object target, unsigned int slot);
    Link connect(MetaFunction callback, EventLoop* ctx = getDefaultObjectEventLoop());
    Link connect(const SignalSubscriber& s);
    bool disconnect(const Link& link);

    void trigger(const MetaFunctionParameters& params);
    void operator()(
      qi::AutoMetaValue p1 = qi::AutoMetaValue(),
      qi::AutoMetaValue p2 = qi::AutoMetaValue(),
      qi::AutoMetaValue p3 = qi::AutoMetaValue(),
      qi::AutoMetaValue p4 = qi::AutoMetaValue(),
      qi::AutoMetaValue p5 = qi::AutoMetaValue(),
      qi::AutoMetaValue p6 = qi::AutoMetaValue(),
      qi::AutoMetaValue p7 = qi::AutoMetaValue(),
      qi::AutoMetaValue p8 = qi::AutoMetaValue());

    std::vector<SignalSubscriber> subscribers();

  public:
    boost::shared_ptr<SignalBasePrivate> _p;
  };

  template<typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(FUNCTION_TYPE  callback, EventLoop* ctx)
  {
    return connect(makeFunctor(callback), ctx);
  }

  template<typename OBJECT_TYPE, typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(OBJECT_TYPE inst, FUNCTION_TYPE fun, EventLoop* ctx)
  {
    return connect(makeFunctor(inst, fun), ctx);
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

QI_NO_METATYPE(qi::SignalBase)

#endif
