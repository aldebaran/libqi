#pragma once
/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_DETAIL_TRACKABLE_HXX_
#define _QI_DETAIL_TRACKABLE_HXX_

#include <type_traits>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/function_types/result_type.hpp>

namespace qi
{
  template<typename T>
  inline Trackable<T>::Trackable()
  : _wasDestroyed(false)
  {
    T* thisAsT = static_cast<T*>(this);
    _ptr = boost::shared_ptr<T>(thisAsT, boost::bind(&Trackable::_destroyed, _1));
  }

  template<typename T>
  inline Trackable<T>::Trackable(T* ptr)
    : _wasDestroyed(false)
  {
    _ptr = boost::shared_ptr<T>(ptr, boost::bind(&Trackable::_destroyed, _1));
  }

  template<typename T>
  inline void Trackable<T>::destroy()
  {
    _ptr.reset();
    wait();
  }

  template<typename T>
  inline void Trackable<T>::wait()
  {
    boost::mutex::scoped_lock lock(_mutex);
    while (!_wasDestroyed)
    {
      _cond.wait(lock);
    }
  }

  template<typename T>
  inline void Trackable<T>::_destroyed()
  {
    // unblock
    boost::mutex::scoped_lock lock(_mutex);
    _wasDestroyed = true;
    _cond.notify_all();
  }

  template<typename T>
  inline Trackable<T>::~Trackable()
  {
    // We expect destroy() to have been called from parent destructor, so from
    // this thread.
    if (!_wasDestroyed)
    {
      qiLogError("qi.Trackable") << "Trackable destroyed without calling destroy()";
      // do it to mitigate the effect, but it's too late
      destroy();
    }
  }

  template<typename T>
  inline boost::weak_ptr<T> Trackable<T>::weakPtr()
  {
    return boost::weak_ptr<T>(_ptr);
  }

  namespace detail
  {
    template <typename T>
    T defaultConstruct()
    {
      return {};
    }

    template <>
    inline void defaultConstruct<void>()
    {
    }

    // Functor that locks a weak ptr and make a call if successful
    // Generalize on shared_ptr and weak_ptr types in case we ever have
    // other types with their semantics
    template<typename WT, typename F>
    class LockAndCall
    {
      WT _wptr;
      F _f;
      boost::function<void()> _onFail;

    public:
      LockAndCall(const WT& arg, F func, boost::function<void()> onFail)
        : _wptr(arg)
        , _f(std::move(func))
        , _onFail(onFail)
      {}

      template <typename... Args>
      // decltype(this->_f(std::forward<Args>(args)...)) does not work on vs2013 \o/
      auto operator()(Args&&... args) const -> decltype(std::declval<F>()(std::forward<Args>(args)...))
      {
        auto s = _wptr.lock();
        if (s)
          return _f(std::forward<Args>(args)...);
        else
        {
          if (_onFail)
            _onFail();
          // hehe, you can't write return {}; because of void here... -_-
          return defaultConstruct<decltype(this->_f(std::forward<Args>(args)...))>();
        }
      }
    };

    template<typename T, bool IS_TRACKABLE>
    struct BindTransformImpl
    {
      typedef const T& type;
      static type transform(const T& arg)
      {
        return arg;
      }
      template<typename F>
      using wrap_type = typename std::decay<F>::type;
      template<typename F>
      static wrap_type<F> wrap(const T& arg, F&& func, boost::function<void()> onFail)
      {
        return std::forward<F>(func);
      }
    };

    template<typename T>
    struct BindTransformImpl<boost::weak_ptr<T>, false>
    {
      typedef T* type;
      static T* transform(const boost::weak_ptr<T>& arg)
      {
        // Note that we assume that lock if successful always return the same pointer
        // And that if lock fails once, it will fail forever from that point
        return arg.lock().get();
      }
      template<typename F>
      using wrap_type = LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>;
      template<typename F>
      static wrap_type<F> wrap(
          const boost::weak_ptr<T>& arg, F&& func, boost::function<void()> onFail)
      {
        return LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>(arg, std::forward<F>(func), onFail);
      }
    };

    template<typename T>
    struct BindTransformImpl<T*, true>
    {
      typedef T* type;
      static T* transform(T* const & arg)
      {
        // Note that we assume that lock if successful always return the same pointer
        return arg;
      }
      template<typename F>
      using wrap_type = LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>;
      template<typename F>
      static wrap_type<F> wrap(
          T*const & arg, F&& func, boost::function<void()> onFail)
      {
        return
          LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>(arg->weakPtr(), std::forward<F>(func), onFail);
      }
    };

    template <typename T, typename K = typename std::decay<T>::type>
    using BindTransform =
      BindTransformImpl<K, std::is_base_of<TrackableBase, typename std::remove_pointer<K>::type>::value>;

    inline void throwPointerLockException()
    {
      throw PointerLockException();
    }
  }

  template <typename RF, typename AF, typename Arg0, typename... Args>
  boost::function<RF> bindWithFallback(boost::function<void()> onFail,
                                       AF&& fun,
                                       Arg0&& arg0,
                                       Args&&... args)
  {
    using Transform = detail::BindTransform<Arg0>;
    typename Transform::type transformed = Transform::transform(arg0);
    boost::function<RF> f = boost::bind(std::forward<AF>(fun), std::move(transformed), std::forward<Args>(args)...);
    return Transform::wrap(arg0, std::move(f), std::move(onFail));
  }
  template <typename RF, typename AF, typename Arg0, typename... Args>
  boost::function<RF> bindSilent(AF&& fun, Arg0&& arg0, Args&&... args)
  {
    return bindWithFallback<RF, AF>({}, std::forward<AF>(fun), std::forward<Arg0>(arg0), std::forward<Args>(args)...);
  }
  template <typename RF, typename AF, typename Arg0, typename... Args>
  boost::function<RF> bind(AF&& fun, Arg0&& arg0, Args&&... args)
  {
    return bindWithFallback<RF, AF>(detail::throwPointerLockException, std::forward<AF>(fun), std::forward<Arg0>(arg0),
        std::forward<Args>(args)...);
  }

  template <typename AF, typename Arg0, typename... Args>
  auto bindWithFallback(boost::function<void()> onFail, AF&& fun, Arg0&& arg0, Args&&... args) ->
      typename detail::BindTransform<Arg0>::template wrap_type<
          decltype(boost::bind(std::forward<AF>(fun),
                               detail::BindTransform<Arg0>::transform(arg0),
                               std::forward<Args>(args)...))>
  {
    using Transform = detail::BindTransform<Arg0>;
    typename Transform::type transformed = Transform::transform(arg0);
    return Transform::wrap(arg0,
                           boost::bind(std::forward<AF>(fun), std::move(transformed), std::forward<Args>(args)...),
                           std::move(onFail));
  }
  template <typename AF, typename Arg0, typename... Args>
  auto bindSilent(AF&& fun, Arg0&& arg0, Args&&... args)
      -> decltype(bindWithFallback({}, std::forward<AF>(fun), std::forward<Arg0>(arg0), std::forward<Args>(args)...))
  {
    return bindWithFallback({}, std::forward<AF>(fun), std::forward<Arg0>(arg0), std::forward<Args>(args)...);
  }
  template <typename AF, typename Arg0, typename... Args>
  auto bind(AF&& fun, Arg0&& arg0, Args&&... args) -> decltype(bindWithFallback(detail::throwPointerLockException,
                                                                                std::forward<AF>(fun),
                                                                                std::forward<Arg0>(arg0),
                                                                                std::forward<Args>(args)...))
  {
    return bindWithFallback(detail::throwPointerLockException,
                            std::forward<AF>(fun),
                            std::forward<Arg0>(arg0),
                            std::forward<Args>(args)...);
  }

  template<typename F, typename ARG0>
  boost::function<F> track(const boost::function<F>& f, const ARG0& arg0)
  {
    using Transform = detail::BindTransform<ARG0>;
    return Transform::wrap(arg0, f, detail::throwPointerLockException);
  }
  template<typename F, typename ARG0>
  boost::function<F> trackWithFallback(boost::function<void()> onFail,
      const boost::function<F>& f, const ARG0& arg0)
  {
    using Transform = detail::BindTransform<ARG0>;
    return Transform::wrap(arg0, f, onFail);
  }
  template<typename F, typename ARG0>
  boost::function<F> trackSilent(const boost::function<F>& f, const ARG0& arg0)
  {
    return trackWithFallback<F, ARG0>(boost::function<void()>(), f, arg0);
  }
}

#endif  // _QI_DETAIL_TRACKABLE_HXX_
