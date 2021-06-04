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
#include <ka/macro.hpp>
#include <ka/mutablestore.hpp>
#include <ka/typetraits.hpp>
#include <qi/signal.hpp>
#include <qi/future.hpp>
#include <qi/strand.hpp>

KA_WARNING_PUSH()
// needs to have dll-interface to be used by clients
KA_WARNING_DISABLE(4251, )
// non dll-interface class * used as base for dll-interface class
KA_WARNING_DISABLE(4275, )

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

    explicit SignalingProperty(SignalBase::OnSubscribers onsubscribe = {});
    explicit SignalingProperty(ExecutionContext* execContext,
                               SignalBase::OnSubscribers onsubscribe = {});

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

  namespace details_property
  {
    // t' = f(t)
    template<typename F, typename T>
    using IsGetterSync =
      std::is_convertible<ka::ResultOf<F(boost::reference_wrapper<const T>)>,
                          T>;

    // futT = f(t)
    template<typename F, typename T>
    using IsGetterAsync =
      std::is_convertible<ka::ResultOf<F(boost::reference_wrapper<const T>)>,
                          Future<T>>;

    // mustInvoke = f(value, new)
    template<typename F, typename T>
    using IsSetterSync = std::is_convertible<
      ka::ResultOf<F(boost::reference_wrapper<T>, const T&)>,
      bool>;

    // futMustInvoke = f(value, new)
    template<typename F, typename T>
    using IsSetterAsync = std::is_convertible<
      ka::ResultOf<F(boost::reference_wrapper<T>, const T&)>,
      Future<bool>>;

    template<typename F, typename T>
    using IsGetter = ka::Disjunction<IsGetterSync<F, T>, IsGetterAsync<F, T>>;

    template<typename F, typename T>
    using IsSetter = ka::Disjunction<IsSetterSync<F, T>, IsSetterAsync<F, T>>;

    template<typename F, typename T, typename Out>
    using EnableIfIsGetter = ka::EnableIf<IsGetter<F, T>::value, Out>;

    template<typename F, typename T, typename Out>
    using EnableIfIsSetter = ka::EnableIf<IsSetter<F, T>::value, Out>;
  } // namespace details_property

  template<typename T>
  class PropertyImpl;

  namespace details_proxyproperty
  {
    template<typename T>
    void setUpProxy(PropertyImpl<T>& prop, AnyWeakObject object, const std::string& propertyName);
  }

  template<typename T>
  class PropertyImpl: public SignalingProperty<T>
  {
  public:
    /// Setter models Procedure<bool (T&, const T&)>
    /// Setter called with storage containing old value, and new value.
    /// Returns true to invoke subscribers, false to 'abort' the update.
    using Setter = boost::function<bool (boost::reference_wrapper<T>, const T&)>;

    /// Getter models Procedure<T (const T&)>
    using Getter = boost::function<T(boost::reference_wrapper<const T>)>;

    /// AsyncSetter models Procedure<Future<bool> (T&, const T&)>
    using AsyncSetter = boost::function<Future<bool> (boost::reference_wrapper<T>, const T&)>;

    /// AsyncGetter models Procedure<Future<T> (const T&)>
    using AsyncGetter = boost::function<Future<T> (boost::reference_wrapper<const T>)>;

  private:
    struct Private {};

    template<typename Get, typename Set>
    PropertyImpl(Private,
                 AutoAnyReference defaultValue,
                 ExecutionContext* execContext,
                 Get&& get,
                 Set&& set,
                 SignalBase::OnSubscribers onsubscribe);

  public:
    /// @note This overload only participates in overload resolution if `Get` can be called as a
    /// getter and `Set` can be called as a setter. See `Getter`, `AsyncGetter`, `Setter` and
    /// `AsyncSetter` for more details on the expected parameters.
    ///
    /// @param defaultValue Initial value of the property.
    /// @param execContext Passed to the `Signal` base constructor.
    /// @param getter Function that returns the value of the property. It can either be a
    /// synchronous function (returning a value of type `T`) or an asynchronous function (returning
    /// a value of type `Future<T>`).
    /// @param setter Function that sets the value of the property. If it returns false, update
    /// operation will be silently aborted (subscribers will not be called). It can either be a
    /// synchronous function (returning a boolean) or an asynchronous function (returning a
    /// `Future<bool>`).
    /// @param onsubscribe Function that is called when subscribers connect to or disconnect from
    /// the property.
    template <typename Get = AsyncGetter,
              typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    PropertyImpl(AutoAnyReference defaultValue,
                 ExecutionContext* execContext,
                 Get&& getter,
                 Set&& setter,
                 SignalBase::OnSubscribers onsubscribe = {})
      : PropertyImpl(Private{},
                     std::move(defaultValue),
                     execContext,
                     ka::fwd<Get>(getter),
                     ka::fwd<Set>(setter),
                     std::move(onsubscribe))
    {
    }

    /// @overload
    template<typename Get = AsyncGetter,
             typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    PropertyImpl(AutoAnyReference defaultValue,
                 Get&& getter = {},
                 Set&& setter = {},
                 SignalBase::OnSubscribers onsubscribe = {})
      : PropertyImpl(Private{},
                     std::move(defaultValue),
                     nullptr,
                     ka::fwd<Get>(getter),
                     ka::fwd<Set>(setter),
                     std::move(onsubscribe))
    {
    }

    /// @overload
    template<typename Get = AsyncGetter,
             typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    PropertyImpl(ExecutionContext* execContext,
                 Get&& getter = {},
                 Set&& setter = {},
                 SignalBase::OnSubscribers onsubscribe = {})
      : PropertyImpl(Private{},
                     {},
                     execContext,
                     ka::fwd<Get>(getter),
                     ka::fwd<Set>(setter),
                     std::move(onsubscribe))
    {
    }

    /// @overload
    template<typename Get = AsyncGetter,
             typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    PropertyImpl(Get&& getter = {},
                 Set&& setter = {},
                 SignalBase::OnSubscribers onsubscribe = {})
      : PropertyImpl(Private{},
                     {},
                     nullptr,
                     ka::fwd<Get>(getter),
                     ka::fwd<Set>(setter),
                     std::move(onsubscribe))
    {
    }

    ~PropertyImpl() override;

    virtual FutureSync<void> set(const T& v) = 0;

    PropertyImpl<T>& operator=(const T& v) { this->set(v).value(); return *this; }

  protected:
    friend void details_proxyproperty::setUpProxy<>(PropertyImpl<T>&, AnyWeakObject, const std::string&);

    AsyncGetter _asyncGetter;
    AsyncSetter _asyncSetter;
    T      _value;

    Future<T> getImpl() const;
    Future<void> setImpl(const T& v);

    struct Tracked : public Trackable<Tracked> { using Trackable<Tracked>::destroy; };
    mutable Tracked _tracked;
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

  public:
    using Getter = typename ImplType::Getter;
    using Setter = typename ImplType::Setter;
    using AsyncGetter = typename ImplType::AsyncGetter;
    using AsyncSetter = typename ImplType::AsyncSetter;

    using ImplType::ImplType;

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

  public:
    using Getter = typename ImplType::Getter;
    using Setter = typename ImplType::Setter;
    using AsyncGetter = typename ImplType::AsyncGetter;
    using AsyncSetter = typename ImplType::AsyncSetter;

    /// Uses the strand that is passed but does not own it.
    /// That strand will be used to wrap the getters and setters and synchronize them.
    ///
    /// @see PropertyImpl<T>::PropertyImpl
    /// @overload
    template<typename Get = AsyncGetter, typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    Property(AutoAnyReference defaultValue,
             Strand& strand,
             Get&& getter = {},
             Set&& setter = {},
             SignalBase::OnSubscribers onsubscribe = {})
      : ImplType(std::move(defaultValue),
                 &strand,
                 ka::fwd<Get>(getter),
                 ka::fwd<Set>(setter),
                 std::move(onsubscribe))
      , _strand{ &strand }
    {
    }

    /// Overload that deduces the default value to use.
    ///
    /// @see PropertyImpl<T>::PropertyImpl
    /// @overload
    template<typename Get = AsyncGetter, typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    Property(Strand& strand,
             Get&& getter = {},
             Set&& setter = {},
             SignalBase::OnSubscribers onsubscribe = {})
      : ImplType(&strand,
                 ka::fwd<Get>(getter),
                 ka::fwd<Set>(setter),
                 std::move(onsubscribe))
      , _strand{ &strand }
    {
    }

    /// Overload that instantiates a strand by default and owns it.
    ///
    /// @see PropertyImpl<T>::PropertyImpl
    /// @overload
    template<typename Get = AsyncGetter, typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    Property(Get&& getter = {},
             Set&& setter = {},
             SignalBase::OnSubscribers onsubscribe = {})
      : ImplType(ka::fwd<Get>(getter),
                 ka::fwd<Set>(setter),
                 std::move(onsubscribe))
    {}

    /// @see PropertyImpl<T>::PropertyImpl
    /// @overload
    template<typename Get = AsyncGetter, typename Set = AsyncSetter,
              details_property::EnableIfIsGetter<Get, T, int> = 0,
              details_property::EnableIfIsSetter<Set, T, int> = 0>
    Property(AutoAnyReference defaultValue,
             Get&& getter = {},
             Set&& setter = {},
             SignalBase::OnSubscribers onsubscribe = {})
      : ImplType(std::move(defaultValue),
                 ka::fwd<Get>(getter),
                 ka::fwd<Set>(setter),
                 std::move(onsubscribe))
    {}

    ~Property() override;

    Property<T>& operator=(const T& v) { this->set(v); return *this; }

    FutureSync<T> get() const override;
    FutureSync<void> set(const T& v) override;

    SignalBase* signal() override { return this; }
    FutureSync<void> setValue(AutoAnyReference value) override;
    FutureSync<AnyValue> value() const override;

  private:
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
      this->set(v).value();
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

KA_WARNING_POP()

#endif  // _QITYPE_PROPERTY_HPP_
