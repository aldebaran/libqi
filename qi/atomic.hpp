#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */


#ifndef _LIBQI_QI_ATOMIC_HPP_
#define _LIBQI_QI_ATOMIC_HPP_

#ifdef _MSC_VER
# include <windows.h>

extern "C" long __cdecl _InterlockedIncrement(long volatile *);
extern "C" long __cdecl _InterlockedDecrement(long volatile *);

# pragma intrinsic(_InterlockedIncrement)
# pragma intrinsic(_InterlockedDecrement)

#endif

#include <boost/static_assert.hpp>

#include <qi/config.hpp>
#include <qi/macro.hpp>

namespace qi
{
  inline long testAndSet(long* cond)
  {
#ifdef __GNUC__
    return __sync_bool_compare_and_swap(cond, 0, 1);
#endif
#ifdef _MSC_VER
    return 1 - InterlockedCompareExchange(cond, 1, 0);
#endif
  }

  template <typename T>
  class Atomic
  {
  public:
    Atomic()
      : _value(0)
    {
    }

    Atomic(T value)
      : _value(value)
    {
    }

    /* prefix operators */
    inline T operator++();
    inline T operator--();
    inline Atomic<T>& operator=(T value);
    inline T swap(T value);

    inline T operator*()
    {
      return _value;
    }

  private:
    BOOST_STATIC_ASSERT_MSG(sizeof(T) == sizeof(int), "qi::Atomic is only supprted for int-like types");

    T _value;
  };

#ifdef __GNUC__
    template <typename T>
    inline T Atomic<T>::operator++()
    {
      return __sync_add_and_fetch(&_value, 1);
    }

    template <typename T>
    inline T Atomic<T>::operator--()
    {
      return __sync_sub_and_fetch(&_value, 1);
    }

    template <typename T>
    inline Atomic<T>& Atomic<T>::operator=(T value)
    {
      __sync_lock_test_and_set(&_value, value);
      return *this;
    }

    template <typename T>
    inline T Atomic<T>::swap(T value)
    {
      return __sync_lock_test_and_set(&_value, value);
    }
#endif

#ifdef _MSC_VER

  template <>
  inline int Atomic<int>::operator++()
  {
    return _InterlockedIncrement(reinterpret_cast<long*>(&_value));
  }

  template <>
  inline int Atomic<int>::operator--()
  {
    return _InterlockedDecrement(reinterpret_cast<long*>(&_value));
  }

  template<>
  inline Atomic<int>& Atomic<int>::operator=(int value)
  {
    InterlockedExchange(reinterpret_cast<long*>(&_value), value);
    return *this;
  }

  template<>
  inline int Atomic<int>::swap(int value)
  {
    return InterlockedExchange(reinterpret_cast<long*>(&_value), value);
  }

  template <>
  inline unsigned int Atomic<unsigned int>::operator++()
  {
    return _InterlockedIncrement(reinterpret_cast<long*>(&_value));
  }

  template <>
  inline unsigned int Atomic<unsigned int>::operator--()
  {
    return _InterlockedDecrement(reinterpret_cast<long*>(&_value));
  }

  template<>
  inline Atomic<unsigned int>& Atomic<unsigned int>::operator=(unsigned int value)
  {
    InterlockedExchange(reinterpret_cast<long*>(&_value), value);
    return *this;
  }

  template<>
  inline unsigned int Atomic<unsigned int>::swap(unsigned int value)
  {
    return InterlockedExchange(reinterpret_cast<long*>(&_value), value);
  }

#endif

  template<typename T>
  QI_API_DEPRECATED class atomic : public Atomic<T>
  {
  public:
    atomic() : Atomic<T>() {}
    atomic(T value) : Atomic<T>(value) {}
  };
}

#endif // _LIBQI_QI_ATOMIC_HPP_
