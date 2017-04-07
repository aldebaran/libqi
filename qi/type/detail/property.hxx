#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_PROPERTY_HXX_
#define _QITYPE_DETAIL_PROPERTY_HXX_

namespace qi
{
  // PropertyInterface
  //-------------------------------------------------------------
  template<typename T>
  SignalBase* PropertyInterface<T>::signal() { return this; }

  template<typename T>
  FutureSync<AnyValue> PropertyInterface<T>::value() const
  {
    Promise<AnyValue> promise;
    adaptFuture(get().async(), promise, AdaptFutureOption_ForwardCancel);
    return promise.future();
  }

  template<typename T>
  FutureSync<void> PropertyInterface<T>::setValue(AutoAnyReference value)
  {
    return set(value.to<T>());
  }

  // PropertyWithoutStorage
  //-------------------------------------------------------------
  template<typename T>
  PropertyWithoutStorage<T>::PropertyWithoutStorage(
      Getter&& getter,
      Setter&& setter,
      SignalBase::OnSubscribers&& onSubscriber)
    : PropertyInterface<T>(std::forward<SignalBase::OnSubscribers>(onSubscriber)),
      _getter(getter),
      _setter(setter)
  {}

  template<typename T>
  PropertyWithoutStorage<T>::PropertyWithoutStorage(
      GetterSync&& getter,
      SetterSync&& setter,
      SignalBase::OnSubscribers&& onSubscriber)
    : PropertyInterface<T>(std::forward<SignalBase::OnSubscribers>(onSubscriber)),
      _getter([=]{ return Future<T>{getter()}; }),
      _setter([=](const T& v, std::reference_wrapper<SignalType> s){ setter(v, s); return Future<void>{nullptr}; })
  {}

  // PropertyWithStorage
  //-------------------------------------------------------------
  template<typename T>
  PropertyWithStorage<T>::PropertyWithStorage(
      Getter&& getter,
      Setter&& setter,
      SignalBase::OnSubscribers&& onSubscriber)
    : PropertyWithStorage<T>(
        T{},
        std::forward<Getter>(getter),
        std::forward<Setter>(setter),
        std::forward<SignalBase::OnSubscribers>(onSubscriber))
  {}

  template<typename T>
  PropertyWithStorage<T>::PropertyWithStorage(
      AutoAnyReference&& defaultValue,
      Getter&& getter,
      Setter&& setter,
      SignalBase::OnSubscribers&& onSubscriber)
    : PropertyWithoutStorage<T>(
        [=] // getter
        {
          if (_getterWithStorage)
            return _getterWithStorage(boost::ref(static_cast<const T&>(_storage)));
          else
            return _storage;
        },
        [=](const T& value, Signal<const T&>& signal) // setter
        {
          if (_setterWithStorage)
          {
            const bool ok = _setterWithStorage(boost::ref(_storage), value);
            if (ok)
              signal(_storage);
          }
          else
          {
            _storage = value;
            signal(_storage);
          }
        },
        std::forward<SignalBase::OnSubscribers>(onSubscriber)),
      _storage(defaultValue.to<T>()),
      _getterWithStorage(getter),
      _setterWithStorage(setter)
  {}

  // UnsafeProperty
  //-------------------------------------------------------------
  template<typename T>
  UnsafeProperty<T>::UnsafeProperty(
      Getter getter,
      Setter setter,
      SignalBase::OnSubscribers onSubscriber)
    : PropertyWithStorage<T>(
        std::move(getter),
        std::move(setter),
        std::move(onSubscriber))
  {}

  template<typename T>
  UnsafeProperty<T>::UnsafeProperty(
      AutoAnyReference defaultValue,
      Getter getter,
      Setter setter,
      SignalBase::OnSubscribers onSubscriber)
    : PropertyWithStorage<T>(
        std::move(defaultValue),
        std::move(getter),
        std::move(setter),
        std::move(onSubscriber))
  {}

  // Property
  //-------------------------------------------------------------
  template<typename T>
  Property<T>::Property(
      Getter getter,
      Setter setter,
      SignalBase::OnSubscribers onsubscribe)
    : Property<T>(
        T{},
        std::move(getter),
        std::move(setter),
        std::move(onsubscribe))
  {}

  template<typename T>
  Property<T>::Property(
      AutoAnyReference defaultValue,
      Getter getter,
      Setter setter,
      SignalBase::OnSubscribers onsubscribe)
    : PropertyWithStorage<T>(
        std::move(defaultValue),
        std::move(getter),
        std::move(setter),
        std::move(onsubscribe))
  {}

  template<typename T>
  FutureSync<T> Property<T>::get() const
  {
    std::unique_lock<std::mutex> lock{_mutex};
    return PropertyWithStorage<T>::get();
  }

  template<typename T>
  FutureSync<void> Property<T>::set(const T& v)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    return PropertyWithStorage<T>::set(v);
  }

  // GenericProperty
  //-------------------------------------------------------------
  inline FutureSync<void> GenericProperty::set(const AnyValue& v)
  {
    std::pair<AnyReference, bool> conv = v.convert(_type);
    if (!conv.first.type())
      throw std::runtime_error(std::string("Failed converting ") + v.type()->infoString() + " to " + _type->infoString());

    Property<AnyValue>::set(AnyValue(conv.first, false, conv.second));

    return FutureSync<void>(0);
  }
}

#endif  // _QITYPE_DETAIL_PROPERTY_HXX_
