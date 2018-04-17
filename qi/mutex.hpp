/*
**  Copyright (C) 2018 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_MUTEX_HPP
#define QI_MUTEX_HPP

#pragma once

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <mutex>
#include <qi/type/traits.hpp>


namespace qi
{
namespace traits
{
  namespace detail
  {
    template<typename M> struct IsMutex : False {};

    // Common mutexes that might be used in this library.
    template<> struct IsMutex<std::mutex>                   : True {};
    template<> struct IsMutex<std::recursive_mutex>         : True {};
#if !BOOST_OS_ANDROID
    template<> struct IsMutex<std::timed_mutex>             : True {};
    template<> struct IsMutex<std::recursive_timed_mutex>   : True {};
#endif
    template<> struct IsMutex<boost::mutex>                 : True {};
    template<> struct IsMutex<boost::recursive_mutex>       : True {};
    template<> struct IsMutex<boost::timed_mutex>           : True {};
    template<> struct IsMutex<boost::recursive_timed_mutex> : True {};
    template<> struct IsMutex<boost::shared_mutex>          : True {};
  }

  /// True if the T is one of the commonly used mutex type:
  /// std::mutex, std::recursive_mutex, std::timed_mutex, std::recursive_timed_mutex, boost::mutex
  /// boost::recursive_mutex, boost::timed_mutex, boost::recursive_timed_mutex, boost::shared_mutex
  template<typename T>
  using IsMutex = typename detail::IsMutex<T>::type;
}
}

namespace std
{
  /// model ScopeLockable M:
  /// Mutex M
  template<typename M, typename = qi::traits::EnableIf<qi::traits::IsMutex<M>::value>>
  std::unique_lock<M> scopelock(M& m)
  {
    return std::unique_lock<M>{ m };
  }
}

namespace boost
{
  /// model ScopeLockable M:
  /// Mutex M
  template<typename M, typename = qi::traits::EnableIf<qi::traits::IsMutex<M>::value>>
  std::unique_lock<M> scopelock(M& m) // std::unique_lock works for boost mutexes
  {
    return std::unique_lock<M>{ m };
  }

  /// model ScopeLockable boost::synchronized_value<T>:
  template<typename T>
  auto scopelock(synchronized_value<T>& syncVal)
    -> decltype(syncVal.synchronize()) // TODO: Remove this when we can use C++14
  {
    return syncVal.synchronize();
  }
}

#endif // QI_MUTEX_HPP
