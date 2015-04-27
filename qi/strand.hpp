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

  void post(const boost::function<void()>& callback);

  qi::Future<void> async(const boost::function<void()>& cb,
      qi::SteadyClockTimePoint tp);
  qi::Future<void> async(const boost::function<void()>& cb,
      qi::Duration delay = qi::Duration(0));

  bool isInThisContext();

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                    \
  template <typename T, typename F, typename ARG0 comma ATYPEDECL>           \
  boost::function<T> schedulerFor(                                           \
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

private:
  boost::shared_ptr<StrandPrivate> _p;

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
};

}

#endif  // _QI_STRAND_HPP_
