#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_PROXYSIGNAL_HPP_
#define _QI_TYPE_PROXYSIGNAL_HPP_

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <qi/signal.hpp>
#include <qi/anyfunction.hpp>
#include <qi/anyobject.hpp>

namespace qi
{
  namespace details_proxysignal
  {
    constexpr inline const char* objectExpiredError()
    {
      return "The object that the proxy signal represents has expired.";
    }

    inline void metaPostSignal(AnyWeakObject object,
                               const std::string& signalName,
                               const GenericFunctionParameters& params)
    {
      if (auto sharedObject = object.lock())
        sharedObject.metaPost(signalName, params);
    }

    /// Resets the bounce event callback on the signal, depending on the `enable` boolean and the
    /// existence of local subscribers.
    ///
    /// If enabled, the `callSubs` callback is called when the remote signal is triggered. If
    /// disabled, the callback is disconnected instead.
    ///
    /// Procedure<void(GenericFunctionParameters)> CallSubs
    template<typename CallSubs>
    Future<SignalLink> resetBounceEventCallback(CallSubs callSubs,
                                                bool enable,
                                                AnyWeakObject weakObject,
                                                std::string signalName,
                                                SignalLink link)
    {
      using namespace ka::functional_ops;

      auto object = weakObject.lock();
      if (!object)
        return makeFutureError<SignalLink>(objectExpiredError());

      Future<SignalLink> newLinkFut;
      if (enable)
      {
        // Bounce the event back to the local subscribers by calling `_callSubscribers` on the
        // signal instead of triggering it, which with our trigger override would re-post the event
        // back to the back-end and would lead to an infinite loop.
        SignalSubscriber bounceEvent(AnyFunction::fromDynamicFunction(
          ka::mv(callSubs) | ka::constant_function(AnyReference(typeOf<void>()))));
        return object.connect(signalName, std::move(bounceEvent)).async();
      }
      else
      {
        return object.disconnect(link).async()
          .then([](Future<void> discFut) {
            if (!discFut.hasValue())
            {
              qiLogVerbose("qitype.proxysignal")
                << "Failed to disconnect from parent signal: "
                << (discFut.hasError() ? discFut.error() : "promise was cancelled");
            }
            return SignalBase::invalidSignalLink;
          });
      }
    }

    /// If non-empty, returns the value of the optional given to it. Otherwise calls some procedure.
    ///
    /// If the procedure is not non-returning, then the behavior is undefined.
    ///
    /// Procedure<void()> Proc
    template<typename Proc>
    struct SrcOptOrInvoke
    {
      Proc proc;
    // Regular:
      KA_GENERATE_FRIEND_REGULAR_OPS_1(SrcOptOrInvoke, proc)

    // ka::opt_t<T> -> T:
      template<typename T>
      T operator()(const ka::opt_t<T>& o) const
      {
        if (o.empty())
          proc();
        return *o;
      }
    };

    /// Helper-function to deduce types for `SrcOptOrInvoke`.
    template<typename Proc>
    SrcOptOrInvoke<Proc> srcOptOrInvoke(Proc proc) { return { ka::mv(proc) }; }

    /// Resets the bounce event callback on an object, and then sets the `OnSubscribers` callback
    /// to a copy of itself with the new signal link, continuously. Once called, an instance of
    /// this type is not meant to be called again.
    ///
    /// The signal pointer must be valid for the duration of this function call.
    ///
    /// Transformation<Procedure> LifeSignal
    /// Procedure<void(GenericFunctionParameters)> CallSubs
    template<typename LifeSignal, typename CallSubs>
    struct ResetBounceEventCallbackOnSubscribersContinuous
    {
      SignalBase* signal;
      LifeSignal lifeSignal;
      CallSubs callSubs;
      AnyWeakObject weakObject;
      std::string signalName;

      using result_type = Future<void>;

    // Procedure:
      result_type operator()(SignalLink link, bool enable) const
      {
        auto self = *this;
        return resetBounceEventCallback(callSubs, enable, weakObject, signalName, link)
           // link changed, rebind ourselves if we're still alive
          .andThen(lifeSignal(
            // Copy self instead of using `this` to avoid lifetime issues.
            [self](SignalLink newLink) {
              self.signal->setOnSubscribers(self.lifeSignal(boost::bind(self, newLink, std::placeholders::_1)));
            }));
      }
    };

    /// Helper-function to deduce types for `ResetBounceEventCallbackOnSubscribersContinuous`.
    template <typename LifeSignal, typename CallSubs>
    ResetBounceEventCallbackOnSubscribersContinuous<LifeSignal, CallSubs>
    resetBounceEventCallbackOnSubscribersContinuous(SignalBase& signal,
                                                    LifeSignal lifeSignal,
                                                    CallSubs callSubs,
                                                    AnyWeakObject weakObject,
                                                    std::string signalName)
    {
      return { &signal, ka::mv(lifeSignal), ka::mv(callSubs), ka::mv(weakObject),
               ka::mv(signalName) };
    }

    inline void setUpProxy(SignalBase& signal, AnyWeakObject object, const std::string& signalName)
    {
      using namespace ka;
      namespace ph = std::placeholders;

      // lockTransfo: (A... → B) → (A... → ka::opt_t<B>)
      // p ↦ (x... ↦ | ka::opt(p(x...))  if the weak_ptr could be locked,
      //             | ka::opt_t<B>()    otherwise, i.e. an empty ka::opt_t)
      auto lockTransfo = scope_lock_transfo(mutable_store(weak_ptr(signal._p)));

      // srcOrThrow: (A... → ka::opt_t<B>) → (A... → B)
      // p ↦ (x... ↦ | *p(x...)  if !p(x...).empty(),
      //             | throw     otherwise)
      auto srcOrThrow =
        std::bind(compose_t{},
                  srcOptOrInvoke([] { throw std::runtime_error("the signal has expired"); }),
                  ph::_1);

      // lifeSignal: (A... → B) → (A... → B)
      // Transforms a procedure p to one that first checks if the signal is
      // still alive, then calls p if alive, and throws otherwise.
      auto lifeSignal = compose(mv(srcOrThrow), mv(lockTransfo));

      auto callSubs = lifeSignal(std::bind(&SignalBase::callSubscribers, std::ref(signal),
                                           ph::_1, MetaCallType_Auto));

      signal.setOnSubscribers(lifeSignal(
        boost::bind(resetBounceEventCallbackOnSubscribersContinuous(signal, lifeSignal, callSubs,
                                                                    object, signalName),
                    SignalBase::invalidSignalLink, ph::_1)));

      // On signal trigger, just forward the trigger to the back-end. When the back-end gets
      // triggered, we get notified back, because we connect to the back-end by the 'bounce event'
      // callback, in which we can notify back our local subscribers.
      signal.setTriggerOverride(
        boost::bind(&details_proxysignal::metaPostSignal, object, signalName, ph::_1));
    }

    inline void tearDownProxy(SignalBase& sig)
    {
      sig.disconnectAll();
    }
  } // namespace details_proxysignal

  /// Signal proxy, using an AnyObject and signal id as back-end.
  template<typename T>
  class ProxySignal: public SignalF<T>
  {
  public:
    using SignalType = SignalF<T>;

    ProxySignal()
    {
    }

    ProxySignal(AnyObject object, const std::string& signalName)
      : SignalType()
    {
      details_proxysignal::setUpProxy(*this, object, signalName);
    }

    ~ProxySignal()
    {
      details_proxysignal::tearDownProxy(*this);
    }

    QI_API_DEPRECATED_MSG(Use qi::makeProxySignal instead)
    void setup(AnyObject object, const std::string& signalName)
    {
      details_proxysignal::setUpProxy(*this, object, signalName);
    }

    QI_API_DEPRECATED_MSG(This function might be removed in the future with no replacement)
    Future<void> onSubscribe(bool enable,
                             GenericObject* object,
                             std::string signalName,
                             SignalLink link)
    {
      return details_proxysignal::resetBounceEventCallback(*this, this->_p, enable,
                                                           AnyObject(object, &AnyObject::noDelete),
                                                           signalName, link);
    }

    QI_API_DEPRECATED_MSG(This function might be removed in the future with no replacement)
    AnyReference bounceEvent(const AnyReferenceVector args)
    {
      this->callSubscribers(args);
      return AnyReference(typeOf<void>());
    }

    QI_API_DEPRECATED_MSG(This function might be removed in the future with no replacement)
    void triggerOverride(const GenericFunctionParameters& params,
                         MetaCallType,
                         GenericObject* object,
                         std::string signalName)
    {
      return details_proxysignal::metaPostSignal(AnyObject(object, &AnyObject::noDelete),
                                                 signalName, params);
    }
  };

  inline void makeProxySignal(SignalBase& target, AnyObject object, const std::string& signalName)
  {
    details_proxysignal::setUpProxy(target, object, signalName);
  }

  template<typename T>
  void makeProxySignal(ProxySignal<T>& target, AnyObject object, const std::string& signalName)
  {
    details_proxysignal::setUpProxy(target, object, signalName);
  }
} // namespace qi

#endif  // _QITYPE_PROXYSIGNAL_HPP_
