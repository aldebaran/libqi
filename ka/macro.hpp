#ifndef KA_MACRO_HPP
#define KA_MACRO_HPP
#pragma once
#include <boost/config.hpp>
#include <boost/predef/compiler.h>

/// Specify that a function may throw or not. Do nothing if noexcept is not available.
#define KA_NOEXCEPT(cond) BOOST_NOEXCEPT_IF(cond)

/// Specify that a function may throw if the given expression may throw.
/// Do nothing if noexcept is not available.
#define KA_NOEXCEPT_EXPR(expr) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(expr))

/// Specify that a function is evaluable at compile-time.
/// Do nothing if constexpr is not available.
///
/// TODO: Remove this macro when all supported compilers support `constexpr`.
#define KA_CONSTEXPR BOOST_CONSTEXPR

/// True if the code is compiled with Visual Studio 2013 or below.
///
/// This is needed because some code must be adapted, due to the limited
/// C++11 support and bugs of Visual Studio 2013.
/// (compiler version 19 is Visual Studio 2015 one, see
/// https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering)
#define KA_COMPILER_VS2013_OR_BELOW \
  (0 < BOOST_COMP_MSVC && BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(19, 0, 0))

/// The only currently supported compiler that doesn't support member function
/// reference qualifiers is the visual studio 2013 one.
#define KA_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS \
  (!KA_COMPILER_VS2013_OR_BELOW)

/// If the expression is false, return false.
/// Designed to be used in a boolean function where all conditions must be true
/// for the function to be true.
/// See isRegular() in conceptpredicates.hpp for a use example.
#define KA_TRUE_OR_RETURN_FALSE( expr__ ) if (!(expr__)) return false

/// Derives (i.e. generates) a function that creates a value of a *non-template
/// type*. This kind of functions are called here "constructor functions".
///
/// Note: The type name is deduced from the function name, by appending "_t".
///
/// Example: Deriving a constructor function
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// struct my_stuff_t {
///   // `my_stuff_t` is constructible from an `int` and a `bool`.
///   ...
/// };
///
/// KA_DERIVE_CTOR_FUNCTION(my_stuff)
///
/// // In user code:
/// auto x = my_stuff(5, true); // `x` has type `my_stuff_t`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Precondition: 'ka/utility.hpp' has been included.
#define KA_DERIVE_CTOR_FUNCTION(FUNCTION)        \
  template<typename... Args>                     \
  FUNCTION##_t FUNCTION(Args&&... args) {        \
    return FUNCTION##_t{ka::fwd<Args>(args)...}; \
  }

/// Derives (i.e. generates) a function that creates a value of a *template
/// type*.
///
/// Template parameters are obtained from function arguments after decaying.
///
/// This macro is the template alternative to `KA_DERIVE_CTOR_FUNCTION`.
///
/// Example: Deriving a constructor function for a template type.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// template<typename T, typename U>
/// struct my_stuff_t {
///   // `my_stuff_t` is constructible from a `T` and a `U`.
///   ...
/// };
///
/// KA_DERIVE_CTOR_FUNCTION_TEMPLATE(my_stuff)
///
/// // In user code:
/// auto x = my_stuff(5, true); // `x` has type `my_stuff_t<int, bool>`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Precondition: 'ka/typetraits.hpp' and 'ka/utility.hpp' have been included.
#define KA_DERIVE_CTOR_FUNCTION_TEMPLATE(FUNCTION)                            \
  template<typename... Args>                                                  \
  FUNCTION##_t<ka::Decay<Args>...> FUNCTION(Args&&... args) {         \
    return FUNCTION##_t<ka::Decay<Args>...>{ka::fwd<Args>(args)...};  \
  }

/// Derives (i.e. generates) a function that creates a value of a *template
/// type* from no argument (`void`).
///
/// This macro is the `void` alternative to `KA_DERIVE_CTOR_FUNCTION_TEMPLATE`.
#define KA_DERIVE_CTOR_FUNCTION_TEMPLATE_VOID(FUNCTION) \
  inline FUNCTION##_t<void> FUNCTION() {                \
    return FUNCTION##_t<void>{};                        \
  }

#endif // KA_MACRO_HPP
