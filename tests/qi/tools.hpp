#ifndef _QI_TESTS_TOOLS_HPP_
#define _QI_TESTS_TOOLS_HPP_
#include <qi/macro.hpp>

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

  /// Allows to know if an instance has been moved.
  struct MoveAware
  {
    int i;
    bool moved = false;
    explicit MoveAware(int i) QI_NOEXCEPT(true)
      : i(i)
    {
    }
    MoveAware() = default;
    MoveAware(const MoveAware& x) QI_NOEXCEPT(true)
      : i(x.i)
    {
    }
    MoveAware& operator=(const MoveAware& x) QI_NOEXCEPT(true)
    {
      i = x.i;
      moved = false;
      return *this;
    }
    MoveAware(MoveAware&& x) QI_NOEXCEPT(true)
      : i(x.i)
    {
      x.moved = true;
    }
    MoveAware& operator=(MoveAware&& x) QI_NOEXCEPT(true)
    {
      i = x.i;
      moved = false;
      x.moved = true;
      return *this;
    }
    bool operator==(const MoveAware& x) const QI_NOEXCEPT(true)
    {
      return i == x.i; // ignore the `moved` flag.
    }
    friend std::ostream& operator<<(std::ostream& o, const MoveAware& x)
    {
      return o << x.i;
    }
  };
} // namespace test

#endif // _QI_TESTS_TOOLS_HPP_
