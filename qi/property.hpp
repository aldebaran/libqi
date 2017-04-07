#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_PROPERTY_HPP_
#define _QI_PROPERTY_HPP_

#include <functional>
#include <mutex>
#include <boost/function.hpp>
#include <qi/signal.hpp>
#include <qi/future.hpp>

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

  /// A typed interface for a property, that is also a signal.
  template<typename T>
  class PropertyInterface: public Signal<const T&>, public PropertyBase
  {
  public:
    /// Recalling the value type behind the property.
    using PropertyType = T;

    /// Recalling the signal type associated to this signal.
    using SignalType = Signal<const T&>;

    PropertyInterface(SignalBase::OnSubscribers&& onSubscriber = SignalBase::OnSubscribers())
      : Signal<const T&>(std::forward<SignalBase::OnSubscribers>(onSubscriber)) {}

    // PropertyBase support
    SignalBase* signal() override;
    FutureSync<AnyValue> value() const override;
    FutureSync<void> setValue(AutoAnyReference value) override;

    virtual FutureSync<T> get() const = 0;
    virtual FutureSync<void> set(const T& v) = 0;
  };

  /// A very common setter that works for most properties that have a simple storage.
  /// @example
  ///   std::string storage;
  ///   TypedProperty<std::string> property{
  ///       [&]{ return storage; },
  ///       propertySetterForStorage(storage)
  ///   };
  template <typename T>
  typename std::function<void(const T&, Signal<const T&>&)> propertySetterForStorage(T& storage)
  {
    return [&](const T& value, Signal<const T&>& signal)
    {
      if (storage != value)
      {
        storage = value;
        QI_EMIT signal(value);
      }
    };
  }

  /// A failing setter with a message explaining it is read-only.
  template <typename T>
  Future<void> propertySetterForReadOnly(const T&, Signal<const T&>&)
  {
    throw std::runtime_error("property is read-only, so it cannot be set");
  }

  /// A synchronous failing setter with a message explaining it is read-only.
  template <typename T>
  void propertySetterForReadOnlySync(const T& v, Signal<const T&>& s)
  {
    propertySetterForReadOnly(v, s);
  }

  /// A typed property, that is also a signal to be connectable to.
  template<typename T>
  class PropertyWithoutStorage: public PropertyInterface<T>
  {
  public:
    /// Recalling the signal type associated to this signal.
    using SignalType = PropertyInterface<T>;

    /// A getter returns a value stored anywhere.
    using Getter = std::function<Future<T>()>;

    /// A getter returns a value stored anywhere, synchrously.
    using GetterSync = std::function<T()>;

    /// A setter receives a value to store anywhere, and a signal to emit if successful.
    using Setter = std::function<Future<void>(const T&, typename std::reference_wrapper<SignalType>)>;

    /// A synchronous setter receives a value to store anywhere, and a signal to emit if successful.
    using SetterSync = std::function<void(const T&, typename std::reference_wrapper<SignalType>)>;

    /// Constructor using accessors.
    /// The setter can be omitted to make a read-only property.
    PropertyWithoutStorage(
        Getter&& getter,
        Setter&& setter = propertySetterForReadOnly<T>,
        SignalBase::OnSubscribers&& onSubscriber = SignalBase::OnSubscribers{});

    /// Constructor using sync accessors.
    /// The setter can be omitted to make a read-only property.
    PropertyWithoutStorage(
        GetterSync&& getter,
        SetterSync&& setter = propertySetterForReadOnlySync<T>,
        SignalBase::OnSubscribers&& onSubscriber = SignalBase::OnSubscribers{});

    /// Maps the interface to the getter.
    FutureSync<T> get() const override { return _getter(); }

    /// Maps the interface to the setter, the property itself is the signal to emit.
    FutureSync<void> set(const T& v) override { return _setter(v, *this); }

  private:
    Getter _getter;
    Setter _setter;
  };

  /// A property with the management of the storage already implemented, and is not safe.
  template<typename T>
  class PropertyWithStorage: public PropertyWithoutStorage<T>
  {
  public:
    /** Setter called with storage containing old value, and new value
    *  Returns true to invoke subscribers, false to 'abort' the update.
    */
    using Getter = boost::function<T(boost::reference_wrapper<const T>)>;
    using Setter = boost::function<bool(boost::reference_wrapper<T>, const T&)>;

    /**
     * @param getter value getter, default to reading _value
     * @param setter value setter, what it returns will be written to
     *        _value. If it returns false, update operation will
     *        be silently aborted (subscribers will not be called)
     * @param onsubscribe callback to call when subscribers connect or
     *        disconnect from the property
    */
    PropertyWithStorage(
        Getter&& getter = Getter{},
        Setter&& setter = Setter{},
        SignalBase::OnSubscribers&& onsubscribe = SignalBase::OnSubscribers{});

    PropertyWithStorage(
        AutoAnyReference&& defaultValue,
        Getter&& getter = Getter{},
        Setter&& setter = Setter{},
        SignalBase::OnSubscribers&& onsubscribe = SignalBase::OnSubscribers{});

    PropertyWithStorage<T>& operator=(const T& v) { this->set(v); return *this; }

  protected:
    T _storage;
    Getter _getterWithStorage;
    Setter _setterWithStorage;
  };

  /**
   * Provide access to a stored value and signal to connected callbacks when the value changed.
   * @see qi::Signal which implement a similar pattern but without storing the value.
   * @remark For thread-safety, consider using Property instead.
   * \includename{qi/property.hpp}
   */
  template <typename T>
  class UnsafeProperty: public PropertyWithStorage<T>
  {
  public:
    using Getter = typename PropertyWithStorage<T>::Getter;
    using Setter = typename PropertyWithStorage<T>::Setter;

    UnsafeProperty(
        Getter getter = Getter{},
        Setter setter = Setter{},
        SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers{});

    UnsafeProperty(
        AutoAnyReference defaultValue,
        Getter getter = Getter{},
        Setter setter = Setter{},
        SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers{});

    UnsafeProperty<T>& operator=(const T& v) { this->set(v); return *this; }
  };

  /** Povide thread-safe access to a stored value and signal to connected callbacks when the value changed.
      @see qi::Signal which implement a similar pattern but without storing the value.
      @remark For more performance in a single-threaded context, consider using UnsafeProperty instead.
      \includename{qi/property.hpp}
   */
  template<typename T>
  class Property: public PropertyWithStorage<T>
  {
  public:
    using PropertyType = T;
    using ImplType = PropertyWithStorage<T>;
    using Getter = typename ImplType::Getter;
    using Setter = typename ImplType::Setter;
    class ScopedLockReadWrite;
    class ScopedLockReadOnly;

    Property(
        Getter getter = Getter{},
        Setter setter = Setter{},
        SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers{});

    Property(
        AutoAnyReference defaultValue,
        Getter getter = Getter{},
        Setter setter = Setter{},
        SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers{});

    Property<T>& operator=(const T& v) { this->set(v); return *this; }
    FutureSync<T> get() const override;
    FutureSync<void> set(const T& v) override;

  protected:
    Getter _getterWithStorage;
    Setter _setterWithStorage;

  private:
    mutable std::mutex _mutex;
  };

  /** Provides (locking) exclusive write-enabled access to the value of a property.
      Locks the internal mutex of the property on construction, releases it on destruction.
      Behaves like a pointer to the property value.
   */
  template<class T>
  class Property<T>::ScopedLockReadWrite
  {
  public:
    ScopedLockReadWrite(Property<T>& property)
      : _property(property)
      , _lock(property._mutex)
    {
    }

    // non-copyable
    ScopedLockReadWrite(const ScopedLockReadWrite&) = delete;
    ScopedLockReadWrite& operator=(const ScopedLockReadWrite&) = delete;

    ~ScopedLockReadWrite()
    {
      _property(_property._storage); // Trigger update of the observer callbacks with the new value.
    }

    T& operator*() { return _property._storage; }
    T* operator->() { return &_property._storage; }

    const T& operator*() const { return _property._storage; }
    const T* operator->() const { return &_property._storage; }

  private:
    Property<T>& _property;
    std::unique_lock<std::mutex> _lock;
  };

  /** Provides (locking) exclusive read-only access to the value of a property.
      Locks the internal mutex of the property on construction, releases it on destruction.
      Behaves like a pointer to the property value.
   */
  template<class T>
  class Property<T>::ScopedLockReadOnly
  {
  public:
    ScopedLockReadOnly(const Property<T>& property)
      : _property(property)
      , _lock(property._mutex)
    {
    }

    // non-copyable
    ScopedLockReadOnly(const ScopedLockReadOnly&) = delete;
    ScopedLockReadOnly& operator=(const ScopedLockReadOnly&) = delete;

    const T& get() const { return _property._storage; }
    const T& operator*() const { return get(); }
    const T* operator->() const { return &get(); }

  private:
    const Property<T>& _property;
    std::unique_lock<std::mutex> _lock;
  };


  /// Type-erased property, simulating a typed property but using AnyValue.
  class QI_API GenericProperty : public Property<AnyValue>
  {
  public:
    GenericProperty(
        TypeInterface* type,
        Getter&& getter = Getter{},
        Setter&& setter = Setter{})
      : Property<AnyValue>(std::forward<Getter>(getter), std::forward<Setter>(setter)),
        _type(type)
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
