#pragma once
#include <utility>
#include <qi/type/traits.hpp>
#include <qi/macroregular.hpp>

namespace qi
{
  /// Moves its value on copy.
  ///
  /// Useful in C++11 to simulate a capture by move in a lambda.
  ///
  /// Example: moving a value inside a lambda
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // sslContext is move-only.
  ///
  /// // First, move it in a `MoveOnCopy` object.
  /// auto context = makeMoveOnCopy(std::move(sslContext));
  ///
  /// // Capture the `MoveOnCopy` object by value, which triggers a call the
  /// // copy constructor. Because the copy constructor is defined to perform
  /// // a move, the ssl context is effectively moved inside the lambda.
  /// async_connect([=](ErrorCode e) { // completion handler (captures by value)
  ///   // ...
  ///   // Unpack the ssl context.
  ///   auto s = createSocket(std::move(*context));
  ///   // ...
  /// });
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Warning: Because of the move, the source operand of a copy is not valid
  ///   anymore after the copy.
  template<typename T>
  class MoveOnCopy
  {
    T value;
  public:
    MoveOnCopy() = default;

    template<typename U, typename = traits::EnableIfNotBaseOf<MoveOnCopy, U>>
    explicit MoveOnCopy(U&& u)
      : value(std::forward<U>(u))
    {
    }

    MoveOnCopy(const MoveOnCopy& x)
      : value(std::move(const_cast<MoveOnCopy&>(x).value))
    {
    }

    // TODO: Use default version when get rid of VS2013.
    MoveOnCopy(MoveOnCopy&& x)
      : value(std::move(x.value))
    {
    }

    MoveOnCopy& operator=(const MoveOnCopy& x)
    {
      value = std::move(const_cast<MoveOnCopy&>(x).value);
      return *this;
    }

    // TODO: Use default version when get rid of VS2013.
    MoveOnCopy& operator=(MoveOnCopy&& x)
    {
      value = std::move(x.value);
    }

    // MoveOnCopy is not regular but we can still provide the relational operators.
    QI_GENERATE_FRIEND_REGULAR_OPS_1(MoveOnCopy, value)

  // Mutable<T>:
    /// As a Mutable, constness of the `MoveOnCopy` does not imply constness of
    /// the "indirected" object.
    T& operator*() const
    {
      return const_cast<T&>(value);
    }
  };

  /// Helper function to perform type deduction for constructing a MoveOnCopy.
  template<typename T>
  MoveOnCopy<traits::Decay<T>> makeMoveOnCopy(T&& t)
  {
    return MoveOnCopy<traits::Decay<T>>{std::forward<T>(t)};
  }
} // namespace qi
