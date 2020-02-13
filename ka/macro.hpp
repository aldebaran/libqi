#ifndef KA_MACRO_HPP
#define KA_MACRO_HPP
#pragma once
#include <boost/config.hpp>
#include <boost/predef/compiler.h>
#include <boost/preprocessor.hpp>

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

/// `KA_PRAGMA(p)`
///
/// Expands to a pragma directive `p` in a compiler specific manner. The
/// directive must be valid for all supported compilers. Otherwise the behavior
/// of this macro is undefined.
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_PRAGMA
#  if BOOST_COMP_GNUC || BOOST_COMP_CLANG
#    define KA_PRAGMA(x) _Pragma(#x)
#  elif BOOST_COMP_MSVC
#    define KA_PRAGMA(x) __pragma(x)
#  else
#    define KA_PRAGMA(_)
#  endif
#endif

/// `KA_PRAGMA_MESSAGE(message)`
///
/// Expands to a pragma directive that will print the string literal `message`
/// in a compiler specific manner.
///
/// Example: Welcoming a library user with a nice message.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// KA_PRAGMA_MESSAGE("Welcome to the ka library !")
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_PRAGMA_MESSAGE
#  if BOOST_COMP_MSVC
#    define KA_PRAGMA_MESSAGE(msg) \
       KA_PRAGMA(message (__FILE__ "(" BOOST_PP_STRINGIZE(__LINE__) "): " msg))
#  elif BOOST_COMP_GNUC || BOOST_COMP_CLANG
#    define KA_PRAGMA_MESSAGE(msg) KA_PRAGMA(message (msg))
#  else
#    define KA_PRAGMA_MESSAGE(_)
#  endif
#endif

/// `KA_PRAGMA_WARNING_ACTION(action)`
///
/// Expands to a warning pragma directive `action`. The directive must be
/// valid for all supported compilers. Otherwise the behavior of this macro is
/// undefined.
///
/// Warning pragma directives are:
///   - For GNUC and Clang: `GCC diagnostic action`
///   - For MSVC:           `warning(action)`
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_PRAGMA_WARNING_ACTION
#  if BOOST_COMP_MSVC
#    define KA_PRAGMA_WARNING_ACTION(act) KA_PRAGMA(warning(act))
#  elif BOOST_COMP_GNUC || BOOST_COMP_CLANG
     // clang supports GCC diagnostic syntax.
#    define KA_PRAGMA_WARNING_ACTION(act) KA_PRAGMA(GCC diagnostic act)
#  else
#    define KA_PRAGMA_WARNING_ACTION(_)
#  endif
#endif

/// `KA_WARNING_PUSH()`
///
/// Pushes the current state of warning pragma directives so it can be retrieved
/// later with `KA_WARNING_POP()`.
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_WARNING_PUSH
#  define KA_WARNING_PUSH() KA_PRAGMA_WARNING_ACTION(push)
#endif

/// `KA_WARNING_POP()`
///
/// Pops the last state of warning pragma directives that was previously pushed
/// with `KA_WARNING_PUSH()`.
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_WARNING_POP
#  define KA_WARNING_POP() KA_PRAGMA_WARNING_ACTION(pop)
#endif

/// `KA_WARNING_DISABLE(msvc_code, gnuc_name)`
///
/// Disables the warning that is identified by `msvc_code` under MSVC or
/// `gnuc_name` under GNUC and Clang. Any parameter can be omitted (i.e. can be
/// left empty). Warning disabling is done only on compilers corresponding to
/// specified parameters.
///
/// Example: Disabling deprecated warnings for the rest of the file or until the
/// next call to `KA_WARNING_POP()`.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// KA_WARNING_DISABLE(4996, deprecated-declarations)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Disabling warnings about "decorated name length exceeded" on MSVC
/// only.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// KA_WARNING_DISABLE(4503,) // no equivalent on GNUC or Clang.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_WARNING_DISABLE
#  if BOOST_COMP_MSVC
#    define KA_WARNING_DISABLE(code, _)          \
       BOOST_PP_EXPR_IF(                         \
         BOOST_PP_NOT(BOOST_PP_IS_EMPTY(code)),  \
         KA_PRAGMA_WARNING_ACTION(disable: code) \
       )
#  elif BOOST_COMP_GNUC || BOOST_COMP_CLANG
#    define KA_WARNING_DISABLE(_, name)                                               \
       BOOST_PP_EXPR_IF(                                                              \
         BOOST_PP_NOT(BOOST_PP_IS_EMPTY(name)),                                       \
         KA_PRAGMA_WARNING_ACTION(ignored BOOST_PP_STRINGIZE(BOOST_PP_CAT(-W, name))) \
       )
#  else
#    define KA_WARNING_DISABLE(_1, _2)
#  endif
#endif

/// `KA_WARNING(message)`
///
/// Generates a compiler warning in a compiler specific manner.
///
/// Example: Generating a warning if `int` and `short` have the same size.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// #if INT_MAX == SHORT_MAX
///   KA_WARNING("`int` and `short` have the same size.")
/// #endif
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Note: It is possible to override the definition of this macro by defining it
/// before the first inclusion of `ka/macro.hpp`.
#ifndef KA_WARNING
#  if BOOST_COMP_MSVC
#    define KA_WARNING(msg) KA_PRAGMA_MESSAGE("warning: " msg)
#  elif BOOST_COMP_GNUC || BOOST_COMP_CLANG
     // clang supports GCC diagnostic syntax.
#    define KA_WARNING(msg) KA_PRAGMA(GCC warning msg)
#  else
#    define KA_WARNING(_)
#  endif
#endif

#endif // KA_MACRO_HPP
