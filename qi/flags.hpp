/*
**  Copyright (C) 2017 SoftBank Robotics
**  See COPYING for the license
*/

#ifndef QI_FLAGS_HPP
#define QI_FLAGS_HPP

#include <ka/typetraits.hpp>
#include <ka/macroregular.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/numeric.hpp>
#include <algorithm>

namespace qi
{
  /// Enumeration E
  ///
  /// This overload is only available for enum types through SFINAE because
  /// std::underlying_type has undefined behavior for non-enum types.
  template <typename E, typename = ka::EnableIf<std::is_enum<E>::value>>
  auto underlying(const E& e) -> ka::UnderlyingType<E>
  {
    return static_cast<ka::UnderlyingType<E>>(e);
  }

  /// Class providing type-safe flags semantics for bitwise combinable types.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  /// enum class BehaviorOption
  /// {
  ///   MoveAround,
  ///   MoveArms,
  ///   Speak,
  /// };
  /// using Opt = BehaviorOption;
  /// using F = Flags<Opt>;
  /// F flags = F({Opt::MoveAround, Opt::Speak});
  ///
  /// // or
  ///
  /// F flags;
  /// flags.set(Opt::MoveArms).set(Opt::MoveAround);
  ///
  /// // or
  ///
  /// F flags1 = F{Opt::MoveAround};
  /// F flags2 = F{Opt::MoveArms};
  /// F flags3 = flags2;
  /// F flags = ( flags1 | flags2 ) & flags3;
  /// // flags == flags2
  ///
  /// // ...
  /// startBehavior(flags);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  ///
  /// HasUnderlying<BitmaskType> Type
  template <typename Type>
  class Flags
  {
  public:
    using type = Type;
    using underlying_type = ka::Decay<decltype(underlying(std::declval<Type>()))>;

  private:
    explicit Flags(underlying_type value)
      : _value(value)
    {}

  public:
  // Regular:
    Flags()
      : _value{}
    {}

    KA_GENERATE_FRIEND_REGULAR_OPS_1(Flags, _value)

  // HasUnderlying:
    friend const underlying_type& underlying(const Flags& f)
    {
      return f._value;
    }

  // Custom:
    explicit Flags(std::initializer_list<Type> values)
      : _value(boost::accumulate(values | boost::adaptors::transformed([](const Type& t) { return underlying(t); }),
                                 underlying_type{},
                                 std::bit_or<underlying_type>()))
    {}

    explicit Flags(const Type& t)
      : _value(underlying(t))
    {}

    bool test(const Type& t) const
    {
      return (_value & underlying(t)) != underlying_type{};
    }

    Flags& set(const Type& t)
    {
      _value |= underlying(t);
      return *this;
    }

    Flags& reset(const Type& t)
    {
      _value &= ~underlying(t);
      return *this;
    }

    friend Flags operator&(const Flags& a, const Flags& b)
    {
      return Flags{a._value & b._value};
    }

    friend Flags operator|(const Flags& a, const Flags& b)
    {
      return Flags{a._value | b._value};
    }

    friend Flags operator^(const Flags& a, const Flags& b)
    {
      return Flags{a._value ^ b._value};
    }

    friend Flags operator~(const Flags& a)
    {
      return Flags{~a._value};
    }

    friend Flags& operator|=(Flags& a, const Flags& b)
    {
      a._value |= b._value;
      return a;
    }

    friend Flags& operator&=(Flags& a, const Flags& b)
    {
      a._value &= b._value;
      return a;
    }

    friend Flags& operator^=(Flags& a, const Flags& b)
    {
      a._value ^= b._value;
      return a;
    }

  private:
    underlying_type _value;
  };
}

#endif // QI_FLAGS_HPP
