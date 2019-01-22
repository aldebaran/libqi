#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <boost/optional.hpp>
#include <boost/config.hpp>
#include <gtest/gtest.h>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>
#include <qi/clock.hpp>
#include <qi/future.hpp>

namespace test
{
  /// Runs a process from construction to destruction.
  /// At destruction, the process is killed with SIGKILL.
  class ScopedProcess
  {
    static const std::chrono::milliseconds defaultWaitReadyDuration;

    using Strings = std::vector<std::string>;
  public:
    explicit ScopedProcess(const std::string& executable,
                           const Strings& arguments = Strings{},
                           std::chrono::milliseconds waitReadyDuration = defaultWaitReadyDuration);

    ~ScopedProcess();

    // non-copyable
    ScopedProcess(const ScopedProcess&) = delete;
    ScopedProcess& operator=(const ScopedProcess&) = delete;

    int pid() const
    {
      return _pid;
    }

  private:
    std::string _executable;
    int _pid;
  };

  /// Predicate P
  template <typename P>
  static testing::AssertionResult verifyBeforeDuration(P pred,
                                                       qi::NanoSeconds dura,
                                                       qi::NanoSeconds period = qi::MilliSeconds{10})
  {
    while (!pred())
    {
      if (dura == qi::NanoSeconds::zero())
      {
        return testing::AssertionFailure()
               << "Predicate was not true before "
               << boost::chrono::duration_cast<qi::MicroSeconds>(dura).count() << " microseconds";
      }
      if (period >= dura) dura = qi::NanoSeconds::zero();
      else dura -= period;
      qi::sleepFor(period);
    }
    return testing::AssertionSuccess();
  }

  static const auto defaultFutureWaitDuration = qi::Seconds{ 5 };

  namespace detail
  {
    struct DoNothing
    {
      template<typename T>
      void operator()(const qi::Future<T>&) const {}
    };

    template<typename T>
    struct AssignValue
    {
      T* t;
      void operator()(const qi::Future<T>& fut) const
      {
        *t = fut.value();
      }
    };

    struct AssignError
    {
      std::string* err;
      template<typename T>
      void operator()(const qi::Future<T>& fut) const
      {
        *err = fut.error();
      }
    };

    template <typename T>
    testing::Message messageOfUnexpectedState(const qi::Future<T>& fut)
    {
      testing::Message msg;
      switch (fut.wait(0))
      {
      case qi::FutureState_None:
        msg << "the future is invalid as it is not tied to a promise";
        break;
      case qi::FutureState_Running:
        msg << "the future has timed out";
        break;
      case qi::FutureState_Canceled:
        msg << "the future has been canceled";
        break;
      case qi::FutureState_FinishedWithError:
        msg << "the future has an error '" << fut.error() << "'";
        break;
      case qi::FutureState_FinishedWithValue:
        msg << "the future has an value '" << fut.value() << "'";
        break;
      }
      return msg;
    }

    template <typename T, typename Proc = DoNothing>
    testing::AssertionResult finishesWithState(qi::Future<T> fut,
                                               qi::FutureState expected,
                                               Proc onSuccess = {},
                                               qi::MilliSeconds delay = defaultFutureWaitDuration)
    {
      const auto state = fut.wait(delay);
      if (state == expected)
      {
        onSuccess(fut);
        return testing::AssertionSuccess();
      }
      return testing::AssertionFailure() << messageOfUnexpectedState(fut);
    }
  }

  inline detail::DoNothing willDoNothing()
  {
    return {};
  }

  template<typename T>
  detail::AssignValue<T> willAssignValue(T& t)
  {
    return {&t};
  }

  inline detail::AssignError willAssignError(std::string& err)
  {
    return {&err};
  }

  template <typename T, typename Proc = detail::DoNothing>
  testing::AssertionResult finishesWithValue(qi::Future<T> fut,
                                             Proc onSuccess = {},
                                             qi::MilliSeconds delay = defaultFutureWaitDuration)
  {
    return detail::finishesWithState(fut, qi::FutureState_FinishedWithValue, onSuccess, delay);
  }

  template <typename T, typename... Args>
  testing::AssertionResult finishesWithValue(qi::FutureSync<T> fut, Args&&... args)
  {
    return finishesWithValue(fut.async(), ka::fwd<Args>(args)...);
  }

  template <typename T, typename Proc = detail::DoNothing>
  testing::AssertionResult finishesWithError(qi::Future<T> fut,
                                             Proc onSuccess = {},
                                             qi::MilliSeconds delay = defaultFutureWaitDuration)
  {
    return detail::finishesWithState(fut, qi::FutureState_FinishedWithError, onSuccess, delay);
  }

  template <typename T, typename... Args>
  testing::AssertionResult finishesWithError(qi::FutureSync<T> fut, Args&&... args)
  {
    return finishesWithError(fut.async(), ka::fwd<Args>(args)...);
  }

  template <typename T, typename Proc = detail::DoNothing>
  testing::AssertionResult finishesAsCanceled(qi::Future<T> fut,
                                              Proc onSuccess = {},
                                              qi::MilliSeconds delay = defaultFutureWaitDuration)
  {
    return detail::finishesWithState(fut, qi::FutureState_Canceled, onSuccess, delay);
  }

  template <typename T, typename... Args>
  testing::AssertionResult finishesAsCanceled(qi::FutureSync<T> fut, Args&&... args)
  {
    return finishesAsCanceled(fut.async(), ka::fwd<Args>(args)...);
  }

  template <typename T, typename Proc = detail::DoNothing>
  testing::AssertionResult isStillRunning(qi::Future<T> fut,
                                          Proc onSuccess = {},
                                          qi::MilliSeconds delay = defaultFutureWaitDuration)
  {
    return detail::finishesWithState(fut, qi::FutureState_Running, onSuccess, delay);
  }

  template <typename T, typename... Args>
  testing::AssertionResult isStillRunning(qi::FutureSync<T> fut, Args&&... args)
  {
    return isStillRunning(fut.async(), ka::fwd<Args>(args)...);
  }

  static const auto defaultConnectionAttemptTimeout = qi::Seconds{10};

  /// With C c, Args... args the following is valid:
  ///   Future<void> fut = c.connect(args...).async();
  template<typename C, typename... Args>
  qi::Future<void> attemptConnect(qi::MilliSeconds timeout, C& c, Args&&... args)
  {
    qi::Future<void> result;
    qi::FutureState state = qi::FutureState_None;
    const auto deadline = qi::SteadyClock::now() + timeout;
    while (qi::SteadyClock::now() < deadline && state != qi::FutureState_FinishedWithValue)
    {
      if (state != qi::FutureState_Running)
        result = c.connect(args...).async();

      state = result.wait(timeout);
    }
    return result;
  }

  /// Overload with defaulted timeout
  template <typename C, typename... Args>
  auto attemptConnect(C& c, Args&&... args)
    -> ka::EnableIf<!std::is_convertible<ka::Decay<C>, qi::MilliSeconds>::value, qi::Future<void>>
  {
    return attemptConnect(defaultConnectionAttemptTimeout, c, ka::fwd<Args>(args)...);
  }

} // namespace test
