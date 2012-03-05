/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once

#ifndef _LIBQI_QI_ATOMIC_HPP_
#define _LIBQI_QI_ATOMIC_HPP_

namespace qi
{
  template <typename T>
  class atomic
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
  short atomic<short>::operator++()
  {
    return _InterlockedIncrement16(&_value);
  }

  template<>
  short atomic<short>::operator--()
  {
    return _InterlockedDecrement16(&_value);
  }

  template <>
  long atomic<long>::operator++()
  {
    return _InterlockedIncrement(&_value);
  }

  template <>
  long atomic<long>::operator--()
  {
    return _InterlockedDecrement(&_value);
  }
#endif
}

#endif // _LIBQI_QI_ATOMIC_HPP_
