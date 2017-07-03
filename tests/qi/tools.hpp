#ifndef _QI_TESTS_TOOLS_HPP_
#define _QI_TESTS_TOOLS_HPP_

namespace test
{
  /// Useful to test support for move-only types.
  template<typename T>
  struct MoveOnly
  {
    T value;
    MoveOnly(const T& value)
      : value(value)
    {
    }
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& x)
      : value(std::move(x.value))
    {
    }
    MoveOnly& operator=(MoveOnly&& x)
    {
      value = std::move(x.value);
      return *this;
    }
    bool operator==(MoveOnly const& x) const
    {
      return value == x.value;
    }
  };
} // namespace test

#endif // _QI_TESTS_TOOLS_HPP_
