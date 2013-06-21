#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_PROXYPROPERTY_HPP_
#define _QITYPE_PROXYPROPERTY_HPP_

#include <qi/log.hpp>
#include <qitype/property.hpp>
#include <qitype/anyfunction.hpp>

namespace qi
{
  /** Property proxy using AnyObject as backend
  * @warning reading and writing the property are synchronous operations
  * and might take time if backend object is a remote proxy.
  */
  template<typename T>
  class ProxyProperty: public Property<T>
  {
  public:
    typedef SignalF<void (const T&)> SignalType;
    /* The signal bounce code is completely duplicated from SignalProxy.
     * Unfortunately factoring this is not trivial:
     * onSubscribe needs to be passed to Signal constructor, and we want to keep
     * it that way.
    */
    ProxyProperty(AnyObject object, const std::string& propertyName)
    : Property<T>(
      boost::bind(&ProxyProperty<T>::_getter, this),
      boost::bind(&ProxyProperty<T>::_setter, this, _1, _2),
      boost::bind(&ProxyProperty<T>::onSubscribe, this, _1))
    , _name(propertyName)
    , _object(object.get())
    , _link(SignalBase::invalidSignalLink)
    {
    }
    ~ProxyProperty();
    void onSubscribe(bool enable);
    AnyReference bounceEvent(const std::vector<AnyReference> args);
    virtual void trigger(const GenericFunctionParameters& params, MetaCallType);
  private:
    T _getter();
    bool _setter(T&, const T&);
    std::string _name;
    GenericObject* _object;
    SignalLink _link;
  };

  template<typename T>
  ProxyProperty<T>::~ProxyProperty()
  {
    SignalType::disconnectAll();
    if (_link != SignalBase::invalidSignalLink)
      onSubscribe(false);
  }

  template<typename T>
  void ProxyProperty<T>::onSubscribe(bool enable)
  {
    if (enable)
    {
      _link = _object->connect(_name,
          SignalSubscriber(
            AnyFunction::fromDynamicFunction(boost::bind(&ProxyProperty<T>::bounceEvent, this, _1))
            ));
    }
    else
    {
      bool ok = !_object->disconnect(_link).hasError();
      if (!ok)
        qiLogError("qitype.proxysignal") << "Failed to disconnect from parent signal";
      _link = SignalBase::invalidSignalLink;
    }
  }

  template<typename T>
  AnyReference ProxyProperty<T>::bounceEvent(const std::vector<AnyReference> args)
  {
    // Receive notify from backend, trigger on our signal, bypassing our trigger overload
    SignalType::trigger(args);
    return AnyReference(typeOf<void>());
  }

  template<typename T>
  void ProxyProperty<T>::trigger(const GenericFunctionParameters& params, MetaCallType)
  {
    // Just forward to backend, which will notify us in bouceEvent(),
    // and then we will notify our local Subscribers
    _object->metaPost(_name, params);
  }
  template<typename T>
  T ProxyProperty<T>::_getter()
  {
    return _object->property<T>(_name);
  }
  template<typename T>
  bool ProxyProperty<T>::_setter(T& target, const T& v)
  {
    // no need to fill target it's never used since we have a getter
    _object->setProperty(_name, v).value(); // throw on remote error
    // Prevent local subscribers from being called
    return false;
  }
}
#endif  // _QITYPE_PROXYPROPERTY_HPP_
