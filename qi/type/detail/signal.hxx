#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_SIGNAL_HXX_
#define _QITYPE_DETAIL_SIGNAL_HXX_

#include <qi/trackable.hpp>
#include <qi/type/detail/manageable.hpp>
#include <boost/bind.hpp>
#include <qi/type/detail/functionsignature.hxx>

namespace qi
{
  template <typename T>
  template <typename F, typename Arg0, typename... Args>
  SignalSubscriber& SignalF<T>::connect(F&& func, Arg0&& arg0, Args&&... args)
  {
    int curId;
    SignalLink* trackLink;
    createNewTrackLink(curId, trackLink);
    SignalSubscriber& s =
      connect(qi::bind(std::forward<F>(func), std::forward<Arg0>(arg0), std::forward<Args>(args)...));
    *trackLink = s;
    return s;
  }

  template<typename T>
  SignalSubscriber& SignalF<T>::connect(AnyFunction f)
  {
    return SignalBase::connect(SignalSubscriber(std::move(f)));
  }
  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const SignalSubscriber& sub)
  {
    return SignalBase::connect(std::move(sub));
  }
  template<typename T>
  template<typename U>
  SignalSubscriber&  SignalF<T>::connect(SignalF<U>& signal)
  {
    int curId;
    SignalLink* trackLink;
    createNewTrackLink(curId, trackLink);
    SignalSubscriber& s = connect(qi::trackWithFallback(
          boost::bind(&SignalF<T>::disconnectTrackLink, this, curId),
          (boost::function<U>&)signal,
          boost::weak_ptr<SignalBasePrivate>(signal._p)));
    *trackLink = s;
    return s;
  }

  template <typename T>
  template <typename... P>
  SignalSubscriber&  SignalF<T>::connect(Signal<P...>& signal)
  {
    typedef void(ftype)(P...);
    int curId;
    SignalLink* trackLink;
    createNewTrackLink(curId, trackLink);
    SignalSubscriber& s = connect(qi::trackWithFallback(
          boost::bind(&SignalF<T>::disconnectTrackLink, this, curId),
          (boost::function<ftype>&)signal,
          boost::weak_ptr<SignalBasePrivate>(signal._p)));
    *trackLink = s;
    return s;
  }

  template<typename F>
  SignalSubscriber& SignalBase::connect(boost::function<F> fun)
  {
    return connect(AnyFunction::from(std::move(fun)));
  }
  // TODO: taking by forward ref is too greedy and connect(SignalSubscriber) takes this overload
  // find a way to fix this
  template<typename T>
  template<typename F>
  SignalSubscriber& SignalF<T>::connect(F c)
  {
    SignalSubscriber& sub = connect(qi::AnyFunction::from(boost::function<T>(std::move(c))));
    if (detail::IsAsyncBind<F>::value)
      sub.setCallType(MetaCallType_Direct);
    return sub;
  }
  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const AnyObject& obj, const std::string& slot)
  {
    return SignalBase::connect(obj, slot);
  }

  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const AnyObject& obj, unsigned int slot)
  {
    return connect(SignalSubscriber(obj, slot));
  }

  namespace detail
  {

  template<typename T> class BounceToSignalBase
  {
    // This default should not be instanciated
    static_assert(sizeof(T) < 0, "You can't instanciate BounceToSignalBase");
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
  : SignalBase(onSubscribers)
  {
    * (boost::function<T>*)this = detail::BounceToSignalBase<T>(*this);
    _setSignature(detail::functionArgumentsSignature<T>());
  }


  template<typename T>
  qi::Signature SignalF<T>::signature() const
  {
    return detail::functionArgumentsSignature<T>();
  }

  inline
  SignalSubscriber& SignalSubscriber::setCallType(MetaCallType ct)
  {
    threadingModel = ct;
    return *this;
  }
} // qi
#endif  // _QITYPE_DETAIL_SIGNAL_HXX_
