#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_PROPERTY_HPP_
#define _QI_PROPERTY_HPP_

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/variant.hpp>
#include <ka/mutablestore.hpp>
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
    PropertyBase() = default;

    // NonCopyable
    PropertyBase(const PropertyBase&) = delete;
    PropertyBase& operator=(const PropertyBase&) = delete;

    virtual ~PropertyBase() = default;
    virtual SignalBase* signal() = 0;
    virtual FutureSync<void> setValue(AutoAnyReference value) = 0;
    virtual FutureSync<AnyValue> value() const = 0;
  };

  /// Base type for all type-parameterized properties that allow typed access to their value.
  ///
  /// std::Semiregular T
  template<typename T>
  class ReadableProperty : public PropertyBase
  {
  public:
    // Forced to keep the FutureSync as a return value type to ensure compatibility with
    // previous code of PropertyImpl.
    virtual FutureSync<T> get() const = 0;

    /// Same as `get()`.
    Future<T> operator*() const
    {
      return get().async();
    }

  // Readable:
    friend Future<T> src(const ReadableProperty& prop)
    {
      return *prop;
    }
  };

  /// A typed-parameterized readable property that is also a signal.
  ///
  /// std::Semiregular T
  template <typename T>
  class SignalingProperty : public SignalF<void(const T&)>,
                            public ReadableProperty<T>
  {
  public:
    using SignalType = SignalF<void(const T&)>;
    using PropertyType = T;

    explicit SignalingProperty(SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());
    explicit SignalingProperty(ExecutionContext* execContext,
                               SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());

    SignalBase* signal() override { return this; }
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
  class PropertyImpl: public SignalingProperty<T>
  {
  public:
    /** Setter called with storage containing old value, and new value
    *  Returns true to invoke subscribers, false to 'abort' the update.
    */
    using Setter = boost::function<bool (boost::reference_wrapper<T>, const T&)>;
    using Getter = boost::function<T(boost::reference_wrapper<const T>)>;

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

    /// This overload takes an ExecutionContext pointer that will be passed to the Signal
    /// constructor.
    PropertyImpl(ExecutionContext* execContext, Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());

    PropertyImpl(AutoAnyReference defaultValue, Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());

    /// This overload takes an ExecutionContext pointer that will be passed to the Signal
    /// constructor.
    PropertyImpl(AutoAnyReference defaultValue, ExecutionContext* execContext = nullptr,
      Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers());

    virtual FutureSync<void> set(const T& v) = 0;

    PropertyImpl<T>& operator=(const T& v) { this->set(v); return *this; }

  protected:
    Getter _getter;
    Setter _setter;
    T      _value;

    T getImpl() const;
    void setImpl(const T& v);
  };

  /// Property that only provides an interface to get its value and forbids (as much as possible)
  /// setting it.
  ///
  /// std::Semiregular T
  template <typename T>
  class ReadOnlyProperty // Sadly we are forced to inherit SignalingProperty publicly for the qi
                         // type system, which means users of this type can always cast it to a
                         // reference to PropertyBase and call the setter.
    : public SignalingProperty<T>,
      public Trackable<ReadOnlyProperty<T>>
  {
  public:
    using ImplType = SignalingProperty<T>;
    using SignalType = typename ImplType::SignalType;
    using PropertyType = typename ImplType::PropertyType;
    using FunctionType = typename ImplType::FunctionType;

    /// Constructs this property from a source property on which it will register as a signal
    /// subscriber. The source property must outlive the read-only property.
    explicit ReadOnlyProperty(SignalingProperty<T>& source);

    ~ReadOnlyProperty();

    FutureSync<T> get() const override;
    FutureSync<AnyValue> value() const override;

  private:
    /// Returns a future in error explaining that this class does not allow setting the value.
    FutureSync<void> setValue(AutoAnyReference) override;

  protected:
    SignalingProperty<T>& _source;
    SignalSubscriber _subscription;
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

    /// Instantiates a strand by default and owns it.
    /// That strand will be used to wrap the getters and setters and synchronize them.
    Property(Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(getter), std::move(setter), std::move(onsubscribe))
    {}

    /// Uses the strand that is passed but does not own it.
    /// That strand will be used to wrap the getters and setters and synchronize them.
    Property(Strand& strand,
             Getter getter = Getter(),
             Setter setter = Setter(),
             SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(&strand, std::move(getter), std::move(setter), std::move(onsubscribe))
      , _strand{ &strand }
    {
    }

    /// Instantiates a strand by default and owns it.
    /// That strand will be used to wrap the getters and setters and synchronize them.
    Property(AutoAnyReference defaultValue, Getter getter = Getter(), Setter setter = Setter(),
      SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(defaultValue), std::move(getter), std::move(setter), std::move(onsubscribe))
    {}

    /// Uses the strand that is passed but does not own it.
    /// That strand will be used to wrap the getters and setters and synchronize them.
    Property(AutoAnyReference defaultValue,
             Strand& strand,
             Getter getter = Getter(),
             Setter setter = Setter(),
             SignalBase::OnSubscribers onsubscribe = SignalBase::OnSubscribers())
      : PropertyImpl<T>(std::move(defaultValue),
                        &strand,
                        std::move(getter),
                        std::move(setter),
                        std::move(onsubscribe))
      , _strand{ &strand }
    {
    }

    ~Property() override;

    Property<T>& operator=(const T& v) { this->set(v); return *this; }

    FutureSync<T> get() const override;
    FutureSync<void> set(const T& v) override;

    SignalBase* signal() override { return this; }
    FutureSync<void> setValue(AutoAnyReference value) override;
    FutureSync<AnyValue> value() const override;

  private:
    // This class has to have a Trackable member even though it holds a strand because it does not
    // always own that strand, and thus it can not always join it before being destroyed. Tracking
    // a member ensures that no stranded callback might be executed after it has been destroyed.
    struct Tracked : public Trackable<Tracked> { using Trackable<Tracked>::destroy; };
    mutable Tracked _tracked;

    Strand& strand() const;
    void joinStrand() QI_NOEXCEPT(true);

    // TODO:
    // Right now, mutable_store_t stores the Mutable first (the pointer in this case) which
    // is the one that will be instanciated by the default constructor. But since Strand is
    // neither copyable nor movable and mutable_store_t relies on boost::variant and boost::variant
    // does not support in-place construction of its value, the Strand value can never be set to the
    // mutable_store_t. So use ka::mutable_store_t once we are able to construct values in-place in
    // it.
    mutable boost::variant<Strand, Strand*> _strand;
  };

  /// Type-erased property, simulating a typed property but using AnyValue.
  class QI_API GenericProperty : public Property<AnyValue>
  {
  public:
    /// Constructs a property with a default value. The property type is the type of the given
    /// value.
    ///
    /// This means that if the property is exposed, its signature will be the one of the type of the
    /// value instead of `AnyValue`'s default signature (i.e. dynamic).
    ///
    /// All arguments are forwarded to the constructor of the base class.
    /// @see qi::Property<AnyValue>::Property(AutoAnyReference, ...)
    ///
    /// Precondition: The value must have a valid type.
    template<typename... Args>
    explicit GenericProperty(const AutoAnyReference& defaultValue, Args&&... args)
      : Property<AnyValue>(defaultValue, ka::fwd<Args>(args)...)
      , _type{ defaultValue.type() }
    {
      _setSignature(makeTupleSignature({ _type }));
    }

    /// Constructs a property of the given type.
    ///
    /// This means that if the property is exposed, its signature will be the one of the type
    /// instead of `AnyValue`'s default signature (i.e. dynamic).
    ///
    /// A default constructed value of the type and the rest of the arguments are forwarded to the
    /// constructor of the base class.
    /// @see qi::Property<AnyValue>::Property(AutoAnyReference, ...)
    ///
    /// Precondition: The type must be valid (not null).
    template<typename... Args>
    explicit GenericProperty(TypeInterface* type, Args&&... args)
      : GenericProperty(AnyValue{ type }, ka::fwd<Args>(args)...)
    {
    }

    GenericProperty& operator=(const AnyValue& v)
    {
      this->set(v);
      return *this;
    }

    FutureSync<void> setValue(AutoAnyReference value) override
    {
      return set(AnyValue(value, false, false));
    }

    FutureSync<void> set(const AnyValue& v) override;

    qi::Signature signature() const override
    {
      return makeTupleSignature({ _type });
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
