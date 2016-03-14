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

#include <atomic>
#include <qi/config.hpp>
#include <qi/macro.hpp>

namespace qi
{

/** Cross-platform implementation of atomic Test-And-Set.
 * \param cond pointer to the value to test and set.
 * \return true (1) if cond is 0, false (0) otherwise.
 */
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

namespace detail
{
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
}

/** Atomic operations on integrals.
 *
 * This class allows to do operations on an integral value from multiple threads,
 * with the guarantee that each operation will not lead to a data race.
 *
 * @remark This is a simplification layer over the standard atomic type.
 *         If you understand the standard atomic, it might be preferable to use it.
 *
 * \includename{qi/atomic.hpp}
 */
template <typename T>
struct Atomic
{
  std::atomic<T> _value;
public:
  /* Default atomic constructor, setting value to 0.
   */
  Atomic()
    : _value{}
  {}
  /** Atomic constructor setting value to its parameter.
   * \param value The default value of the atomic.
   */
  Atomic(T value)
    : _value(std::move(value))
  {}
  // This is needed in c++03 for lines like:
  // Atomic<int> i = 0;
  // There is no copy there, but the constructor *must* exist
  Atomic(const Atomic& other)
    : _value(other._value.load())
  {}

  /// Atomic pre-increment of the value.
  T operator++()
  { return ++_value; }
  /// Atomic pre-decrement of the value.
  T operator--()
  { return --_value; }

  /// Atomic post-increment of the value.
  T operator++(int)
  {
    return _value++;
  }
  /// Atomic post-decrement of the value.
  T operator--(int)
  {
    return _value--;
  }

  Atomic<T>& operator=(T value)
  { _value = std::move(value); return *this; }
  Atomic<T>& operator=(const Atomic<T>& value)
  { _value = value.load(); return *this; }

  /** If value is testValue, replace it with setValue.
   * \return true if swap was performed
   */
  bool setIfEquals(T testValue, T setValue)
  { return _value.compare_exchange_strong(testValue, setValue); }

  /** Swap the atomic value with value.
   * \return the previously held value
   */
  T swap(T value)
  { return _value.exchange(value); }

  /** Return the contained valu
   * Deprecated since 2.5.0
   */

  QI_API_DEPRECATED_MSG(Use 'load' instead)
  T operator*() const
  { return _value.load(); }

  T load() const
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

/**
 * \def QI_THREADSAFE_NEW
 * \brief Safe static initialization of variables.
 * \verbatim
 * Accept a list of pointers (expected to be static function variables)
 * and new them once in a thread-safe manner.
 * Implementation aims for minimal overhead when initialization is done.
 *
 * `QI_THREADSAFE_NEW` is there to provide a safe static initialization of
 * variables in C++03. Its most common use case is the following:
 *
 * .. code-block:: cpp
 *
 *   static std::vector<int> vec;
 *
 *   void threadSafeFunction()
 *   {
 *     static boost::mutex* mutex; // = 0 is optional
 *     QI_THREADSAFE_NEW(mutex);
 *     boost::mutex::scoped_lock l(*mutex);
 *     vec.push_back(0);
 *   }
 *
 * Using a simple `static boost::mutex` does not guarantee safe initialization in
 * a multithreaded environment in C++03 (even though GCC's implementation is
 * safe), that's why `QI_THREADSAFE_NEW` is needed.
 *
 * In C++11, the following is safe:
 *
 * .. code-block:: cpp
 *
 *   static std::vector<int> vec;
 *
 *   void threadSafeFunction()
 *   {
 *     static boost::mutex mutex;
 *     boost::mutex::scoped_lock l(mutex);
 *     vec.push_back(0);
 *   }
 * \endverbatim
 */

#define QI_THREADSAFE_NEW(...)  \
  QI_ONCE(QI_VAARGS_APPLY(_QI_INSTANCIATE, _, __VA_ARGS__);)

/**
 * \def QI_ONCE
 * \brief Execute code once, parallel calls are blocked until code finishes.
 *
 * \verbatim
 * .. code-block:: cpp
 *
 *   void myFunction()
 *   {
 *     QI_ONCE(std::cout << "first initialization" << std::endl);
 *     std::cout << "doing stuff" << std::endl;
 *   }
 *
 * In this code, you have two guarantees:
 * - "first initialization" will be written only once
 * - "doing stuff" will never appear before "first initialization"
 *
 * `QI_ONCE` is optimized so that further calls after initialization have the less
 * overhead possible.
 *
 * You can also put multiple instructions in a `QI_ONCE`.
 *
 * .. code-block:: cpp
 *
 *   QI_ONCE(
 *       doStuff();
 *       doMoreStuff();
 *       );
 *
 * This macro is only useful in C++03 and the function above may be written in
 * C++11:
 *
 * .. code-block:: cpp
 *
 *   void myFunction()
 *   {
 *     static std::once_flag flag;
 *     std::call_once(flag,
 *         [](){std::cout << "first initialization" << std::endl;});
 *     std::cout << "doing stuff" << std::endl;
 *   }
 * \endverbatim
 */
#define QI_ONCE(code)                                                   \
  static qi::detail::StaticAtomicInt QI_UNIQ_DEF(atomic_guard_a) = {0}; \
  static qi::detail::StaticAtomicInt QI_UNIQ_DEF(atomic_guard_b) = {0}; \
  while (!QI_UNIQ_DEF(atomic_guard_a).setIfEquals(1, 1))                \
  {                                                                     \
    bool tok = QI_UNIQ_DEF(atomic_guard_b).setIfEquals(0, 1);           \
    if (tok)                                                            \
    {                                                                   \
      try                                                               \
      {                                                                 \
        code;                                                           \
      }                                                                 \
      catch (...)                                                       \
      {                                                                 \
        QI_UNIQ_DEF(atomic_guard_b) = 0;                                \
        throw;                                                          \
      }                                                                 \
      ++QI_UNIQ_DEF(atomic_guard_a);                                    \
    }                                                                   \
  }

#endif // QI_ATOMIC_HPP_
