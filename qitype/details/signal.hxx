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


  template<typename T>
  SignalSubscriber& SignalF<T>::connect(AnyFunction f)
  {
    return SignalBase::connect(SignalSubscriber(f));
  }
  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const SignalSubscriber& sub)
  {
    return SignalBase::connect(sub);
  }
  template<typename T>
  template<typename U>
  SignalSubscriber&  SignalF<T>::connect(SignalF<U>& signal)
  {
    return connect((boost::function<U>&)signal);
  }

  template<typename T>
  template<QI_SIGNAL_TEMPLATE_DECL>
  SignalSubscriber&  SignalF<T>::connect(Signal<QI_SIGNAL_TEMPLATE>& signal)
  {
    typedef typename detail::VoidFunctionType<QI_SIGNAL_TEMPLATE>::type ftype;
    return connect((boost::function<ftype>&)signal);
  }

  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const boost::function<T>& fun)
  {
    return connect(AnyFunction::from(fun));
  }
  template<typename F>
  SignalSubscriber& SignalBase::connect(const boost::function<F>& fun)
  {
    return connect(AnyFunction::from(fun));
  }
  template<typename T>
  template<typename CALLABLE>
  SignalSubscriber&  SignalF<T>::connect(CALLABLE c)
  {
    return connect((boost::function<T>)c);
  }
  template<typename T>
  SignalSubscriber& SignalF<T>::connect(AnyObject obj, const std::string& slot)
  {
    return SignalBase::connect(obj, slot);
  }

  template<typename T>
  SignalSubscriber& SignalF<T>::connect(AnyObject obj, unsigned int slot)
  {
    return connect(SignalSubscriber(obj, slot));
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

  inline
  SignalSubscriber& SignalSubscriber::setCallType(MetaCallType ct)
  {
    threadingModel = ct;
    return *this;
  }
} // qi
#endif  // _QITYPE_DETAILS_SIGNAL_HXX_
