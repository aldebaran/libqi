#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_PROPERTY_HXX_
#define _QITYPE_DETAIL_PROPERTY_HXX_

#include <boost/thread/locks.hpp>
#include <qi/property.hpp>
#include <qi/future.hpp>
#include <qi/property.hpp>
#include <ka/errorhandling.hpp>

namespace qi
{
  inline FutureSync<void> GenericProperty::set(const AnyValue& v)
  {
    auto conv = v.convert(_type);
    if (!conv->type())
      throw std::runtime_error(std::string("Failed converting ") + v.type()->infoString() + " to " + _type->infoString());

    const auto convOwnedRef = conv.ownsReference();
    return Property<AnyValue>::set(AnyValue(conv.release(), false, convOwnedRef));
  }

  template<typename T>
  SignalingProperty<T>::SignalingProperty(SignalBase::OnSubscribers onsubscribe)
    : SignalF<void(const T&)>(std::move(onsubscribe))
  {
  }

  template<typename T>
  SignalingProperty<T>::SignalingProperty(ExecutionContext* execContext,
                                          SignalBase::OnSubscribers onsubscribe)
    : SignalF<void(const T&)>(execContext, std::move(onsubscribe))
  {
  }

  template <typename T>
  PropertyImpl<T>::PropertyImpl(ExecutionContext* execContext,
                                Getter getter,
                                Setter setter,
                                SignalBase::OnSubscribers onsubscribe)
    : SignalingProperty<T>(execContext, std::move(onsubscribe))
    , _getter(std::move(getter))
    , _setter(std::move(setter))
  {
  }

  template <typename T>
  PropertyImpl<T>::PropertyImpl(Getter getter, Setter setter, SignalBase::OnSubscribers onsubscribe)
    : SignalingProperty<T>(std::move(onsubscribe))
    , _getter(std::move(getter))
    , _setter(std::move(setter))
  {
  }

  template <typename T>
  PropertyImpl<T>::PropertyImpl(AutoAnyReference defaultValue,
                                Getter getter,
                                Setter setter,
                                SignalBase::OnSubscribers onsubscribe)
    : SignalingProperty<T>(std::move(onsubscribe))
    , _getter(std::move(getter))
    , _setter(std::move(setter))
    , _value(defaultValue.to<T>())
  {
  }

  template <typename T>
  PropertyImpl<T>::PropertyImpl(AutoAnyReference defaultValue,
                                ExecutionContext* execContext,
                                Getter getter,
                                Setter setter,
                                SignalBase::OnSubscribers onsubscribe)
    : SignalingProperty<T>(execContext, std::move(onsubscribe))
    , _getter(std::move(getter))
    , _setter(std::move(setter))
    , _value(defaultValue.to<T>())
  {
  }

  template<typename T>
  T PropertyImpl<T>::getImpl() const
  {
    if (_getter)
      return _getter(boost::ref(_value));
    else
      return _value;
  }
  template<typename T>
  void PropertyImpl<T>::setImpl(const T& v)
  {
    qiLogDebug("qitype.property") << "set " << this << " " << (!!_setter);
    if (_setter)
    {
      const bool ok = _setter(boost::ref(_value), v);
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
  ReadOnlyProperty<T>::ReadOnlyProperty(SignalingProperty<T>& s)
    : _source{ s }
    , _subscription{ s.connect(track([=](const T& val){ (*this)(val); }, this)) }
  {
  }

  template<typename T>
  ReadOnlyProperty<T>::~ReadOnlyProperty()
  {
    ka::invoke_catch(qi::exceptionLogError("qi.type.readonlyproperty",
                                           "Error while destroying a read only property"),
                     [&] {
                       this->destroy();
                       _source.disconnectAsync(_subscription);
                     });
  }

  template<typename T>
  FutureSync<T> ReadOnlyProperty<T>::get() const
  {
    return _source.get();
  }

  template<typename T>
  FutureSync<AnyValue> ReadOnlyProperty<T>::value() const
  {
    return _source.get().async().then(FutureCallbackType_Sync, [](const T& v) {
      return AnyValue::from(v);
    });
  }

  template<typename T>
  FutureSync<void> ReadOnlyProperty<T>::setValue(AutoAnyReference)
  {
    return makeFutureError<void>("Property is read-only, it cannot be set from this interface.");
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
  Property<T>::~Property()
  {
    _tracked.destroy();
    joinStrand();
    SignalBase::clearExecutionContext();
  }

  template<typename T>
  FutureSync<T> Property<T>::get() const
  {
    return strand().async(track([this]{ return this->getImpl(); }, &_tracked));
  }

  template<typename T>
  FutureSync<void> Property<T>::set(const T& v)
  {
    return strand().async(track([this, v]{ this->setImpl(v); }, &_tracked));
  }

  template<typename T>
  FutureSync<AnyValue> Property<T>::value() const
  {
    return strand().async(track([this]{ return AnyValue::from(this->getImpl()); }, &_tracked));
  }

  template<typename T>
  FutureSync<void> Property<T>::setValue(AutoAnyReference value)
  {
    const auto v = value.to<T>();
    return strand().async(track([this, v]{ this->setImpl(v); }, &_tracked));
  }

  template<typename T>
  Strand& Property<T>::strand() const
  {
    struct Src : boost::static_visitor<Strand&>, ka::src_t {};
    return boost::apply_visitor(Src{}, _strand);
  }

  template<typename T>
  void Property<T>::joinStrand() QI_NOEXCEPT(true)
  {
    struct JoinStrand : boost::static_visitor<void>
    {
      void operator()(Strand*) const
      {
        // Do nothing, we do not own the strand, we have no right to join it.
      }

      void operator()(Strand& strand) const
      {
        strand.join();
      }
    };
    boost::apply_visitor(JoinStrand{}, _strand);
  }
}

#endif  // _QITYPE_DETAIL_PROPERTY_HXX_
