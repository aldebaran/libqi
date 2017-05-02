#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_PROXYSIGNAL_HPP_
#define _QI_TYPE_PROXYSIGNAL_HPP_

#include <boost/bind.hpp>
#include <qi/signal.hpp>
#include <qi/anyfunction.hpp>
#include <qi/anyobject.hpp>

namespace qi
{

  /// Signal proxy, using an AnyObject and signal id as backend.
  template<typename T>
  class ProxySignal: public SignalF<T>
  {
  public:
    using SignalType = SignalF<T>;
    ProxySignal(AnyObject object, const std::string& signalName)
    : SignalType()
    {
      setup(object, signalName);
    }
    ProxySignal() {}
    ~ProxySignal();
    void setup(AnyObject object, const std::string& signalName)
    {
      SignalBase::setOnSubscribers(boost::bind(&ProxySignal<T>::onSubscribe, this, _1,
        object.asGenericObject(), signalName, SignalBase::invalidSignalLink));
      SignalBase::setTriggerOverride(boost::bind(&ProxySignal<T>::triggerOverride, this, _1, _2,
        object.asGenericObject(), signalName));
    }
    Future<void> onSubscribe(bool enable, GenericObject* object, std::string signalName, SignalLink link);
    AnyReference bounceEvent(const AnyReferenceVector args);
    void triggerOverride(const GenericFunctionParameters& params,
      MetaCallType callType, GenericObject* object, std::string signalName);
  };

  template<typename T>
  void makeProxySignal(SignalF<T>& target, AnyObject object, const std::string& signalName)
  {
    ProxySignal<T>& proxy = static_cast<ProxySignal<T> &>(target);
    proxy.setup(object, signalName);
  }

  template<typename T>
  void makeProxySignal(ProxySignal<T>& target, AnyObject object, const std::string& signalName)
  {
    target.setup(object, signalName);
  }

  template<typename T>
  ProxySignal<T>::~ProxySignal()
  {
    SignalType::disconnectAll(); // will invoke onsubscribe
  }

  template<typename T>
  qi::Future<void> ProxySignal<T>::onSubscribe(bool enable, GenericObject* object, std::string signalName,
    SignalLink link)
  {
    Future<SignalLink> connectingOrDisconnecting = [=]
    {
      if (enable)
      {
        return object->connect(
              signalName, SignalSubscriber(
                AnyFunction::fromDynamicFunction(
                  boost::bind(&ProxySignal<T>::bounceEvent, this, _1)))).async();
      }
      else
      {
        return object->disconnect(link).async().then([](Future<void> f)
        {
          bool ok = !f.hasError();
          if (!ok)
            qiLogError("qitype.proxysignal") << "Failed to disconnect from parent signal";
          return SignalBase::invalidSignalLink;
        });
      }
    }();

    boost::weak_ptr<SignalBasePrivate> weakP = this->_p;
    return connectingOrDisconnecting.andThen([=](SignalLink link)
    { // link changed, rebind ourselve if we're still alive
      if (auto p = weakP.lock())
      {
        this->setOnSubscribers([=](bool enable) // TODO: remove `this->` after upgrading from GCC4.8
                                                // Currently necessary because of a bug in g++4.8
        {
          return onSubscribe(enable, object, signalName, link);
        });
      }
    });
  }

  template<typename T>
  AnyReference ProxySignal<T>::bounceEvent(const AnyReferenceVector args)
  {
    // Trigger on our signal, bypassing our trigger overload
    SignalType::callSubscribers(args);
    return AnyReference(typeOf<void>());
  }

  template<typename T>
  void ProxySignal<T>::triggerOverride(const GenericFunctionParameters& params, MetaCallType,
     GenericObject* object, std::string signalName)
  {
    // Just forward to backend, which will notify us in bouceEvent(),
    // and then we will notify our local Subscribers
    object->metaPost(signalName, params);
  }

}



#endif  // _QITYPE_PROXYSIGNAL_HPP_
