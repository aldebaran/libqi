#pragma once
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <qi/macroregular.hpp>

namespace qi
{
  /// Polymorphic procedure that handles an exception through a dedicated handler
  /// and rethrows the exception as-is.
  ///
  /// Warning: Must be called in a `catch` clause, otherwise `std::terminate` will
  ///   be called (because there is no exception to rethrow).
  ///
  /// Example: Logs in error log and rethrows the exception.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// using Log = ExceptionLogError<std::string>;
  /// HandleExceptionAndRethrow<Log> logRethrow{Log{"logCategory", "doStuff"}};
  /// // ...
  /// auto res = invokeCatch(logRethrow, doStuff, arg0, arg1)
  /// // ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: Members are public because there's no invariant to maintain.
  ///
  /// PolymorphicProcedure<void (T), void ()> Proc
  template<typename Proc>
  struct HandleExceptionAndRethrow
  {
    Proc handleException;
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(HandleExceptionAndRethrow, handleException)
  // PolymorphicProcedure<void (T), void()>:
    template<typename T>
    void operator()(T&& e) const
    {
      handleException(std::forward<T>(e));
      throw;
    }
    void operator()() const
    {
      handleException();
      throw;
    }
  };

  /// Polymorphic function that maps an exception type to an arbitrary value.
  ///
  /// Handled exceptions are std::exception and boost::exception. If the exception
  /// is unknown (in the `catch (...)` case for example), you can call the nullary
  /// overload.
  ///
  /// Example: Turns a function that throws exceptions into a mathematical total function.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// ExceptionValue<int> code{stdCode, boostCode, unknownCode};
  /// int res = invokeCatch(code, doStuff, arg0, arg1)
  /// // test res for errors
  /// // ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// There is no constraint on T.
  template<typename T>
  struct ExceptionValue
  {
    T stdValue, boostValue, unknownValue;
  // Regular (if T is):
    QI_GENERATE_FRIEND_REGULAR_OPS_3(ExceptionValue, stdValue, boostValue, unknownValue)
  // PolymorphicFunction<T (std::exception), T (boost::exception), T ()>:
    T operator()(std::exception const&) const
    {
      return stdValue;
    }
    T operator()(boost::exception const&) const
    {
      return boostValue;
    }
    T operator()() const
    {
      return unknownValue;
    }
  };

  /// Invokes a procedure in a try-catch clause and delegates exception handling
  /// to a dedicated handler.
  ///
  /// std::exceptions and boost::exceptions are passed as-is to the handler.
  /// In all other cases, the handler is called without parameter.
  ///
  /// This function can be used for example to log the exception and rethrow, to
  /// translate the exception to a value (useful to compose functions in a
  /// purely functional manner), etc.
  ///
  /// See ExceptionValue, HandleExceptionAndRethrow.
  ///
  /// Note: The exception types are hardcoded instead of specifying them via
  ///   a variadic list because, in a variadic template approach catching the
  ///   exceptions E0, E1 and any exception would generate this kind of code:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// try {
  ///   try {
  ///     try {
  ///       return f(args...);
  ///     } catch (const E0& e0) {
  ///       return handleException(e0);
  ///     }
  ///   } catch (const E1& e1) {
  ///     return handleException(e1);
  ///   }
  /// } catch (...) {
  ///   return handleException();
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   which is not equivalent to this handwritten code:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///  try {
  ///    return f(args...);
  ///  } catch (const E0& e0) {
  ///    return handleException(e0);
  ///  } catch (const E1& e1) {
  ///    return handleException(e1);
  ///  } catch (...) {
  ///    return handleException();
  ///  }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   because if the exception handler rethrows the exception, the "catch all"
  ///   clause will always be called.
  ///   So it seems, there is no way in C++ to specify in a compile-time way a
  ///   list of exceptions to catch...
  ///
  /// PolymorphicProcedure<Ret (const std::exception&), Ret (const boost::exception&), Ret ()> Proc,
  /// Procedure<Ret (Args...)> F
  template<typename Proc, typename F, typename... Args>
  auto invokeCatch(Proc&& handleException, F&& f, Args&&... args)
      // TODO: Remove this when migrating to C++14.
      -> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
  {
    try
    {
      return std::forward<F>(f)(std::forward<Args>(args)...);
    }
    catch (std::exception const& e)
    {
      return std::forward<Proc>(handleException)(e);
    }
    catch (boost::exception const& e)
    {
      return std::forward<Proc>(handleException)(e);
    }
    catch (...)
    {
      return std::forward<Proc>(handleException)();
    }
  }

} // namespace qi
