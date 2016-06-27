#pragma once
/**
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_FUTUREUNWRAP_HPP_
#define _QI_FUTUREUNWRAP_HPP_

#include <type_traits>
#include <qi/macro.hpp>

namespace qi
{
namespace detail
{
  template <typename T>
  T tryUnwrap(T anything)
  {
    return anything;
  }

  template <typename T>
  Future<T> tryUnwrap(Future<Future<T>> future)
  {
    return future.unwrap();
  }

  template <typename T>
  QI_API_DEPRECATED_MSG('last integer argument is deprecated')
  auto tryUnwrap(T&& anything, int) -> decltype(tryUnwrap(std::forward<T>(anything)))
  {
    return tryUnwrap(std::forward<T>(anything));
  }
} // detail
} // qi

#endif // _QI_FUTUREUNWRAP_HPP_

