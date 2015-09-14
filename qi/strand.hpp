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

namespace qi
{

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
  /** Destroys the strand
   *
   * This will wait for all scheduled tasks to finish
   */
  ~Strand();

  // DEPRECATED
  QI_API_DEPRECATED void post(const boost::function<void()>& callback) override;

  QI_API_DEPRECATED qi::Future<void> async(const boost::function<void()>& cb,
      qi::SteadyClockTimePoint tp) override;
  QI_API_DEPRECATED qi::Future<void> async(const boost::function<void()>& cb,
      qi::Duration delay = qi::Duration(0)) override;

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

  bool isInThisContext() override;

  // C++14 this can be a lambda, but we need perfect forwarding in the capture in scheduleFor below
  template <typename F>
  struct SchedulerHelper2
  {
    F _func;
    Strand* _strand;

    SchedulerHelper2(F f, Strand* strand)
      : _func(std::move(f))
      , _strand(strand)
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const -> qi::Future<decltype(std::bind(_func, std::forward<Args>(args)...)())>
    {
      // boost::bind does not work T_T
      return _strand->async2(std::bind(_func, std::forward<Args>(args)...));
    }
  };

  template <typename F>
  SchedulerHelper2<typename std::decay<F>::type> schedulerFor(F&& func)
  {
    return SchedulerHelper2<typename std::decay<F>::type>(std::forward<F>(func), this);
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

}

#endif  // _QI_STRAND_HPP_
