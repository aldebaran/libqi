#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_STRAND_HPP_
#define _QI_STRAND_HPP_

#include <deque>
#include <atomic>
#include <qi/assert.hpp>
#include <qi/detail/executioncontext.hpp>
#include <boost/enable_shared_from_this.hpp>
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

}

// we use ExecutionContext's helpers in schedulerFor, we don't need to implement all the methods
class StrandPrivate : public ExecutionContext, public boost::enable_shared_from_this<StrandPrivate>
{
public:
  enum class State;

  struct Callback;

  using Queue = std::deque<boost::shared_ptr<Callback>>;

  qi::ExecutionContext& _eventLoop;
  std::atomic<unsigned int> _curId;
  std::atomic<unsigned int> _aliveCount;
  bool _processing; // protected by mutex, no need for atomic
  std::atomic<int> _processingThread;
  boost::mutex _mutex;
  boost::condition_variable _processFinished;
  bool _dying;
  Queue _queue;

  StrandPrivate(qi::ExecutionContext& eventLoop);

  Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp) override;
  Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay) override;

  boost::shared_ptr<Callback> createCallback(boost::function<void()> cb);
  void enqueue(boost::shared_ptr<Callback> cbStruct);

  void process();
  void cancel(boost::shared_ptr<Callback> cbStruct);

  // don't care
  bool isInThisContext() override { QI_ASSERT(false); throw 0; }
  void postImpl(boost::function<void()> callback) override { QI_ASSERT(false); throw 0; }
  qi::Future<void> async(const boost::function<void()>& callback, qi::SteadyClockTimePoint tp) override
  { QI_ASSERT(false); throw 0; }
  qi::Future<void> async(const boost::function<void()>& callback, qi::Duration delay) override
  { QI_ASSERT(false); throw 0; }
  using ExecutionContext::async;
private:
  void stopProcess(boost::mutex::scoped_lock& lock,
                   bool finished);
};

inline StrandPrivate::StrandPrivate(qi::ExecutionContext& eventLoop)
  : _eventLoop(eventLoop)
  , _curId(0)
  , _aliveCount(0)
  , _processing(false)
  , _processingThread(0)
  , _dying(false)
{
}

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
  QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
  qi::Future<void> async(const boost::function<void()>& cb,
                         qi::SteadyClockTimePoint tp) override;
  QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
  qi::Future<void> async(const boost::function<void()>& cb,
                         qi::Duration delay) override;
  using ExecutionContext::async;

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                    \
  template <typename T, typename F, typename ARG0 comma ATYPEDECL>           \
  QI_API_DEPRECATED_MSG(Use generic 'schedulerFor' overload instead) boost::function<T> schedulerFor(                                                             \
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
      -> detail::WrapInStrand<typename std::decay<F>::type>
  {
    return detail::WrapInStrand<typename std::decay<F>::type>(std::forward<F>(func),
                                                              _p,
                                                              std::move(onFail));
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
  template <typename F, typename... Args>
  static auto callInStrand(
      F& func,
      const boost::function<void()>& onFail,
      boost::weak_ptr<StrandPrivate> weakStrand,
      Args&&... args)
      -> qi::Future<typename std::decay<decltype(func(std::forward<Args>(args)...))>::type>
  {
    if (auto strand = weakStrand.lock())
      return strand->async(std::bind(func, std::forward<Args>(args)...));
    else
    {
      if (onFail)
        onFail();
      return qi::makeFutureError<
          typename std::decay<decltype(func(std::forward<Args>(args)...))>::type>("strand is dead");
    }
  }
  // C++14 this can be a lambda, but we need perfect forwarding in the capture in scheduleFor below
  template <typename F>
  struct WrapInStrand
  {
    static const bool is_async = true;

    F _func;
    boost::weak_ptr<StrandPrivate> _strand;
    boost::function<void()> _onFail;

    WrapInStrand(F f, boost::weak_ptr<StrandPrivate> strand, boost::function<void()> onFail)
      : _func(std::move(f))
      , _strand(std::move(strand))
      , _onFail(std::move(onFail))
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const
        -> decltype(callInStrand(_func, _onFail, _strand, std::forward<Args>(args)...))
    {
      return callInStrand(_func, _onFail, _strand, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto operator()(Args&&... args)
        -> decltype(callInStrand(_func, _onFail, _strand, std::forward<Args>(args)...))
    {
      return callInStrand(_func, _onFail, _strand, std::forward<Args>(args)...);
    }
  };
} // detail
} // qi

# ifdef _MSC_VER
#  pragma warning( pop )
# endif

#endif  // _QI_STRAND_HPP_
