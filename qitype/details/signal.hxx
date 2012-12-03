#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_SIGNAL_HXX_
#define _QITYPE_DETAILS_SIGNAL_HXX_

#include <qitype/manageable.hpp>

#include <boost/bind.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
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
      : lockCount(0)
      , weakPointer(ptr)
      {
      }

      bool tryLock()
      {
        ++lockCount;
        sharedPointer = weakPointer.lock();
        return sharedPointer;
      }

      void unlock()
      {
        if (!--lockCount)
          sharedPointer.reset();
      }

      WeakLock* clone()
      {
        return new BoostWeakPointerLock(weakPointer);
      }
    private:
      qi::Atomic<int>  lockCount;
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
  inline SignalSubscriber& Signal<T>::connect(O* target, MF method, MetaCallType threadingModel)
  {
    return SignalBase::connect(SignalSubscriber(target, method, threadingModel));
  }

  template<typename T>
  template<typename O, typename MF>
  inline SignalSubscriber& Signal<T>::connect(boost::shared_ptr<O> target, MF method, MetaCallType threadingModel)
  {
    return SignalBase::connect(SignalSubscriber(target, method, threadingModel));
  }

  template<typename O, typename MF>
  SignalSubscriber::SignalSubscriber(O* ptr, MF function, MetaCallType model)
  {
    enabled = true;
    target = 0;
    threadingModel = model;
    detail::SignalSubscriberHelper<O*, MF>::set(*this, ptr, function);
  }
  template<typename O, typename MF>
  SignalSubscriber::SignalSubscriber(boost::shared_ptr<O> ptr, MF function, MetaCallType model)
  {
    enabled = true;
    target = 0;
    threadingModel = model;
    detail::SignalSubscriberHelper<boost::shared_ptr<O>, MF>::set(*this, ptr, function);
  }
  namespace detail
  {
    struct Appender
  {
    inline Appender(std::vector<GenericValuePtr>*target)
    :target(target)
    {
    }
    template<typename T>
    void
    operator() (const T &v) const
    {
      target->push_back(AutoGenericValuePtr(v));
    }

    std::vector<GenericValuePtr>* target;
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
      std::vector<GenericValuePtr> args;
      boost::fusion::for_each(s, Appender(&args));
      _signal.trigger(args);
    }
    SignalBase& _signal;
  };
  } // detail

  template<typename T>
  Signal<T>::Signal(OnSubscribers onSubscribers)
  : SignalBase(onSubscribers)
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

  template<typename T>
  SignalSubscriber& SignalSubscriber::track(boost::weak_ptr<T> ptr)
  {
    if (weakLock)
      throw std::runtime_error("Only one weak lock supported");
    weakLock = new detail::BoostWeakPointerLock<T>(ptr);
    return *this;
  }
} // qi
#endif  // _QITYPE_DETAILS_SIGNAL_HXX_
