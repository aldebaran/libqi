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
#include <ka/empty.hpp>
#include <ka/functional.hpp>

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

  namespace details_property
  {
    template<typename T>
    struct ToFuture
    {
    // Regular:
      KA_GENERATE_FRIEND_REGULAR_OPS_0(ToFuture)

    // Function<Future<T>(U)>:
      template<typename U>
      Future<T> operator()(U&& u) const
      {
        return futurize(static_cast<T>(ka::fwd<U>(u)));
      }

    // Function<Future<T>(Future<T>)>:
      constexpr
      Future<T> operator()(Future<T> futT) const
      {
        return futT;
      }

    // Function<Future<T>(Future<U>)>:
      template<typename U>
      Future<T> operator()(Future<U> futU) const
      {
        return std::move(futU).andThen(FutureCallbackType_Sync, [](U u) {
          return ToFuture<T>{}(ka::mv(u));
        }).unwrap();
      }
    };

    // Returns true if some function F is nowhere defined, meaning that it can
    // never be called.
    template<typename T> constexpr
    bool isNowhereDefined(const T&) { return false; }

    template<typename F> constexpr
    bool isNowhereDefined(const std::function<F>& f) { return !f; }

    template<typename F> constexpr
    bool isNowhereDefined(const boost::function<F>& f) { return f.empty(); }

    constexpr
    bool isNowhereDefined(std::nullptr_t) { return true; }

    template<typename F> constexpr
    bool isNowhereDefined(F* p) { return p == nullptr; }

    // Wraps a function `F` into another function that calls `F` and converts its
    // result into a `Future<T>`.
    template<typename T, typename F> constexpr
    auto futurizeOutput(F&& f)
      -> decltype(ka::compose(ToFuture<T>{}, ka::fwd<F>(f)))
    {
      return ka::compose(ToFuture<T>{}, ka::fwd<F>(f));
    }

    template<typename T, typename F>
    typename PropertyImpl<T>::AsyncGetter toAsyncGetter(F&& get)
    {
      if (isNowhereDefined(get))
        return {};
      return futurizeOutput<T>(ka::fwd<F>(get));
    }

    template<typename T, typename F>
    typename PropertyImpl<T>::AsyncSetter toAsyncSetter(F&& set)
    {
      if (isNowhereDefined(set))
        return {};
      return futurizeOutput<bool>(ka::fwd<F>(set));
    }
  } // namespace details_property

  template<typename T>
  template<typename Get, typename Set>
  PropertyImpl<T>::PropertyImpl(Private,
                                AutoAnyReference defaultValue,
                                ExecutionContext* execContext,
                                Get&& getter,
                                Set&& setter,
                                SignalBase::OnSubscribers onsubscribe)
    : SignalingProperty<T>(execContext, std::move(onsubscribe))
    , _asyncGetter(details_property::toAsyncGetter<T>(ka::fwd<Get>(getter)))
    , _asyncSetter(details_property::toAsyncSetter<T>(ka::fwd<Set>(setter)))
    , _value(defaultValue.isValue() ? defaultValue.to<T>() : T{})
  {
  }

  template <typename T>
  PropertyImpl<T>::~PropertyImpl()
  {
    _tracked.destroy();
  }

  template<typename T>
  Future<T> PropertyImpl<T>::getImpl() const
  {
    if (_asyncGetter)
      return _asyncGetter(boost::cref(_value));
    else
      return futurize(_value);
  }
  template<typename T>
  Future<void> PropertyImpl<T>::setImpl(const T& v)
  {
    qiLogDebug("qitype.property") << "set " << this << " " << (!!_asyncSetter);
    Future<void> resFut;
    if (_asyncSetter)
    {
      resFut = _asyncSetter(boost::ref(_value), boost::cref(v))
        .andThen(FutureCallbackType_Sync, track([this](bool ok) {
          if (ok)
            (*this)(_value);
        }, &_tracked));
    }
    else
    {
      _value = v;
      (*this)(_value);
      resFut = futurize();
    }
    return resFut;
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
    return _source.get().async().andThen(FutureCallbackType_Sync, [](const T& v) {
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
    return this->getImpl();
  }

  template<typename T>
  FutureSync<void> UnsafeProperty<T>::set(const T& v)
  {
    return this->setImpl(v);
  }

  template<typename T>
  FutureSync<AnyValue> UnsafeProperty<T>::value() const
  {
    return this->getImpl().andThen(&AnyValue::from<T>);
  }

  template<typename T>
  FutureSync<void> UnsafeProperty<T>::setValue(AutoAnyReference value)
  {
    return this->setImpl(value.to<T>());
  }

  template<typename T>
  Property<T>::~Property()
  {
    this->_tracked.destroy();
    joinStrand();
    SignalBase::clearExecutionContext();
  }

  template<typename T>
  FutureSync<T> Property<T>::get() const
  {
    return strand().async(track([this]{ return this->getImpl(); }, &this->_tracked)).unwrap();
  }

  template<typename T>
  FutureSync<void> Property<T>::set(const T& v)
  {
    return strand().async(track([this, v]{ return this->setImpl(v); }, &this->_tracked)).unwrap();
  }

  template<typename T>
  FutureSync<AnyValue> Property<T>::value() const
  {
    return strand()
      .async(track([this] { return this->getImpl().andThen(&AnyValue::from<T>); }, &this->_tracked))
      .unwrap();
  }

  template<typename T>
  FutureSync<void> Property<T>::setValue(AutoAnyReference value)
  {
    const auto v = value.to<T>();
    return strand().async(track([this, v]{ return this->setImpl(v); }, &this->_tracked))
      .unwrap();
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
