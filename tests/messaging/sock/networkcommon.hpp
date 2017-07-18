#pragma once
#ifndef _QI_TESTS_MESSAGING_NETWORKCOMMON_HPP
#define _QI_TESTS_MESSAGING_NETWORKCOMMON_HPP
#include <algorithm>
#include <random>
#include <boost/lexical_cast.hpp>

/// @file
/// Contains functions and types used by socket tests.

/// Precondition: With p = reinterpret_cast<unsigned char*>(t),
///   writable_counted_range(p, sizeof(T))
template<typename T>
void overwrite(T* t)
{
  auto p = reinterpret_cast<unsigned char*>(t);
  std::fill(p, p + sizeof(T), '\xFF');
}

/// Expects a string with the format:
///   error_code: error_message
/// where error_code is an integer.
inline int code(const std::string& s)
{
  std::string c{begin(s), std::find(begin(s), end(s), ':')};
  return boost::lexical_cast<int>(c);
}

// Distribution D, Generator G, BasicLockable L
template<typename D, typename G, typename L>
auto syncRand(D& dist, G& gen, L& lockable) -> decltype(dist(gen))
{
  std::lock_guard<L> lock{lockable};
  return dist(gen);
}

#endif // _QI_TESTS_MESSAGING_NETWORKCOMMON_HPP
