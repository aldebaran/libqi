#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_SIGNAL_HXX_
#define _QITYPE_DETAILS_SIGNAL_HXX_

#include <qitype/manageable.hpp>
#include <boost/bind.hpp>
#include <qitype/details/functionsignature.hxx>

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
        sub.handler = AnyFunction::from(function, ptr);
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
        sub.handler = AnyFunction::from(function, ptr.get());
        // Register a locker on the weak pointer
        sub.weakLock = new BoostWeakPointerLock<O>(ptr);

      }
    };
  }

  template<typename T>
  template<typename O, typename MF>
  inline SignalSubscriber& SignalF<T>::connect(O* target, MF method, MetaCallType threadingModel)
  {
    return SignalBase::connect(SignalSubscriber(target, method, threadingModel));
  }

  template<typename T>
  template<typename O, typename MF>
  inline SignalSubscriber& SignalF<T>::connect(boost::shared_ptr<O> target, MF method, MetaCallType threadingModel)
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

  template<typename T> class BounceToSignalBase
  {
    // This default should not be instanciated
    BOOST_STATIC_ASSERT(sizeof(T) < 0);
    public:
    BounceToSignalBase(SignalBase& sb)
    {
    }
  };
  #define pushArg(z, n, _) \
    args.push_back(AutoAnyReference(p ##n));
  #define makeBounce(n, argstypedecl, argstype, argsdecl, argsues, comma)     \
  template<typename R comma argstypedecl> \
  class BounceToSignalBase<R(argstype)>  {  \
  public:                      \
    BounceToSignalBase(SignalBase& signalBase) : signalBase(signalBase) {} \
    R operator()(argsdecl) {   \
      std::vector<AnyReference> args; \
      BOOST_PP_REPEAT(n, pushArg, _);    \
      signalBase.trigger(args);          \
    }                                    \
  private:                               \
    SignalBase& signalBase;              \
  };
  QI_GEN(makeBounce)
  #undef makeBounce
  #undef pushArg

  } // detail

  template<typename T>
  SignalF<T>::SignalF(OnSubscribers onSubscribers)
  : SignalBase(onSubscribers)
  {
    * (boost::function<T>*)this = detail::BounceToSignalBase<T>(*this);
    _setSignature(detail::functionArgumentsSignature<T>());
  }

  template<typename T>
  SignalF<T>& SignalF<T>::operator = (const SignalF<T>& b)
  { // Keep our boost::function as is.
    *(SignalBase*)this = b;
    return *this;
  }

  template<typename T>
  SignalF<T>::SignalF(const SignalF<T>& b)
  {
    * (boost::function<T>*)this = detail::BounceToSignalBase<T>(*this);
  }

  template<typename T>
  qi::Signature SignalF<T>::signature() const
  {
    return detail::functionArgumentsSignature<T>();
  }

  template<typename T>
  template<typename U>
  SignalSubscriber& SignalF<T>::connect(SignalF<U>& signal)
  {
    return connect((boost::function<U>&)signal);
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
