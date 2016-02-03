#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_PROPERTY_HXX_
#define _QITYPE_DETAIL_PROPERTY_HXX_

#include <boost/thread/locks.hpp>
#include <qi/future.hpp>

namespace qi
{
  inline FutureSync<void> GenericProperty::set(const AnyValue& v)
  {
    std::pair<AnyReference, bool> conv = v.convert(_type);
    if (!conv.first.type())
      throw std::runtime_error(std::string("Failed converting ") + v.type()->infoString() + " to " + _type->infoString());

    Property<AnyValue>::set(AnyValue(conv.first, false, conv.second));

    return FutureSync<void>(0);
  }

  template<typename T>
  PropertyImpl<T>::PropertyImpl(Getter getter, Setter setter,
    SignalBase::OnSubscribers onsubscribe)
  : SignalF<void(const T&)>(std::move(onsubscribe))
  , _getter(std::move(getter))
  , _setter(std::move(setter))
  {
  }

  template<typename T>
  PropertyImpl<T>::PropertyImpl(AutoAnyReference defaultValue,
    Getter getter, Setter setter,
    SignalBase::OnSubscribers onsubscribe)
  : SignalF<void(const T&)>(std::move(onsubscribe))
  , _getter(std::move(getter))
  , _setter(std::move(setter))
  , _value(defaultValue.to<T>())
  {
  }

  template<typename T>
  T PropertyImpl<T>::getImpl() const
  {
    if (_getter)
      return _getter(_value);
    else
      return _value;
  }
  template<typename T>
  void PropertyImpl<T>::setImpl(const T& v)
  {
    qiLogDebug("qitype.property") << "set " << this << " " << (!!_setter);
    if (_setter)
    {
      const bool ok = _setter(_value, v);
      if (ok)
        (*this)(_value);
    }
    else
    {
      _value = v;
      (*this)(_value);
    }
  }


  template<typename T>
  FutureSync<T> UnsafeProperty<T>::get() const
  {
    return FutureSync<T>(this->getImpl());
  }

  template<typename T>
  FutureSync<void> UnsafeProperty<T>::set(const T& v)
  {
    this->setImpl(v);
    return FutureSync<void>(0);
  }

  template<typename T>
  FutureSync<AnyValue> UnsafeProperty<T>::value() const
  {
    return FutureSync<AnyValue>(AnyValue::from(this->getImpl()));
  }

  template<typename T>
  FutureSync<void> UnsafeProperty<T>::setValue(AutoAnyReference value)
  {
    this->setImpl(value.to<T>());
    return FutureSync<void>(0);
  }


  template<typename T>
  FutureSync<T> Property<T>::get() const
  {
    boost::mutex::scoped_lock lock(_mutex);
    return FutureSync<T>(this->getImpl());
  }

  template<typename T>
  FutureSync<void> Property<T>::set(const T& v)
  {
    boost::mutex::scoped_lock lock(_mutex);
    this->setImpl(v);
    return FutureSync<void>(0);
  }

  template<typename T>
  FutureSync<AnyValue> Property<T>::value() const
  {
    boost::mutex::scoped_lock lock(_mutex);
    return FutureSync<AnyValue>(AnyValue::from(this->getImpl()));
  }

  template<typename T>
  FutureSync<void> Property<T>::setValue(AutoAnyReference value)
  {
    boost::mutex::scoped_lock lock(_mutex);
    this->setImpl(value.to<T>());
    return FutureSync<void>(0);
  }

  template<class T>
  typename Property<T>::ScopedLockReadWrite
    Property<T>::getLockedReadWrite()
  {
    return ScopedLockReadWrite(*this);
  }

  template<class T>
  typename Property<T>::ScopedLockReadOnly
    Property<T>::getLockedReadOnly() const
  {
    return ScopedLockReadOnly(*this);
  }
}

#endif  // _QITYPE_DETAIL_PROPERTY_HXX_
