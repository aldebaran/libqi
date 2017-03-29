#pragma once
#include <qi/macroregular.hpp>

/// @file
/// Contains functional tools: no-op procedure, etc.

namespace qi {
  /// A procedure that does nothing.
  template<typename F>
  struct NoOpProcedure;

  /// You can specify the value to return.
  template<typename Ret, typename... Args>
  struct NoOpProcedure<Ret (Args...)>
  {
    Ret _ret;
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(NoOpProcedure, _ret)
  // Procedure:
    Ret operator()(const Args&...) const
    {
      return _ret;
    }
  };

  template<typename... Args>
  struct NoOpProcedure<void (Args...)>
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(NoOpProcedure)
  // Procedure:
    void operator()(const Args&...) const
    {
    }
  };

  /// A polymorphic transformation that takes a procedure and returns it as-is.
  ///
  /// A transformation is a unary function from a type to itself.
  struct IdTransfo
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(IdTransfo)
  // PolymorphicTransformation:
    template<typename T>
    inline T operator()(const T& t) const
    {
      return t;
    }
  };

  /// Bind a data to a procedure without changing its semantics.
  /// One use is to bind a shared pointer to extend the lifetime of some data
  /// related to the procedure.
  ///
  /// This type is useful equivalent to a variadic generic lambda in C++14:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// T data;
  /// auto dataBoundProc = [data](auto&&... args) {
  ///   // ...
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Warning: This type is not strictly equivalent to a lambda (C++14 or even C++17),
  ///   as it is regular and lambdas are not.
  ///
  /// Procedure Proc
  template<typename Proc, typename T>
  struct DataBoundProc
  {
    Proc _proc;
    T _data;
  // Regular (if T is regular):
    QI_GENERATE_FRIEND_REGULAR_OPS_2(DataBoundProc, _proc, _data)
  // Procedure:
    template<typename... Args>
    auto operator()(Args&&... args) -> decltype(_proc(std::forward<Args>(args)...))
    {
      return _proc(std::forward<Args>(args)...);
    }
  };

  /// A polymorphic transformation that takes a procedure and returns
  /// an equivalent one that is bound to an arbitrary data.
  /// The bound data can be a shared pointer to extend the lifetime of an object
  /// the procedure is dependent on.
  ///
  /// Example: extending instance lifetime by binding a shared pointer
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Network N
  /// template<typename N>
  /// struct X : std::enable_shared_from_this<X> {
  ///   SendMessageEnqueue<N, SocketPtr<N>> _sendMsg;
  ///
  ///   void doStuff() {
  ///     // ...
  ///     auto onSent = [](...) {
  ///        // use X's members here
  ///     };
  ///     // _sendMsg will wrap any asynchronous task by binding it with a
  ///     // shared pointer on this instance.
  ///     _sendMsg(msg, ssl, onSent,
  ///       dataBoundTransfo(shared_from_this()) // dataTransfo
  ///     );
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular T
  template<typename T>
  struct DataBoundTransfo
  {
    T _data;
  // Regular (if T is regular):
    QI_GENERATE_FRIEND_REGULAR_OPS_1(DataBoundTransfo, _data)
  // PolymorphicTransformation:
    template<typename Proc>
    DataBoundProc<traits::Decay<Proc>, T> operator()(Proc&& p) const
    {
      return {std::forward<Proc>(p), _data};
    }
  };

  /// Helper function to deduce types for DataBoundTransfo.
  template<typename T>
  DataBoundTransfo<T> dataBoundTransfo(const T& maintainAlive)
  {
    return {maintainAlive};
  }

} // namespace qi
