#ifndef QI_EITHER_HPP
#define QI_EITHER_HPP
#pragma once
#include <boost/variant.hpp>
#include <ka/utility.hpp>

namespace qi
{
  /// Convenient alias to a variant of two types.
  template<typename A, typename B>
  using Either = boost::variant<A, B>;

  /// Helper to create visitors.
  ///
  /// See `visit` for an example.
  ///
  /// There is no constraint on `T`.
  template<typename T>
  using VisitorBase = boost::static_visitor<T>;

  /// Applies the polymorphic procedure on the content of a 'sum type'
  /// (`Either`,  `boost::variant`...).
  ///
  /// Example: Defining a visitor and applying it.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct Norm : VisitorBase<std::size_t>
  /// {
  ///   std::size_t operator()(const std::string& x) const
  ///   {
  ///     return x.size();
  ///   }
  ///   std::size_t operator()(int i) const
  ///   {
  ///     return i;
  ///   }
  /// };
  ///
  /// // In a function:
  /// using E = Either<std::string, int>;
  /// {
  ///   E e{"abc"};
  ///   std::size_t i = visit(Norm{}, e); // i == 3
  /// }
  /// {
  ///   E e{12};
  ///   std::size_t i = visit(Norm{}, e); // i == 12
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// PolymorphicProcedure<U (A || ...)> Proc,
  /// boost::variant<A...> T
  template<typename Proc, typename T>
  auto visit(Proc&& proc, T&& variant)
    -> decltype(boost::apply_visitor(ka::fwd<Proc>(proc), ka::fwd<T>(variant)))
  {
    using ka::fwd;
    return boost::apply_visitor(fwd<Proc>(proc), fwd<T>(variant));
  }
} // namespace qi

#endif // QI_EITHER_HPP

