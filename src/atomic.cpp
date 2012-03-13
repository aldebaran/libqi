/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/atomic.hpp>

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

namespace qi
{
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