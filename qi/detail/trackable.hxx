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
  class Actor;

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
    template <typename WT, typename F>
    class LockAndCall
    {
      WT _wptr;
      F _f;
      boost::function<void()> _onFail;

    public:
      LockAndCall(WT arg, F func, boost::function<void()> onFail)
        : _wptr(std::move(arg))
        , _f(std::move(func))
        , _onFail(std::move(onFail))
      {}

      template <typename... Args>
      // decltype(this->_f(std::forward<Args>(args)...)) does not work on vs2013 \o/
      auto operator()(Args&&... args) -> decltype(std::declval<F>()(std::forward<Args>(args)...))
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

    template <typename T, bool IsActor>
    struct ObjectWrap;

    template <typename T>
    struct ObjectWrap<T, false>
    {
      template <typename F>
      using wrap_type = typename std::decay<F>::type;
      template <typename F>
      static wrap_type<F> wrap(const T& arg, F&& func, boost::function<void()> onFail)
      {
        return std::forward<F>(func);
      }
    };

    template <typename T>
    struct ObjectWrap<T, true>
    {
      static const bool is_async = true;
      template <typename F>
      using wrap_type = decltype(
          std::declval<T>()->strand()->schedulerFor(std::declval<typename std::decay<F>::type>()));
      template <typename F>
      static wrap_type<F> wrap(const T& arg, F&& func, boost::function<void()> onFail)
      {
        return arg->strand()->schedulerFor(std::forward<F>(func), std::move(onFail));
      }
    };

    struct IsAsyncBindImpl
    {
      struct ArbitraryBigBuf
      {
        char arbitrary_buf[128];
      };
      template <typename T>
      static decltype(T::is_async) f(int);
      template <typename T>
      static ArbitraryBigBuf f(void*);
    };

    // can't use a "using" here because visual gets the SFINAE wrong in the conditional (lol.)
    template <typename T>
    struct IsAsyncBind
        : std::conditional<sizeof(decltype(IsAsyncBindImpl::template f<T>(0))) != sizeof(IsAsyncBindImpl::ArbitraryBigBuf),
                           std::true_type,
                           std::false_type>::type
    {
    };

    template <bool IsAsync, typename F, typename... Args>
    struct DecayAsyncResultImpl;

    template <typename F, typename... Args>
    struct DecayAsyncResultImpl<false, F, Args...>
    {
      using type = decltype(std::declval<F>()(std::declval<Args>()...));
    };

    template <typename F, typename... Args>
    struct DecayAsyncResultImpl<true, F, Args...>
    {
      // I'd like to use a decltype, but vs2013 fails to parse this
      //using type = typename decltype(std::declval<F>()(std::declval<Args>()...))::TemplateValue;
      using type = typename std::result_of<F(Args...)>::type::TemplateValue;
    };

    template <typename T, typename... Args>
    using DecayAsyncResult = DecayAsyncResultImpl<IsAsyncBind<typename std::decay<T>::type>::value, T, Args...>;

    template <typename T, bool IsTrackable>
    struct BindTransformImpl
    {
      static T transform(T arg)
      {
        return arg;
      }
      template <typename F>
      using wrap_type = typename std::decay<F>::type;
      template <typename F>
      static wrap_type<F> wrap(T arg, F&& func, boost::function<void()> onFail)
      {
        return std::forward<F>(func);
      }
    };

    template <typename T>
    struct BindTransformImpl<T*, false>
    {
      using ObjectWrapType = ObjectWrap<T*, std::is_base_of<Actor, T>::value>;
      template <typename F>
      using wrap_type = typename ObjectWrapType::template wrap_type<F>;

      static T* transform(T* arg)
      {
        return arg;
      }

      template <typename F>
      static wrap_type<F> wrap(T* arg, F&& func, boost::function<void()> onFail)
      {
        return ObjectWrapType::wrap(arg, std::forward<F>(func), onFail);
      }
    };

    template <typename T>
    struct BindTransformImpl<T*, true>
    {
      template <typename F>
      using wrap_type = LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>;

      static T* transform(T* arg)
      {
        // Note that we assume that lock if successful always return the same pointer
        return arg;
      }

      template <typename F>
      static wrap_type<F> wrap(T* arg, F&& func, boost::function<void()> onFail)
      {
        return LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>(
            arg->weakPtr(),
            std::forward<F>(func),
            onFail);
      }
    };

    template <typename T>
    struct BindTransformImpl<boost::weak_ptr<T>, false>
    {
      template <typename F>
      using wrap_type = LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>;

      static T* transform(const boost::weak_ptr<T>& arg)
      {
        // Note that we assume that lock if successful always return the same pointer
        // And that if lock fails once, it will fail forever from that point
        return arg.lock().get();
      }

      template <typename F>
      static wrap_type<F> wrap(const boost::weak_ptr<T>& arg, F&& func, boost::function<void()> onFail)
      {
        return LockAndCall<boost::weak_ptr<T>, typename std::decay<F>::type>(
            arg,
            std::forward<F>(func),
            onFail);
      }
    };

    template <typename T>
    struct BindTransformImpl<boost::shared_ptr<T>, false>
    {
      using ObjectWrapType = ObjectWrap<boost::shared_ptr<T>, std::is_base_of<Actor, T>::value>;
      template <typename F>
      using wrap_type = typename ObjectWrapType::template wrap_type<F>;

      static boost::shared_ptr<T> transform(boost::shared_ptr<T> arg)
      {
        return std::move(arg);
      }

      template <typename F>
      static wrap_type<F> wrap(const boost::shared_ptr<T>& arg, F&& func, boost::function<void()> onFail)
      {
        return ObjectWrapType::wrap(arg, std::forward<F>(func), onFail);
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
  QI_API_DEPRECATED typename std::enable_if<std::is_function<RF>::value, boost::function<RF>>::type
  bindWithFallback(boost::function<void()> onFail, AF&& fun, Arg0&& arg0, Args&&... args)
  {
    using Transform = detail::BindTransform<Arg0>;
    auto transformed = Transform::transform(arg0);
    boost::function<RF> f = boost::bind(std::forward<AF>(fun), std::move(transformed), std::forward<Args>(args)...);
    return Transform::wrap(arg0, std::move(f), std::move(onFail));
  }
  template <typename RF, typename AF, typename Arg0, typename... Args>
  QI_API_DEPRECATED typename std::enable_if<std::is_function<RF>::value, boost::function<RF>>::type bindSilent(AF&& fun,
                                                                                             Arg0&& arg0,
                                                                                             Args&&... args)
  {
    return bindWithFallback<RF, AF>({}, std::forward<AF>(fun), std::forward<Arg0>(arg0), std::forward<Args>(args)...);
  }
  template <typename RF, typename AF, typename Arg0, typename... Args>
  QI_API_DEPRECATED typename std::enable_if<std::is_function<RF>::value, boost::function<RF>>::type bind(AF&& fun,
                                                                                       Arg0&& arg0,
                                                                                       Args&&... args)
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
    auto transformed = Transform::transform(arg0);
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

  // with support for R
  template <typename R, typename AF, typename Arg0, typename... Args>
  auto bindWithFallback(boost::function<void()> onFail, AF&& fun, Arg0&& arg0, Args&&... args) ->
      typename std::enable_if<!std::is_function<R>::value,
                              typename detail::BindTransform<Arg0>::template wrap_type<
                                  decltype(boost::bind<R>(std::forward<AF>(fun),
                                                          detail::BindTransform<Arg0>::transform(arg0),
                                                          std::forward<Args>(args)...))>>::type
  {
    using Transform = detail::BindTransform<Arg0>;
    auto transformed = Transform::transform(arg0);
    return Transform::wrap(arg0,
                           boost::bind<R>(std::forward<AF>(fun), std::move(transformed), std::forward<Args>(args)...),
                           std::move(onFail));
  }
  template <typename R, typename AF, typename Arg0, typename... Args>
  auto bindSilent(AF&& fun, Arg0&& arg0, Args&&... args) -> typename std::enable_if<
      !std::is_function<R>::value,
      decltype(
          bindWithFallback<R>({}, std::forward<AF>(fun), std::forward<Arg0>(arg0), std::forward<Args>(args)...))>::type
  {
    return bindWithFallback<R>({}, std::forward<AF>(fun), std::forward<Arg0>(arg0), std::forward<Args>(args)...);
  }
  template <typename R, typename AF, typename Arg0, typename... Args>
  auto bind(AF&& fun, Arg0&& arg0, Args&&... args) ->
      typename std::enable_if<!std::is_function<R>::value,
                              decltype(bindWithFallback<R>(detail::throwPointerLockException,
                                                           std::forward<AF>(fun),
                                                           std::forward<Arg0>(arg0),
                                                           std::forward<Args>(args)...))>::type
  {
    return bindWithFallback<R>(detail::throwPointerLockException,
                               std::forward<AF>(fun),
                               std::forward<Arg0>(arg0),
                               std::forward<Args>(args)...);
  }

  template <typename F, typename Arg0>
  auto trackWithFallback(boost::function<void()> onFail, F&& f, Arg0&& arg0)
      -> decltype(detail::BindTransform<Arg0>::wrap(std::forward<Arg0>(arg0), std::forward<F>(f), std::move(onFail)))
  {
    return detail::BindTransform<Arg0>::wrap(std::forward<Arg0>(arg0), std::forward<F>(f), std::move(onFail));
  }
  template <typename F, typename Arg0>
  auto track(F&& f, Arg0&& arg0)
      -> decltype(trackWithFallback(detail::throwPointerLockException, std::forward<F>(f), std::forward<Arg0>(arg0)))
  {
    return trackWithFallback(detail::throwPointerLockException, std::forward<F>(f), std::forward<Arg0>(arg0));
  }
  template <typename F, typename Arg0>
  auto trackSilent(F&& f, Arg0&& arg0) -> decltype(trackWithFallback({}, std::forward<F>(f), std::forward<Arg0>(arg0)))
  {
    return trackWithFallback({}, std::forward<F>(f), std::forward<Arg0>(arg0));
  }

  template<typename F, typename Arg0>
  boost::function<F> trackWithFallback(boost::function<void()> onFail,
      boost::function<F> f, const Arg0& arg0)
  {
    return detail::BindTransform<Arg0>::wrap(arg0, std::move(f), std::move(onFail));
  }
  template<typename F, typename Arg0>
  boost::function<F> trackSilent(boost::function<F> f, const Arg0& arg0)
  {
    return trackWithFallback<F, Arg0>({}, std::move(f), arg0);
  }
  template<typename F, typename Arg0>
  boost::function<F> track(boost::function<F> f, const Arg0& arg0)
  {
    return trackWithFallback<F, Arg0>(detail::throwPointerLockException, std::move(f), arg0);
  }
}

#endif  // _QI_DETAIL_TRACKABLE_HXX_
