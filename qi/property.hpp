#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_PROPERTY_HPP_
#define _QI_PROPERTY_HPP_

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <qi/signal.hpp>
#include <qi/future.hpp>
#include <qi/strand.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
   // needs to have dll-interface to be used by clients
#  pragma warning( disable: 4251 )
   // non dll-interface class * used as base for dll-interface class
#  pragma warning( disable: 4275 )
#endif

namespace qi
{
  /** Type-erased virtual interface implemented by all Property classes.
  */
  class QI_API PropertyBase
  {
  public:
    virtual ~PropertyBase() = default;
    virtual SignalBase* signal() = 0;
    virtual FutureSync<void> setValue(AutoAnyReference value) = 0;
    virtual FutureSync<AnyValue> value() const = 0;
  };

  namespace util
  {
    // Used as a qi::Property::Setter to only store and dispatch a value if it was changed.
    struct SetAndNotifyIfChanged
    {
      template<class T>
      bool operator()(boost::reference_wrapper<T> ref_to_current_value, const T& newValue) const {
        T& currentValue = ref_to_current_value;
        if (currentValue != newValue)
        {
          currentValue = newValue;
          return true;
        }
        return false;
      }
    };

  }

  template<typename T>
  class PropertyImpl: public SignalF<void(const T&)>, public PropertyBase
  {
  public:
    /** Setter called with storage containing old value, and new value
    *  Returns true to invoke subscribers, false to 'abort' the update.
    */
    using Setter = boost::function<bool (boost::reference_wrapper<T>, const T&)>;
    using Getter = boost::function<T(boost::reference_wrapper<const T>)>;
    using SignalType = SignalF<void(const T&)>;
    using PropertyType = T;

    /**
     * @param getter value getter, default to reading _value.
     * @param setter value setter.
              If it returns false, update operation will
     *        be silently aborted (subscribers will not be called).
     * @param onsubscribe callback to call when subscribers connect or
     *        disconnect from the property.
    */
    PropertyImpl(Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());

    PropertyImpl(AutoAnyReference defaultValue, Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());

    virtual FutureSync<T> get() const = 0;
    virtual FutureSync<void> set(const T& v) = 0;

    PropertyImpl<T>& operator=(const T& v) { this->set(v); return *this; }

    SignalBase* signal() override { return this; }

  protected:
    Getter _getter;
    Setter _setter;
    T      _value;

    T getImpl() const;
    void setImpl(const T& v);
  };

  /** Povide access to a stored value and signal to connected callbacks when the value changed.
      @see qi::Signal which implement a similar pattern but without storing the value.
      @remark For thread-safety, consider using Property instead.
      \includename{qi/property.hpp}
   */
  template<typename T>
  class UnsafeProperty: public PropertyImpl<T>
  {
  public:
    using ImplType = PropertyImpl<T>;
    using Getter = typename ImplType::Getter;
    using Setter = typename ImplType::Setter;

    UnsafeProperty(Getter getter = Getter(), Setter setter = Setter(),
                   SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(getter), std::move(setter), std::move(onsubscribe))
    {}

    UnsafeProperty(AutoAnyReference defaultValue, Getter getter = Getter(), Setter setter = Setter(),
                   SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(defaultValue), std::move(getter), std::move(setter), std::move(onsubscribe))
    {}

    UnsafeProperty<T>& operator=(const T& v) { this->set(v); return *this; }

    FutureSync<T> get() const override;
    FutureSync<void> set(const T& v) override;

    SignalBase* signal() override { return this; }
    FutureSync<void> setValue(AutoAnyReference value) override;
    FutureSync<AnyValue> value() const override;

  };


  /** Povide thread-safe access to a stored value and signal to connected callbacks when the value changed.
      @see qi::Signal which implement a similar pattern but without storing the value.
      @remark For more performance in a single-threaded context, consider using UnsafeProperty instead.
      \includename{qi/property.hpp}
   */
  template<typename T>
  class Property: public PropertyImpl<T>
  {
  public:
    using ImplType = PropertyImpl<T>;
    using Getter = typename ImplType::Getter;
    using Setter = typename ImplType::Setter;

    Property(Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(getter), std::move(setter), std::move(onsubscribe))
    {}

    Property(AutoAnyReference defaultValue, Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(defaultValue), std::move(getter), std::move(setter), std::move(onsubscribe))
    {}

    Property<T>& operator=(const T& v) { this->set(v); return *this; }

    FutureSync<T> get() const override;
    FutureSync<void> set(const T& v) override;

    SignalBase* signal() override { return this; }
    FutureSync<void> setValue(AutoAnyReference value) override;
    FutureSync<AnyValue> value() const override;

  private:
    mutable Strand _strand;
  };

  /// Type-erased property, simulating a typed property but using AnyValue.
  class QI_API GenericProperty : public Property<AnyValue>
  {
  public:
    GenericProperty(TypeInterface* type, Getter getter = Getter(), Setter setter = Setter())
    :Property<AnyValue>(getter, setter)
    , _type(type)
    { // Initialize with default value for given type
      set(AnyValue(_type));
      std::vector<TypeInterface*> types(&_type, &_type + 1);
      _setSignature(makeTupleSignature(types));
    }

    GenericProperty& operator=(const AnyValue& v) { this->set(v); return *this; }

    FutureSync<void> setValue(AutoAnyReference value) override { return set(AnyValue(value, false, false));}
    FutureSync<void> set(const AnyValue& v) override;
    qi::Signature signature() const override {
      return makeTupleSignature(std::vector<TypeInterface*>(&_type, &_type + 1));
    }
  private:
    TypeInterface* _type;
  };
}

#include <qi/type/detail/property.hxx>

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_PROPERTY_HPP_
