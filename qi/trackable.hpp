#pragma once
/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */


#ifndef _QI_TRACKABLE_HPP_
# define _QI_TRACKABLE_HPP_

# include <boost/thread/mutex.hpp>
# include <boost/shared_ptr.hpp>
# include <boost/thread/condition_variable.hpp>
# include <boost/function.hpp>

# include <ka/typetraits.hpp>
# include <qi/macro.hpp>
# include <ka/macroregular.hpp>
# include <qi/log.hpp>

namespace qi
{

  /// Common base class to templates Trackable for compile-time detection.
  class TrackableBase {};

  /**
   * \brief Object tracking by blocking destruction while shared pointers are present.
   *
   * \includename{qi/trackable.hpp}
   * Inherit from Trackable to allow a form of tracking that blocks destruction
   * while shared pointers are held. This allows using your class without a
   * shared_ptr wrapper.
   *
   * \warning when inheriting from this class, you *must* invoke the destroy()
   * method from your destructor, before any operation that puts your object in
   * an invalid state.
   *
   * \warning since destroy() blocks until all shared pointers are destroyed,
   * deadlocks may occur if used improperly.
   *
   */
  template<typename T>
  class Trackable: public TrackableBase
  {
  public:
    /// Default constructor
    Trackable();
    /// @deprecated Not required anymore, use the default constructor instead.
    QI_API_DEPRECATED_MSG(Use default constructor instead) Trackable(T* ptr);
    ~Trackable();

    /**
     * \return A weak_ptr from this. While a shared_ptr exists from this weak_ptr,
     *         a call to destroy will block()
     *
     */
    boost::weak_ptr<T> weakPtr() const;

    /**
     * Blocks until destroy() is called and all shared_ptr built from weak_ptr()
     * are deleted.
     */
    void wait();
  protected:
    /**
     * *Must* be called by parent class destructor, first thing.
     * Can block until lock holders terminate
     */
    void destroy();
  private:
    void _destroyed();

    boost::shared_ptr<T>      _ptr;
    boost::condition_variable _cond;
    boost::mutex              _mutex;
    bool                      _wasDestroyed;
  };

  class QI_API PointerLockException: public std::exception
  {
  public:
    virtual const char* what() const throw()
    {
      return "Pointer Lock failed";
    }
  };

#ifdef DOXYGEN
  /** Bind a set of arguments or placeholders to a function.
   *
   * Handles first function argument of kind boost::weak_ptr and qi::Trackable:
   * will try to lock and throw qi::PointerLockException in case of failure
   */
  template<typename RF, typename AF> boost::function<RF> bind(const AF& fun, ...);
#endif

  /**
   * \brief Wrap given function f with a tracking check on arg0, which must
   *        be a weak pointer or a Trackable instance.
   * \return a function that, when called:
   *   - If lock can be acquired, calls f
   *   - Else throws qi::PointerLockException
   */
  template<typename F, typename ARG0>
  boost::function<F> track(boost::function<F> f, const ARG0& arg0);
  /**
   * \brief Wrap given function f with a tracking check on arg0, which must
   *        be a weak pointer or a Trackable instance.
   * \return a function that, when called:
   *   - If lock can be acquired, calls f
   *   - Else calls onFail
   */
  template<typename F, typename ARG0>
  boost::function<F> trackWithFallback(boost::function<void()> onFail, boost::function<F> f, const ARG0& arg0);

  /// A polymorphic transformation that takes a procedure and returns a
  /// "tracked with fallback" equivalent.
  /// The trackable value must derive from Trackable.
  ///
  /// Procedure<void ()> Proc, Trackable T
  template<typename Proc, typename T>
  struct TrackWithFallbackTransfo
  {
    Proc _fallback;
    T* _trackable;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_2(TrackWithFallbackTransfo, _fallback, _trackable)
  // PolymorphicTransformation:
    template<typename F>
    auto operator()(F&& f) const -> decltype(
      trackWithFallback(_fallback, std::forward<F>(f), _trackable))
    {
      return trackWithFallback(_fallback, std::forward<F>(f), _trackable);
    }
  };

  /// Helper to deduce types to construct a TrackWithFallbackTransfo.
  ///
  /// Procedure<void ()> Proc, Trackable T
  template<typename Proc, typename T>
  TrackWithFallbackTransfo<ka::Decay<Proc>, T> trackWithFallbackTransfo(Proc&& fallback, T* t)
  {
    return {std::forward<Proc>(fallback), t};
  }

  /// A polymorphic transformation that takes a procedure and returns a
  /// "tracked silent" equivalent.
  /// The trackable value must derive from Trackable.
  ///
  /// Trackable T
  template<typename T>
  struct TrackSilentTransfo
  {
    T* _trackable;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(TrackSilentTransfo, _trackable)
  // PolymorphicTransformation:
    template<typename F>
    auto operator()(F&& f) const -> decltype(
      trackSilent(std::forward<F>(f), _trackable))
    {
      return trackSilent(std::forward<F>(f), _trackable);
    }
  };

  /// Helper to deduce types to construct a TrackSilentTransfo.
  ///
  /// Trackable T
  template<typename T>
  TrackSilentTransfo<T> trackSilentTransfo(T* t)
  {
    return {t};
  }
}

# include <qi/detail/trackable.hxx>
#endif  // _QI_TRACKABLE_HPP_
