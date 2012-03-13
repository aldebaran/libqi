/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once

#ifndef _LIBQI_QI_ATOMIC_HPP_
#define _LIBQI_QI_ATOMIC_HPP_

#ifdef _MSC_VER
# include <windows.h>
# pragma intrinsic(_InterlockedIncrement16)
# pragma intrinsic(_InterlockedDecrement16)
# pragma intrinsic(_InterlockedIncrement)
# pragma intrinsic(_InterlockedDecrement)

extern "C" long __cdecl _InterlockedIncrement(long volatile *);
extern "C" long __cdecl _InterlockedDecrement(long volatile *);
extern "C" short __cdecl _InterlockedIncrement16(short volatile *);
extern "C" short __cdecl _InterlockedDecrement16(short volatile *);
#endif

#include <qi/config.hpp>

namespace qi
{
  template <typename T>
  class QI_API atomic
  {
  public:
    atomic()
      : _value(0)
    {
    }

    atomic(T value)
      : _value(value)
    {
    }

    /* prefix operators */
    T operator++();
    T operator--();

    T operator*()
    {
      return _value;
    }

  private:
    T _value;
  };

#ifdef __GNUC__
    template <typename T>
    T atomic<T>::operator++()
    {
      return __sync_add_and_fetch(&_value, 1);
    }

    template <typename T>
    T atomic<T>::operator--()
    {
      return __sync_sub_and_fetch(&_value, 1);
    }
#endif

#ifdef _MSC_VER
  template<>
  inline short atomic<short>::operator++()
  {
    return _InterlockedIncrement16(&_value);
  }

  template<>
  inline short atomic<short>::operator--()
  {
    return _InterlockedDecrement16(&_value);
  }

  template <>
  inline long atomic<long>::operator++()
  {
    return _InterlockedIncrement(&_value);
  }

  template <>
  inline long atomic<long>::operator--()
  {
    return _InterlockedDecrement(&_value);
  }
#endif
}

#endif // _LIBQI_QI_ATOMIC_HPP_
