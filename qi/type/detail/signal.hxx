#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_SIGNAL_HXX_
#define _QITYPE_DETAIL_SIGNAL_HXX_

#include <qi/trackable.hpp>
#include <qi/type/detail/manageable.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <qi/type/detail/functionsignature.hxx>

namespace qi
{
  template <typename T>
  template <typename F, typename Arg0, typename... Args>
  SignalSubscriber SignalF<T>::connect(F&& func, Arg0&& arg0, Args&&... args)
  {
    SignalSubscriber s =
      connect(qi::bind(std::forward<F>(func), std::forward<Arg0>(arg0), std::forward<Args>(args)...));
    return s;
  }

  template<typename T>
  SignalSubscriber SignalF<T>::connect(AnyFunction f)
  {
    auto execContext = executionContext();
    if (execContext)
    {
      return SignalBase::connect(SignalSubscriber(std::move(f), execContext));
    }
    else
    {
      return SignalBase::connect(SignalSubscriber(std::move(f), MetaCallType_Auto));
    }
  }
  template<typename T>
  SignalSubscriber SignalF<T>::connect(const SignalSubscriber& sub)
  {
    return SignalBase::connect(std::move(sub));
  }

  template<typename T>
  template<class ForcedSignalType, class SignalType>
  SignalSubscriber SignalF<T>::connectSignal(SignalType& signal)
  {
    int curId = 0;
    SignalLink* trackLink = nullptr;
    createNewTrackLink(curId, trackLink);

    boost::weak_ptr<SignalBasePrivate> maybeThisSignalPrivate(this->_p);

    auto onSignalLost = [=]{
      if (auto thisSignalPrivate = maybeThisSignalPrivate.lock())
      {
        disconnectTrackLink(*thisSignalPrivate, curId);
      }
    };

    auto forwardSignalCall = qi::trackWithFallback(
      std::move(onSignalLost),
      static_cast<ForcedSignalType&>(signal),
      boost::weak_ptr<SignalBasePrivate>(signal._p));

    SignalSubscriber s = connect(std::move(forwardSignalCall));

    *trackLink = s;
    return s;
  }

  template<typename T>
  template<typename U>
  SignalSubscriber  SignalF<T>::connect(SignalF<U>& signal)
  {
    return connectSignal<boost::function<U>>(signal);
  }

  template <typename T>
  template <typename... P>
  SignalSubscriber  SignalF<T>::connect(Signal<P...>& signal)
  {
    typedef void(ftype)(P...);
    return connectSignal<boost::function<ftype>>(signal);
  }

  template<typename F>
  SignalSubscriber SignalBase::connect(boost::function<F> fun)
  {
    return connect(AnyFunction::from(std::move(fun)));
  }
  // TODO: taking by forward ref is too greedy and connect(SignalSubscriber) takes this overload
  // find a way to fix this
  template<typename T>
  template<typename F>
  SignalSubscriber SignalF<T>::connect(F c)
  {
    SignalSubscriber sub = connect(qi::AnyFunction::from(boost::function<T>(std::move(c))));
    if (detail::IsAsyncBind<F>::value)
      sub.setCallType(MetaCallType_Direct);
    return sub;
  }
  template<typename T>
  SignalSubscriber SignalF<T>::connect(const AnyObject& obj, const std::string& slot)
  {
    return SignalBase::connect(obj, slot);
  }

  template<typename T>
  SignalSubscriber SignalF<T>::connect(const AnyObject& obj, unsigned int slot)
  {
    return connect(SignalSubscriber(obj, slot));
  }

  namespace detail
  {

  template<typename T> class BounceToSignalBase;
  #define pushArg(z, n, _) \
    args.push_back(AutoAnyReference(p ##n));
  #define makeBounce(n, argstypedecl, argstype, argsdecl, argsues, comma)     \
  template<typename R comma argstypedecl> \
  class BounceToSignalBase<R(argstype)>  {  \
  public:                      \
    BounceToSignalBase(SignalBase& signalBase) : signalBase(signalBase) {} \
    R operator()(argsdecl) {   \
      AnyReferenceVector args; \
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
    : SignalF(nullptr, std::move(onSubscribers))
  {
  }

  template<typename T>
  SignalF<T>::SignalF(ExecutionContext* execContext, OnSubscribers onSubscribers)
    : SignalBase(execContext, onSubscribers)
  {
    * (boost::function<T>*)this = detail::BounceToSignalBase<T>(*this);
    _setSignature(detail::functionArgumentsSignature<T>());
  }


  template<typename T>
  qi::Signature SignalF<T>::signature() const
  {
    return detail::functionArgumentsSignature<T>();
  }
} // qi
#endif  // _QITYPE_DETAIL_SIGNAL_HXX_
