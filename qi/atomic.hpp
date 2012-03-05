/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

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

#ifdef __GNUC__
    /* prefix */
    T operator++()
    {
      return __sync_add_and_fetch(&_value, 1);
    }

    T operator--()
    {
      return __sync_sub_and_fetch(&_value, 1);
    }
#endif

    T operator*()
    {
      return _value;
    }

  private:
    T _value;
  };

#ifdef MSVC
  template <>
  long atomic<long>::operator++()
  {
    return _InterlockedIncrement(&_value);
  }

  template<>
  short atomic<short>::operator++()
  {
    return _InterlockedIncrement16(&_value);
  }

  template<>
  __int64 atomic<__int64>::operator++()
  {
    return _InterlockedIncrement64(&_value);
  }

  template <>
  long atomic<long>::operator--()
  {
    return _InterlockedDecrement(&_value);
  }

  template<>
  short atomic<short>::operator--()
  {
    return _InterlockedDecrement16(&_value);
  }

  template<>
  __int64 atomic<__int64>::operator--()
  {
    return _InterlockedDecrement64(&_value);
  }
#endif
}
