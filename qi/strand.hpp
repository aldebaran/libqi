#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_STRAND_HPP_
#define _QI_STRAND_HPP_

#include <qi/detail/executioncontext.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/function_traits.hpp>

# ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4996 ) // TODO: Reactivate this warning once msvc stop triggerring a warning on overloading a deprecated function
# endif

namespace qi
{

namespace detail
{

  // C++14 this can be a lambda, but we need perfect forwarding in the capture in scheduleFor below
  template <typename F>
  struct WrapInStrand;

  // this structure is just a wrapper with a is_async field
  template <typename F>
  struct WrapAsAsync;

  template <typename F>
  WrapAsAsync<typename std::decay<F>::type> wrapAsAsync(F&& f);

}

class StrandPrivate;

/** Class that schedules tasks sequentially
 *
 * A strand allows one to schedule work on an eventloop with the guaranty
 * that two callback will never be called concurrently.
 *
 * Methods are thread-safe except for destructor which must never be called
 * concurrently.
 *
 * \includename{qi/strand.hpp}
 */
class QI_API Strand : public ExecutionContext, private boost::noncopyable
{
public:
  /// Construct a strand that will schedule work on the default event loop
  Strand();
  /// Construct a strand that will schedule work on executionContext
  Strand(qi::ExecutionContext& executionContext);
  /// Call detroy()
  ~Strand();

  /** Joins the strand
   *
   * This will wait for currently running tasks to finish and will drop all tasks scheduled from the moment of the call
   * on. A strand can't be reused after it has been join()ed.
   *
   * It is safe to call this method concurrently with other methods. All the returned futures will be set to error.
   */
  void join();

  // DEPRECATED
  QI_API_DEPRECATED qi::Future<void> async(const boost::function<void()>& cb,
      qi::SteadyClockTimePoint tp) override;
  QI_API_DEPRECATED qi::Future<void> async(const boost::function<void()>& cb,
      qi::Duration delay) override;
  using ExecutionContext::async;

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                    \
  template <typename T, typename F, typename ARG0 comma ATYPEDECL>           \
  QI_API_DEPRECATED boost::function<T> schedulerFor(                                           \
      const F& func, const ARG0& arg0 comma ADECL,                           \
      const boost::function<void()>& fallbackCb = boost::function<void()>()) \
  {                                                                          \
    boost::function<T> funcbind = qi::bind<T>(func, arg0 comma AUSE);        \
    return qi::trackWithFallback(                                            \
        fallbackCb,                                                          \
        SchedulerHelper<boost::function_traits<T>::arity, T>::_scheduler(    \
            funcbind, this),                                                 \
        arg0);                                                               \
  }
  QI_GEN(genCall)
#undef genCall
  // END DEPRECATED

  /**
   * \return true if current code is running in this strand, false otherwise. If the strand is dying (destroy() has been
   * called, returns false)
   */
  bool isInThisContext() override;

  template <typename F>
  auto schedulerFor(F&& func, boost::function<void()> onFail = {})
  // very very ugly stuff that deals with qi::bind internals
  // this could just be auto in c++14 and could be decltype in c++11, but vs2013 can't deduce the type here...
      -> detail::WrapAsAsync<typename std::decay<detail::BindTransform<boost::weak_ptr<StrandPrivate>>::wrap_type<
          detail::WrapInStrand<typename std::decay<F>::type>>>::type>
  {
    return detail::wrapAsAsync(
        qi::trackWithFallback(std::move(onFail),
                              detail::WrapInStrand<typename std::decay<F>::type>(std::forward<F>(func), *this),
                              boost::weak_ptr<StrandPrivate>(_p)));
  }

private:
  boost::shared_ptr<StrandPrivate> _p;

  void postImpl(boost::function<void()> callback) override;

  qi::Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp) override;
  qi::Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay) override;

  // DEPRECATED
  template <int N, typename T>
  struct SchedulerHelper;
#define typedefi(z, n, _)                                   \
  typedef typename boost::function_traits<T>::BOOST_PP_CAT( \
      BOOST_PP_CAT(arg, BOOST_PP_INC(n)), _type) BOOST_PP_CAT(P, n);
#define placeholders(z, n, __) , BOOST_PP_CAT(_, BOOST_PP_INC(n))
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                   \
  template <typename T>                                                     \
  struct SchedulerHelper<n, T>                                              \
  {                                                                         \
    BOOST_PP_REPEAT(n, typedefi, _);                                        \
    typedef typename boost::function_traits<T>::result_type R;              \
    static boost::function<T> _scheduler(const boost::function<T>& f,       \
                                         Strand* strand)                    \
    {                                                                       \
      return qi::bind<T>(&_asyncCall, strand,                               \
                         f BOOST_PP_REPEAT(n, placeholders, _));            \
    }                                                                       \
    static qi::Future<R> _asyncCall(Strand* strand,                         \
                                    const boost::function<T>& func comma    \
                                           ADECL)                           \
    {                                                                       \
      /* use qi::bind again since first arg may be a Trackable */           \
      return ((qi::ExecutionContext*)strand)                                \
                  ->async(qi::bind<R()>(func comma AUSE));                  \
    }                                                                       \
  };
  QI_GEN(genCall)
#undef genCall
#undef placeholders
#undef typedefi
  // END DEPRECATED
};

namespace detail
{

  // C++14 this can be a lambda, but we need perfect forwarding in the capture in scheduleFor below
  template <typename F>
  struct WrapInStrand
  {
    F _func;
    Strand& _strand;

    WrapInStrand(F f, Strand& strand)
      : _func(std::move(f))
      , _strand(strand)
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const
        -> qi::Future<typename std::decay<decltype(std::bind(_func, std::forward<Args>(args)...)())>::type>
    {
      // boost::bind does not work T_T
      return _strand.async(std::bind(_func, std::forward<Args>(args)...));
    }
  };

  // this structure is just a wrapper with a is_async field
  template <typename F>
  struct WrapAsAsync
  {
    static const bool is_async = true;

    F _func;

    template <typename... Args>
    auto operator()(Args&&... args) const -> typename std::decay<decltype(_func(std::forward<Args>(args)...))>::type
    {
      return _func(std::forward<Args>(args)...);
    }
  };

  template <typename F>
  WrapAsAsync<typename std::decay<F>::type> wrapAsAsync(F&& f)
  {
    return WrapAsAsync<typename std::decay<F>::type>{std::forward<F>(f)};
  }

}

}

# ifdef _MSC_VER
#  pragma warning( pop )
# endif

#endif  // _QI_STRAND_HPP_
