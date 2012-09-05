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
#include <qimessaging/metaobjectbuilder.hpp>

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

    Link connect(qi::Object* target, unsigned int slot);
    Link connect(MetaFunction callback,
                 EventLoop* ctx = getDefaultObjectEventLoop());
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

  protected:
    SignalBasePrivate* _p;
  };

   /** Event subscriber info.
   *
   * Only one of handler or target must be set.
   */
  struct QIMESSAGING_API SignalSubscriber
  {
    SignalSubscriber()
    : handler(0), eventLoop(0), target(0), method(0) {}
    SignalSubscriber(MetaFunction func, EventLoop* ctx)
    : handler(func), eventLoop(ctx), target(0), method(0) {}
    SignalSubscriber(Object * target, unsigned int method)
    : handler(0), eventLoop(0), target(target), method(method) {}

    void call(const MetaFunctionParameters& args);
    // Source information
    SignalBase*        source;
    /// Uid that can be passed to Object::disconnect()
    SignalBase::Link  linkId;

    // Target information
    //   Mode 1: Direct functor call
    MetaFunction       handler;
    EventLoop*         eventLoop;
    //  Mode 2: metaCall
    Object*            target;
    unsigned int       method;
  };


  template<typename T>
  class QIMESSAGING_API Signal: public SignalBase
  {
  public:
    Signal();
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

  namespace detail {

    template<typename T> inline
    std::string functionArgumentSignature()
    {
      qi::SignatureStream sigs;

      typedef typename boost::function_types::parameter_types<T>::type ArgsType;
      boost::mpl::for_each<
      boost::mpl::transform_view<ArgsType,
      boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1> > > > > (qi::detail::signature_function_arg_apply(sigs));

      return sigs.str();
    }
  }

  template<typename T>
  inline Signal<T>::Signal()
  : SignalBase(detail::functionArgumentSignature<T>())
  {

  }
}

QI_NO_METATYPE(qi::SignalBase)

#endif
