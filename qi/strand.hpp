#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_STRAND_HPP_
#define _QI_STRAND_HPP_

#include <deque>
#include <atomic>
#include <memory>
#include <ka/macro.hpp>
#include <ka/functional.hpp>
#include <ka/mutablestore.hpp>
#include <qi/assert.hpp>
#include <qi/config.hpp>
#include <qi/detail/executioncontext.hpp>
#include <qi/detail/futureunwrap.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/function_traits.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, ) // TODO: Reactivate this warning once msvc stop triggerring a warning on overloading a deprecated function

namespace qi
{

// C++14 these can be lambdas, but we need perfect forwarding in the capture in scheduleFor below
namespace detail
{
  template <typename F>
  struct Stranded;

  template <typename F>
  struct StrandedUnwrapped;
}

// we use ExecutionContext's helpers in schedulerFor, we don't need to implement all the methods
class QI_API StrandPrivate : public ExecutionContext, public boost::enable_shared_from_this<StrandPrivate>
{
public:
  enum class State;

  struct Callback;

  using Queue = std::deque<boost::shared_ptr<Callback>>;

  qi::ExecutionContext& _executor;
  std::atomic<unsigned int> _curId;
  std::atomic<unsigned int> _aliveCount;
  bool _processing; // protected by mutex, no need for atomic
  std::atomic<int> _processingThread;
  boost::recursive_mutex _mutex;
  boost::condition_variable_any _processFinished;
  bool _dying;
  Queue _queue;
  class ScopedPromiseGroup;
  std::shared_ptr<ScopedPromiseGroup> _deferredTasksFutures; // Shared to avoid including issues

  explicit StrandPrivate(qi::ExecutionContext& executor);
  ~StrandPrivate();

  void join() QI_NOEXCEPT(true);

  // Schedules the callback for execution. If the trigger date `tp` is in the past, executes the
  // callback immediately in the calling thread.
  Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp, ExecutionOptions options = defaultExecutionOptions()) override;

  // Schedules the callback for execution. If delay is 0, executes the callback immediately in the
  // calling thread.
  Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options = defaultExecutionOptions()) override;

  // Schedules the callback for deferred execution and returns immediately.
  Future<void> deferImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options = defaultExecutionOptions());

  boost::shared_ptr<Callback> createCallback(boost::function<void()> cb, ExecutionOptions options);
  void enqueue(boost::shared_ptr<Callback> cbStruct, ExecutionOptions options);

  void process();
  void cancel(boost::shared_ptr<Callback> cbStruct);
  bool isInThisContext() const override;

  void postImpl(boost::function<void()> /*callback*/, ExecutionOptions /*options*/) override
  { QI_ASSERT(false); throw 0; }

  qi::Future<void> async(const boost::function<void()>& /*callback*/, qi::SteadyClockTimePoint /*tp*/) override
  { QI_ASSERT(false); throw 0; }

  qi::Future<void> async(const boost::function<void()>& /*callback*/, qi::Duration /*delay*/) override
  { QI_ASSERT(false); throw 0; }

  using ExecutionContext::async;
private:
  void stopProcess(boost::recursive_mutex::scoped_lock& lock,
                   bool finished);

  bool joined = false;

  template<class Task>
  auto track(Task&& task)
    // TODO: C++ >= 14 : Remove the following line.
    -> decltype(ka::scope_lock_proc(ka::fwd<Task>(task), ka::mutable_store(weak_from_this())))
  {
    return ka::scope_lock_proc(ka::fwd<Task>(task), ka::mutable_store(weak_from_this()));
  }

};


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
  using OptionalErrorMessage = boost::optional<std::string>;

  /// Construct a strand that will schedule work on the default event loop
  Strand();
  /// Construct a strand that will schedule work on executionContext
  Strand(qi::ExecutionContext& executionContext);

  /// Call detroy()
  ~Strand();

  /** Joins the strand.
   *
   * This will wait for currently running tasks to finish and will drop all tasks scheduled from the moment of the call
   * on. A strand can't be reused after it has been join()ed.
   *
   * It is safe to call this method concurrently with other methods. All the returned futures will be set to error.
   * @Note: Under extreme circumstances such as system memory exhaustion, this
   *        method could still throw a `std::bad_alloc` exception, thus causing a call
   *        to `std::terminate` because of the `noexcept` specifier. This behavior is
   *        considered acceptable.
   */
  void join() QI_NOEXCEPT(true);

  /** Joins the strand.
   *
   * @deprecated Use join() which is currently noexcept.
   *
   * This version catches any exception and returns its message.
   * This version must be preferred in destructors to prevent abort.
   *
   * If there is no exception, an empty error message is returned.
   *
   * Example: Preventing a destructor to throw exceptions because of `join`.
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   * ~MyActor() {
   *   if (const auto error = _strand.join(std::nothrow)) {
   *     qiLogWarning() << "Error while joining the strand. Detail: " << *error;
   *   }
   * }
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  QI_API_DEPRECATED_MSG(Use 'join()' instead)
  OptionalErrorMessage join(std::nothrow_t) QI_NOEXCEPT(true);

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
  bool isInThisContext() const override;


  /// Returns a function which, when called, defers a call to the original
  /// function to the strand.
  ///
  /// If the strand has been joined, the returned function will be a no-op.
  ///
  /// @example This code provides a function to set a data from within the
  /// strand:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Strand strand;
  /// int data = 0;
  /// auto dataSetter = [&](int i){ data = i };
  /// auto safeDataSetter = strand.schedulerFor(dataSetter);
  /// safeDataSetter(42);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Since the strand prevents concurrent calls, the data is safely set if
  /// this setter is used.
  ///
  /// If the function was already asynchronous (returning a qi::Future<T>),
  /// the returned function will return a qi::Future<qi::Future<T>>, that you
  /// can choose to unwrap.
  ///
  /// If you want the return type of the transformed function to remain a
  /// qi::Future<T>, @see unwrappedSchedulerFor.
  ///
  /// @note Using std::bind with arguments wrapped in std::ref on a function
  /// transformed with schedulerFor may have counter-intuitive effects. STL's
  /// invocation of the bound function will lose the std::ref, so that when
  /// the deferred call is performed, the reference will not be recognized,
  /// and the argument will be copied to the target function.
  template <typename F>
  auto schedulerFor(F&& func, boost::function<void()> onFail = {}, ExecutionOptions options = defaultExecutionOptions())
      -> detail::Stranded<typename std::decay<F>::type>
  {
    return detail::Stranded<typename std::decay<F>::type>(std::forward<F>(func),
                                                              boost::atomic_load(&_p),
                                                              std::move(onFail),
                                                              options);
  }

  /// Returns a function which, when called, defers a call to the original
  /// function to the strand, but with the return value unwrapped if possible.
  /// @see schedulerFor for details.
  template <typename F>
  auto unwrappedSchedulerFor(F&& func, boost::function<void()> onFail = {}, ExecutionOptions options = defaultExecutionOptions())
      -> detail::StrandedUnwrapped<typename std::decay<F>::type>
  {
    return detail::StrandedUnwrapped<typename std::decay<F>::type>(std::forward<F>(func),
                                                              boost::atomic_load(&_p),
                                                              std::move(onFail),
                                                              options);
  }

  /**
   * Defers a function for execution in the strand thus without allowing the strand to call it from
   * inside this function. It implies that this function always returns immediately.
   * @param cb: The function to execute.
   * @param delay: Duration that defer will wait (without blocking the caller) before queuing the
   *               function for execution. If zero (the default value), the function will be queued
   *               immediately.
   * @param options: Execution options.
   * @returns A future that is set once the function argument is executed
   */
  Future<void> defer(const boost::function<void()>& cb,
                      MicroSeconds delay = MicroSeconds::zero(),
                      ExecutionOptions options = defaultExecutionOptions());

private:
  boost::shared_ptr<StrandPrivate> _p;

  void postImpl(boost::function<void()> callback, ExecutionOptions options) override;

  qi::Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp, ExecutionOptions options) override;
  qi::Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options) override;

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
      ExecutionOptions options,
      Args... args)
      -> decltype(weakStrand.lock()->async(std::bind(func, std::move(args)...), options)) // TODO: remove in C++14
  {
    if (auto strand = weakStrand.lock())
    {
      return strand->async(std::bind(func, std::move(args)...), options);
    }
    else
    {
      if (onFail)
        onFail();
      return qi::makeFutureError<
          typename std::decay<decltype(func(std::forward<Args>(args)...))>::type>("strand is dead");
    }
  }

  // C++14 these can be lambdas, but we need perfect forwarding in the capture in scheduleFor
  // A callable object that, when called defers the call of the given function to the strand.
  template <typename F>
  struct Stranded
  {
    static const bool is_async = true;

    F _func;
    boost::weak_ptr<StrandPrivate> _strand;
    boost::function<void()> _onFail;
    ExecutionOptions _executionOptions;

    Stranded(F f, boost::weak_ptr<StrandPrivate> strand, boost::function<void()> onFail, ExecutionOptions options)
      : _func(std::move(f))
      , _strand(std::move(strand))
      , _onFail(std::move(onFail))
      , _executionOptions(std::move(options))
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const
        -> decltype(callInStrand(_func, _onFail, _strand, _executionOptions, std::forward<Args>(args)...))
    {
      return callInStrand(_func, _onFail, _strand, _executionOptions, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto operator()(Args&&... args)
        -> decltype(callInStrand(_func, _onFail, _strand, _executionOptions, std::forward<Args>(args)...))
    {
      return callInStrand(_func, _onFail, _strand, _executionOptions, std::forward<Args>(args)...);
    }
  };

  // Like Stranded, but unwraps the result.
  template <typename F>
  struct StrandedUnwrapped
  {
    static const bool is_async = true;

  private:
    Stranded<F> _stranded;

  public:
    template <typename FF>
    StrandedUnwrapped(FF&& f, const boost::weak_ptr<StrandPrivate>& strand, const boost::function<void()>& onFail, ExecutionOptions options)
      : _stranded(std::forward<FF>(f), strand, onFail, options)
    {}

    template <typename... Args>
    auto operator()(Args&&... args) const
        -> decltype(tryUnwrap(_stranded(std::forward<Args>(args)...)))
    {
      return tryUnwrap(_stranded(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto operator()(Args&&... args)
        -> decltype(tryUnwrap(_stranded(std::forward<Args>(args)...)))
    {
      return tryUnwrap(_stranded(std::forward<Args>(args)...));
    }
  };
} // detail
} // qi

KA_WARNING_POP()

# include <qi/async.hpp>

#endif  // _QI_STRAND_HPP_
