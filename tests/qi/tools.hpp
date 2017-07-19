#ifndef _QI_TESTS_TOOLS_HPP_
#define _QI_TESTS_TOOLS_HPP_

namespace test
{
  /// Useful to test support for move-only types.
  template<typename T>
  class MoveOnly
  {
    mutable T value;
    bool moved = false;
    void checkNotMoved() const
    {
      if (moved) throw std::runtime_error("operating on moved instance");
    }
  public:
    explicit MoveOnly(const T& value = T())
      : value(value)
    {
    }
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& x)
      : value((x.checkNotMoved(), std::move(x.value)))
    {
      x.moved = true;
    }
    MoveOnly& operator=(MoveOnly&& x)
    {
      x.checkNotMoved();
      value = std::move(x.value);
      x.moved = true;
      return *this;
    }
    bool operator==(MoveOnly const& x) const
    {
      checkNotMoved();
      x.checkNotMoved();
      return value == x.value;
    }
  // Mutable<T>:
    T& operator*() const
    {
      checkNotMoved();
      return value;
    }
  };

  enum class RefKind
  {
    LValue,
    RValue
  };
} // namespace test

#endif // _QI_TESTS_TOOLS_HPP_
