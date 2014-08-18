#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef QI_ATOMIC_HPP_
#define QI_ATOMIC_HPP_

#ifdef _MSC_VER

#include <windows.h>
#include <intrin.h>

extern "C" long __cdecl _InterlockedIncrement(long volatile *);
extern "C" long __cdecl _InterlockedDecrement(long volatile *);

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)

#endif

#include <boost/atomic.hpp>
#include <qi/config.hpp>
#include <qi/macro.hpp>

namespace qi
{

inline long testAndSet(long* cond)
{
#if defined __GNUC__
  return __sync_bool_compare_and_swap(cond, 0, 1);
#elif defined _MSC_VER
  return 1 - InterlockedCompareExchange(cond, 1, 0);
#else
  #error "Unknown platform, testAndSet not implemented"
#endif
}

/* /!\ WARNING
 * The 'volatile' is needed even though we use atomic compiler builtins.
 * Without the volatile, a thread doing
 *    while (!setIfEquals(1,1))
 * Is never unstuck by a thread doing
 *    setIfEquals(0,1)
 *
 * StaticAtomicInt has public member so that it can be initialized at
 * static-initialization time (to make thread-safe static initialization
 * inside functions)
 */
struct StaticAtomicInt
{
public:
  /* prefix operators */
  inline int operator++();
  inline int operator--();
  inline StaticAtomicInt& operator=(int value);
  /** If value is \p testValue, replace it with \p setValue.
   * \return true if swap was performed
   */
  inline bool setIfEquals(int testValue, int setValue);

  inline int swap(int value);

  inline int operator*() const
  {
    return _value;
  }

public:
  volatile
#ifdef _MSC_VER
  long
#else
  int
#endif
     _value;
};

#ifdef __GNUC__
inline int StaticAtomicInt::operator++()
{
  return __sync_add_and_fetch(&_value, 1);
}
inline int StaticAtomicInt::operator--()
{
  return __sync_sub_and_fetch(&_value, 1);
}
inline StaticAtomicInt& StaticAtomicInt::operator=(int value)
{
  __sync_lock_test_and_set(&_value, value);
  return *this;
}
inline int StaticAtomicInt::swap(int value)
{
  return __sync_lock_test_and_set(&_value, value);
}
inline bool StaticAtomicInt::setIfEquals(int testValue, int setValue)
{
  return __sync_bool_compare_and_swap(&_value, testValue, setValue);
}
#elif defined(_MSC_VER)
inline int StaticAtomicInt::operator++()
{
  return _InterlockedIncrement(&_value);
}
inline int StaticAtomicInt::operator--()
{
  return _InterlockedDecrement(&_value);
}
inline StaticAtomicInt& StaticAtomicInt::operator=(int value)
{
  InterlockedExchange(&_value, value);
  return *this;
}
inline int StaticAtomicInt::swap(int value)
{
  return InterlockedExchange(&_value, value);
}
inline bool StaticAtomicInt::setIfEquals(int testValue, int setValue)
{
  return _InterlockedCompareExchange(&_value, setValue, testValue) == testValue;
}
#endif

template <typename T>
struct Atomic
{
public:
  boost::atomic<T> _value;

  Atomic()
    : _value(0)
  {}
  Atomic(T value)
    : _value(value)
  {}
  // This is needed in c++03 for lines like:
  // Atomic<int> i = 0;
  // There is no copy there, but the constructor *must* exist
  Atomic(const Atomic& other)
    : _value(*other)
  {}

  /* prefix operators */
  T operator++()
  { return ++_value; }
  T operator--()
  { return --_value; }

  Atomic<T>& operator=(T value)
  { _value = value; return *this; }
  Atomic<T>& operator=(const Atomic<T>& value)
  { _value = *value; return *this; }

  /** If value is testValue, replace it with setValue.
   * \return true if swap was performed
   */
  bool setIfEquals(T testValue, T setValue)
  { return _value.compare_exchange_strong(testValue, setValue); }

  T swap(T value)
  { return _value.exchange(value); }

  T operator*() const
  { return _value.load(); }
};

namespace detail
{
template<typename T> void newAndAssign(T** ptr)
{
  *ptr = new T();
}
}

}

#define _QI_INSTANCIATE(_, a, elem) ::qi::detail::newAndAssign(&elem);

/* The code below relies on the fact that initialisation of the qi::Atomic
 * can happen at static initialization time, and that proper memory barriers
 * are setup by its ++, swap and get operations.
 */

/** Accept a list of pointers (expected to be static function variables)
 * and new them once in a thread-safe manner.
 * Implementation aims for minimal overhead when initialization is done.
 */
#define QI_THREADSAFE_NEW(...)  \
  QI_ONCE(QI_VAARGS_APPLY(_QI_INSTANCIATE, _, __VA_ARGS__);)

/// Execute code once, parallel calls are blocked until code finishes.
#define QI_ONCE(code)                                           \
  static qi::StaticAtomicInt QI_UNIQ_DEF(atomic_guard_a) = {0}; \
  static qi::StaticAtomicInt QI_UNIQ_DEF(atomic_guard_b) = {0}; \
  while (!QI_UNIQ_DEF(atomic_guard_a).setIfEquals(1, 1))        \
  {                                                             \
    bool tok = QI_UNIQ_DEF(atomic_guard_b).setIfEquals(0, 1);   \
    if (tok)                                                    \
    {                                                           \
      code;                                                     \
      ++QI_UNIQ_DEF(atomic_guard_a);                            \
    }                                                           \
  }

#endif // QI_ATOMIC_HPP_
