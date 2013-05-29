#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_PROXYSIGNAL_HPP_
#define _QITYPE_PROXYSIGNAL_HPP_

#include <qitype/signal.hpp>
#include <qitype/functiontype.hpp>

namespace qi
{

  /// Signal proxy, using an ObjectPtr and signal id as backend.
  template<typename T> class ProxySignal: public SignalF<T>
  {
  public:
    typedef SignalF<T> SignalType;
    ProxySignal(ObjectPtr object, const std::string& signalName)
    : SignalType(boost::bind(&ProxySignal<T>::onSubscribe, this, _1))
    , _name(signalName)
    , _object(object.get())
    , _link(SignalBase::invalidLink)
    {
    }
    ~ProxySignal();
    void onSubscribe(bool enable);
    GenericValuePtr bounceEvent(const std::vector<GenericValuePtr> args);
    virtual void trigger(const GenericFunctionParameters& params, MetaCallType);
  private:
    std::string _name;
    GenericObject* _object;
    SignalBase::Link _link;
  };


  template<typename T>
  ProxySignal<T>::~ProxySignal()
  {
    SignalType::disconnectAll();
    if (_link != SignalBase::invalidLink)
      onSubscribe(false);
  }

  template<typename T>
  void ProxySignal<T>::onSubscribe(bool enable)
  {
    std::string sig = _name + "::" + SignalType::signature().toString();
    if (enable)
    {
      _link = _object->xConnect(sig,
          SignalSubscriber(
            makeDynamicGenericFunction(boost::bind(&ProxySignal<T>::bounceEvent, this, _1))
            ));
    }
    else
    {
      bool ok = !_object->disconnect(_link).hasError();
      if (!ok)
        qiLogError("qitype.proxysignal") << "Failed to disconnect from parent signal";
      _link = SignalBase::invalidLink;
    }
  }

  template<typename T>
  GenericValuePtr ProxySignal<T>::bounceEvent(const std::vector<GenericValuePtr> args)
  {
    // Trigger on our signal, bypassing our trigger overload
    SignalType::trigger(args);
    return GenericValuePtr(typeOf<void>());
  }

  template<typename T>
  void ProxySignal<T>::trigger(const GenericFunctionParameters& params, MetaCallType)
  {
    // Just forward to backend, which will notify us in bouceEvent(),
    // and then we will notify our local Subscribers
    _object->metaPost(_name + "::" + SignalType::signature().toString(), params);
  }

}



#endif
