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

#endif // KA_MACRO_HPP
