#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_SIGNAL_HXX_
#define _QITYPE_DETAILS_SIGNAL_HXX_

#include <boost/fusion/functional/generation/make_unfused.hpp>
#include <boost/fusion/functional/invocation/invoke_procedure.hpp>

namespace qi
{
  namespace detail
  {

    template<typename T>
    class BoostWeakPointerLock: public WeakLock
    {
    public:
      BoostWeakPointerLock(typename boost::weak_ptr<T> ptr)
      : weakPointer(ptr)
      {
      }

      bool tryLock()
      {
        sharedPointer = weakPointer.lock();
        return sharedPointer;
      }

      void unlock()
      {
        sharedPointer.reset();
      }

      WeakLock* clone()
      {
        return new BoostWeakPointerLock(weakPointer);
      }
    private:
      boost::shared_ptr<T> sharedPointer;
      boost::weak_ptr<T>   weakPointer;
    };

    // Subscriber from unknown pointer type and member function
    template<typename O, typename MF>
    class SignalSubscriberHelper
    {
    public:
      static void set(SignalSubscriber& sub, O ptr, MF function)
      {
        sub.handler = makeGenericFunction(ptr, function);
        sub.weakLock = 0;
      }
    };

    // Subscriber from shared pointer and member function
    template<typename O, typename MF>
    class SignalSubscriberHelper<boost::shared_ptr<O>, MF>
    {
    public:
      static void set(SignalSubscriber& sub, boost::shared_ptr<O> ptr, MF function)
      {
        // bind the pointer, not the shared ptr
        sub.handler = makeGenericFunction(ptr.get(), function);
        // Register a locker on the weak pointer
        sub.weakLock = new BoostWeakPointerLock<O>(ptr);
      }
    };
  }

  template<typename T>
 template<typename O, typename MF>
 inline SignalBase::Link Signal<T>::connect(O* target, MF method)
 {
   return SignalBase::connect(SignalSubscriber(target, method));
 }

  template<typename O, typename MF>
  SignalSubscriber::SignalSubscriber(O* ptr, MF function, EventLoop* ctx)
  {
    enabled = true;
    active = 0;
    eventLoop = ctx;
    detail::SignalSubscriberHelper<O, MF>::set(*this, ptr, function);
  }
  template<typename O, typename MF>
  SignalSubscriber::SignalSubscriber(boost::shared_ptr<O> ptr, MF function, EventLoop* ctx)
  {
    enabled = true;
    active = 0;
    eventLoop = ctx;
    detail::SignalSubscriberHelper<O, MF>::set(*this, ptr, function);
  }
  namespace detail
  {
    struct Appender
  {
    inline Appender(std::vector<GenericValue>*target)
    :target(target)
    {
    }
    template<typename T>
    void
    operator() (const T &v) const
    {
      target->push_back(AutoGenericValue(v));
    }

    std::vector<GenericValue>* target;
  };
  template<typename T>
  struct FusedEmit
  {
    typedef typename boost::function_types::result_type<T>::type RetType;
    typedef typename boost::function_types::parameter_types<T>::type ArgsType;
    typedef typename boost::mpl::push_front<ArgsType, SignalBase*>::type InstArgsType;
    typedef typename boost::mpl::push_front<InstArgsType, RetType>::type FullType;
    typedef typename boost::function_types::function_type<FullType>::type LinearizedType;
    FusedEmit(Signal<T>& signal)
    : _signal(signal) {}

    template <class Seq>
    struct result
    {
      typedef typename boost::function_types::result_type<T>::type type;
    };
    template <class Seq>
    typename result<Seq>::type
    operator()(Seq const & s) const
    {
      std::vector<GenericValue> args;
      boost::fusion::for_each(s, Appender(&args));
      _signal.trigger(args);
    }
    SignalBase& _signal;
  };
  } // detail

  template<typename T>
  Signal<T>::Signal()
  : SignalBase()
  {
    detail::FusedEmit<T> fusor = detail::FusedEmit<T>(*this);
    * (boost::function<T>*)this = boost::fusion::make_unfused(fusor);
  }

  template<typename T>
  Signal<T>& Signal<T>::operator = (const Signal<T>& b)
  { // Keep our boost::function as is.
    *(SignalBase*)this = b;
    return *this;
  }

  template<typename T>
  Signal<T>::Signal(const Signal<T>& b)
  {
    detail::FusedEmit<T> fusor = detail::FusedEmit<T>(*this);
    * (boost::function<T>*)this = boost::fusion::make_unfused(fusor);
  }

  template<typename T>
  std::string Signal<T>::signature() const
  {
    return detail::functionArgumentsSignature<T>();
  }
} // qi
#endif  // _QITYPE_DETAILS_SIGNAL_HXX_
