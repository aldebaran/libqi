#include <iterator>
#include <memory>
#include <stdexcept>
#include <boost/optional/optional_io.hpp>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/opt.hpp>
#include <ka/functional.hpp>
#include <ka/testutils.hpp>
#include <ka/typetraits.hpp>
#include <ka/functor.hpp>
#include <ka/flatten.hpp>
#include <ka/testutils.hpp>

TEST(Optional, Regular) {
  using namespace ka;
  using O = opt_t<int>;
  O o0, o1, o2, o3;
  o1.set(1);
  o2.set(2);
  o3.set(3);
  ASSERT_TRUE(is_regular({o0, o1, o2, o3}));
}

TEST(Optional, RegularVoid) {
  using namespace ka;
  using O = opt_t<void>;
  O o0, o1, o2, o3;
  o1.set();
  o2.set();
  o3.set();
  ASSERT_TRUE(is_regular({o0, o1, o2, o3}));
}

TEST(Optional, Empty) {
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_TRUE(o.empty());
  o.set(1);
  ASSERT_FALSE(o.empty());
  o = O{};
  ASSERT_TRUE(o.empty());
}

TEST(Optional, EmptyVoid) {
  using namespace ka;
  using O = opt_t<void>;
  O o;
  ASSERT_TRUE(o.empty());
  o.set();
  ASSERT_FALSE(o.empty());
  o = O{};
  ASSERT_TRUE(o.empty());
}

TEST(Optional, ReadableMutable) {
  using namespace ka;
  using O = opt_t<int>;
  const int i = 1;
  O o;
  o.set(i);
  ASSERT_EQ(i, *o);
  *o = i - 1;
  ASSERT_EQ(i - 1, *o);
  o.set(i + 1);
  ASSERT_EQ(i + 1, *o);
  {
    O o;
    static_assert(Equal<int&, decltype(*o)>::value, "");
  } {
    const O o{}; // TODO: remove `{}` when clang is upgraded.
    static_assert(Equal<const int&, decltype(*o)>::value, "");
  }
}

TEST(Optional, ReadableVoid) {
  using namespace ka;
  using O = opt_t<void>;
  O o;
  o.set();
  ASSERT_NO_THROW(*o);
  static_assert(Equal<void, decltype(*o)>::value, "");
  {
    O o;
    static_assert(Equal<void, decltype(*o)>::value, "");
  } {
    const O o{}; // TODO: remove `{}` when clang is upgraded.
    static_assert(Equal<void, decltype(*o)>::value, "");
  }
}

TEST(Optional, GetPtr) {
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_EQ(nullptr, o.get_ptr());
  o.set(1);
  ASSERT_NE(nullptr, o.get_ptr());
  o = O{};
  ASSERT_EQ(nullptr, o.get_ptr());
  {
    O o;
    static_assert(Equal<int*, decltype(o.get_ptr())>::value, "");
  } {
    const O o{}; // TODO: remove `{}` when clang is upgraded.
    static_assert(Equal<const int*, decltype(o.get_ptr())>::value, "");
  }
}

TEST(Optional, GetPtrVoid) {
  using namespace ka;
  using O = opt_t<void>;
  O o;
  ASSERT_EQ(nullptr, o.get_ptr());
  o.set();
  ASSERT_NE(nullptr, o.get_ptr());
  o = O{};
  ASSERT_EQ(nullptr, o.get_ptr());
  {
    O o;
    static_assert(Equal<void*, decltype(o.get_ptr())>::value, "");
  } {
    const O o{}; // TODO: remove `{}` when clang is upgraded.
    static_assert(Equal<const void*, decltype(o.get_ptr())>::value, "");
  }
}

TEST(Optional, ConversionToBool) {
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_TRUE(o.empty());
  o.set(1);
  ASSERT_FALSE(o.empty());
  o = O{};
  ASSERT_TRUE(o.empty());
}

TEST(Optional, ConversionToBoolVoid) {
  using namespace ka;
  using O = opt_t<void>;
  O o;
  ASSERT_TRUE(o.empty());
  o.set();
  ASSERT_FALSE(o.empty());
  o = O{};
  ASSERT_TRUE(o.empty());
}

TEST(Optional, CallSet) {
  using namespace ka;
  opt_t<int> o;
  ASSERT_TRUE(o.empty());
  o.call_set([]() {return 1;});
  ASSERT_FALSE(o.empty());
  ASSERT_EQ(1, *o);
}

TEST(Optional, CallSetVoid) {
  using namespace ka;
  opt_t<void> o;
  ASSERT_TRUE(o.empty());
  int i = 0;
  o.call_set([&]() {++i;});
  ASSERT_EQ(1, i);
  ASSERT_FALSE(o.empty());
}

namespace {
  struct exec_call_t {
    /// Procedure<T (Args...)> Proc
    template<typename Proc, typename... Args>
    auto operator()(Proc proc, Args&&... args) const -> ka::opt_t<ka::Decay<decltype(proc(ka::fwd<Args>(args)...))>> {
      using namespace ka;
      opt_t<Decay<decltype(proc(fwd<Args>(args)...))>> o;
      o.call_set(proc, fwd<Args>(args)...);
      return o;
    }
  };

  struct exec_no_call_t {
    /// Procedure<T (Args...)> Proc
    template<typename Proc, typename... Args>
    auto operator()(Proc proc, Args&&... args) const -> ka::opt_t<ka::Decay<decltype(proc(ka::fwd<Args>(args)...))>> {
      using namespace ka;
      return opt_t<Decay<decltype(proc(fwd<Args>(args)...))>>{};
    }
  };
}

TEST(Optional, CallAndSetGeneric) {
  using namespace ka;
  const auto f = [](int i, double d) -> double {
    return i + d;
  };
  {
    opt_t<double> o = exec_call_t{}(f, 1, 2.0);
    ASSERT_FALSE(o.empty());
    ASSERT_EQ(3.0, *o);
  } {
    opt_t<double> o = exec_no_call_t{}(f, 1, 2.0);
    ASSERT_TRUE(o.empty());
  }
}

// This test is removed in the case of VS2013 because of the following error in
// boost::optional code:
//  error C2280: 'ka::move_only_t<int>::move_only_t(const ka::move_only_t<int> &)'
//    : attempting to reference a deleted function
//
// TODO: Remove this if when VS2013 is not supported anymore.
#if !KA_COMPILER_VS2013_OR_BELOW
TEST(Optional, CallAndSetGenericMoveOnly) {
  using namespace ka;
  using M = move_only_t<int>;
  const auto f = [](M x) {
    *x += 1;
    return x;
  };
  int const i = 3;
  {
    opt_t<M> o = exec_call_t{}(f, M{i});
    ASSERT_FALSE(o.empty());
    ASSERT_EQ(i + 1, **o); // First `*` is for `opt_t`, second one is for `move_only_t`.
  } {
    opt_t<M> o = exec_no_call_t{}(f, M{i});
    ASSERT_TRUE(o.empty());
  }
}
#endif

TEST(Optional, CallAndSetGenericVoid) {
  using namespace ka;
  {
    int i = 1;
    double d = 2.0;
    const auto f = [&]() {
      d += i;
    };
    opt_t<void> o = exec_call_t{}(f);
    ASSERT_FALSE(o.empty());
    ASSERT_EQ(3.0, d);
  } {
    int i = 1;
    double d = 2.0;
    const auto f = [&]() {
      d += i;
    };
    opt_t<void> o = exec_no_call_t{}(f);
    ASSERT_TRUE(o.empty());
    ASSERT_EQ(2.0, d);
  }
}

namespace {
  /// Procedure<T (Args...)> Proc
  template<typename E, typename Proc, typename... Args>
  auto exec_custom(E exec, Proc proc, Args&&... args) -> decltype(exec(proc, ka::fwd<Args>(args)...)) {
    // pre-processing...
    auto o = std::move(exec(proc, ka::fwd<Args>(args)...));
    // post-processing...
    return o;
  }
}

TEST(Optional, Generic) {
  using ka::Equal;
  const auto f = [](int a, long b) {
    return a + b;
  };
  {
    auto o = exec_custom(exec_call_t{}, f, 1, 2L);
    ASSERT_FALSE(o.empty());
    static_assert(Equal<long&, decltype(*o)>::value, "");
    ASSERT_EQ(3, *o);
  } {
    auto o = exec_custom(exec_no_call_t{}, f, 1, 2L);
    ASSERT_TRUE(o.empty());
    static_assert(Equal<long&, decltype(*o)>::value, "");
  }
}

// This test is removed in the case of VS2013 because of the following error in
// boost::optional code:
//  error C2280: 'ka::move_only_t<int>::move_only_t(const ka::move_only_t<int> &)'
//    : attempting to reference a deleted function
//
// TODO: Remove this if when VS2013 is not supported anymore.
#if !KA_COMPILER_VS2013_OR_BELOW
TEST(Optional, GenericMoveOnly) {
  using ka::Equal;
  using M = ka::move_only_t<int>;
  const auto f = [](M x) {
    *x += 1;
    return x;
  };
  int const i = 3;
  {
    auto o = exec_custom(exec_call_t{}, f, M{i});
    ASSERT_FALSE(o.empty());
    static_assert(Equal<int&, decltype(**o)>::value, "");
    ASSERT_EQ(i + 1, **o);
  } {
    auto o = exec_custom(exec_no_call_t{}, f, M{i});
    ASSERT_TRUE(o.empty());
    static_assert(Equal<int&, decltype(**o)>::value, "");
  }
}
#endif

TEST(Optional, GenericVoid) {
  using ka::Equal;
  {
    int a = 1;
    const auto f = [&](long b) {
      a += b;
    };
    auto o = exec_custom(exec_call_t{}, f, 2L);
    ASSERT_FALSE(o.empty());
    static_assert(Equal<void, decltype(*o)>::value, "");
    ASSERT_EQ(3, a);
  } {
    int a = 1;
    const auto f = [&](long b) {
      a += b;
    };
    auto o = exec_custom(exec_no_call_t{}, f, 2L);
    ASSERT_TRUE(o.empty());
    static_assert(Equal<void, decltype(*o)>::value, "");
    ASSERT_EQ(1, a);
  }
}

TEST(Optional, ConstructorFunction) {
  using namespace ka;

  ASSERT_FALSE(opt(1).empty());

  static_assert(Equal<opt_t<int>, decltype(opt(1))>::value, "");
  ASSERT_EQ(opt(1), opt_t<int>{}.set(1));

  static_assert(Equal<opt_t<char>, decltype(opt('a'))>::value, "");
  ASSERT_EQ(opt('a'), opt_t<char>{}.set('a'));

  static_assert(Equal<opt_t<bool>, decltype(opt(true))>::value, "");
  ASSERT_EQ(opt(true), opt_t<bool>{}.set(true));
}

TEST(Optional, ConstructorFunctionVoid) {
  using namespace ka;

  ASSERT_FALSE(opt().empty());

  static_assert(Equal<opt_t<void>, decltype(opt())>::value, "");
  ASSERT_EQ(opt(), opt_t<void>{}.set());
}

TEST(Optional, ContainerNonVoidEmpty) {
  using namespace ka;
  using O = opt_t<int>;
  static_assert(Equal<int, O::value_type>::value, "");
  static_assert(Equal<int&, O::reference>::value, "");
  static_assert(Equal<int const&, O::const_reference>::value, "");
  static_assert(Equal<int*, O::iterator>::value, "");
  static_assert(Equal<int const*, O::const_iterator>::value, "");
  static_assert(Equal<std::ptrdiff_t, O::difference_type>::value, "");
  static_assert(Equal<std::size_t, O::size_type>::value, "");
  O a; // default construction
  {
    O const& c = a;
    ASSERT_EQ(c.begin(), c.end()); // begin, end
    ASSERT_EQ(c.cbegin(), c.cend()); // cbegin, cend
    ASSERT_EQ(c.size(), 0u); // size
    ASSERT_EQ(c.max_size(), 1u); // max_size
    ASSERT_TRUE(c.empty()); // empty
  }
  a.set(1);
  O b(a); // copy
  b = a; // assignment
  a.swap(b); // swap
  swap(a, b); // swap (free function)
  {
    O const& c = a;
    O const& d = b;
    ASSERT_TRUE(c == d); // equality
    ASSERT_FALSE(c != d); // inequality
    ASSERT_EQ(c.begin() + 1, c.end());
    ASSERT_EQ(c.cbegin() + 1, c.cend());
    ASSERT_EQ(c.size(), 1u);
    ASSERT_EQ(c.max_size(), 1u);
    ASSERT_FALSE(c.empty());
  }
}

TEST(Optional, ContainerNonVoidNonEmpty) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 12;
  int const j = 21;
  O a;
  O b;
  {
    O const& c = a;
    O const& d = b;
    ASSERT_TRUE(c.empty());
    ASSERT_TRUE(d.empty());
    ASSERT_EQ(c.size(), 0u);
    ASSERT_EQ(d.size(), 0u);
    ASSERT_EQ(c.max_size(), 1u);
    ASSERT_EQ(d.max_size(), 1u);
  }
  a.set(x);
  b.set(j);
  {
    O const& c = a;
    O const& d = b;
    ASSERT_FALSE(c.empty());
    ASSERT_FALSE(d.empty());
    ASSERT_NE(c, d);
    ASSERT_EQ(*c.begin(), x);
    ASSERT_EQ(*d.begin(), j);
    ASSERT_EQ(c.begin() + 1, c.end());
    ASSERT_EQ(d.begin() + 1, d.end());
    ASSERT_EQ(*c.cbegin(), x);
    ASSERT_EQ(*d.cbegin(), j);
    ASSERT_EQ(c.cbegin() + 1, c.cend());
    ASSERT_EQ(d.cbegin() + 1, d.cend());
    ASSERT_EQ(c.size(), 1u);
    ASSERT_EQ(d.size(), 1u);
    ASSERT_FALSE(c.empty());
    ASSERT_FALSE(d.empty());
  }
  a = b;
  {
    O const& c = a;
    O const& d = b;
    ASSERT_EQ(c, d);
  }
}

TEST(Optional, ContainerVoid) {
  using namespace ka;
  using O = opt_t<void>;
  static_assert(Equal<void, O::value_type>::value, "");
  static_assert(Equal<std::size_t, O::size_type>::value, "");
  O a;
  O b;
  ASSERT_EQ(a.size(), 0u);
  ASSERT_EQ(b.size(), 0u);
  ASSERT_TRUE(a.empty());
  ASSERT_TRUE(b.empty());
  ASSERT_EQ(a.max_size(), 1u);
  ASSERT_EQ(b.max_size(), 1u);
  a.set();
  b.set();
  ASSERT_EQ(a, b);
  ASSERT_EQ(a.size(), 1u);
  ASSERT_EQ(b.size(), 1u);
  ASSERT_FALSE(a.empty());
  ASSERT_FALSE(b.empty());
  ASSERT_EQ(a.max_size(), 1u);
  ASSERT_EQ(b.max_size(), 1u);
  a = b;
  ASSERT_EQ(a, b);
}

TEST(Optional, SequenceContainerCtorNT) {
  using namespace ka;
  using O = opt_t<char>;
  ASSERT_EQ(O(0, 'a'), O());
  ASSERT_TRUE(O(0, 'a').empty());
  ASSERT_EQ(O(1, 'a'), O().set('a'));
  ASSERT_THROW(O(2, 'a'), std::length_error);
}

TEST(Optional, SequenceContainerCtorII) {
  using namespace ka;
  using O = opt_t<char>;
  char const x = 'a';
  ASSERT_EQ(O(&x, &x+0), O());
  ASSERT_TRUE(O(&x, &x+0).empty());
  ASSERT_EQ(O(&x, &x+1), O().set(x));
  ASSERT_THROW(O(&x, &x+2), std::length_error);
}

TEST(Optional, SequenceContainerCtorIL) {
  using namespace ka;
  using O = opt_t<char>;
  char const x = 'a';
  ASSERT_EQ(O({}), O());
  ASSERT_TRUE(O({}).empty());
  ASSERT_EQ(O({x}), O().set(x));
  ASSERT_THROW(O({x, x}), std::length_error);
}

TEST(Optional, SequenceContainerAssignmentIl) {
  using namespace ka;
  using O = opt_t<char>;
  char const x = 'a';
  O a;
#if KA_COMPILER_VS2013_OR_BELOW
  a = std::initializer_list<char>{}; // Explicit type needed by VS2013.
#else
  a = {};
#endif
  ASSERT_EQ(a, O());
  ASSERT_TRUE(a.empty());
  a = {x};
  ASSERT_EQ(a, O().set(x));
  ASSERT_THROW((a = {x, x}), std::length_error);
}

namespace {

struct X {
  X() = default;
  X(X const&) = delete;
  X(X&&) = delete;
  X& operator=(X const&) = delete;
  X& operator=(X&&) = delete;

  int a;
  bool b;
  char c;
  X(int a, bool b, char c) : a(a), b(b), c(c) {
  }

  bool operator==(X const& x) const {
    return a == x.a && b == x.b && c == x.c;
  }
};

} // namespace

TEST(OptionalEmplace, SequenceContainer) {
  using namespace ka;
  using O = opt_t<X>;
  int const a = 5;
  bool const b = true;
  char const c = 'j';
  O o;
  ASSERT_THROW(o.emplace(o.begin()+4, a, b, c), std::runtime_error); // invalid iterator
  O::const_iterator i = o.emplace(o.end(), a, b, c);
  ASSERT_EQ(i, o.begin());
  ASSERT_EQ(src(o).a, a);
  ASSERT_EQ(src(o).b, b);
  ASSERT_EQ(src(o).c, c);
  ASSERT_THROW(o.emplace(o.end(), a, b, c), std::length_error); // not empty
}

namespace {

struct Y {
  int a;
  bool b;
  char c;

  bool operator==(Y const& y) const {
    return a == y.a && b == y.b && c == y.c;
  }
};

} // namespace

TEST(Optional, SequenceContainerInsertPT) {
  using namespace ka;
  using O = opt_t<Y>;
  int const a = 5;
  bool const b = true;
  char const c = 'm';
  O o;
  Y const y{a, b, c};
  ASSERT_THROW(o.insert(o.begin()+4, y), std::runtime_error); // invalid iterator
  O::const_iterator i = o.insert(o.end(), y);
  ASSERT_EQ(i, o.begin());
  ASSERT_EQ(src(o).a, a);
  ASSERT_EQ(src(o).b, b);
  ASSERT_EQ(src(o).c, c);
  ASSERT_THROW(o.insert(o.end(), y), std::length_error); // not empty
}

TEST(Optional, SequenceContainerInsertPRv) {
  using namespace ka;
  using namespace functional_ops;
  using O = opt_t<std::unique_ptr<int>>;
  auto uniq = [](int x) {
    return std::unique_ptr<int>(new int(x));
  };
  int const x = 5;
  auto ssrc = src_t{} * src_t{};
  O o;
  ASSERT_THROW(o.insert(o.begin()+4, uniq(x)), std::runtime_error); // invalid iterator
  O::const_iterator it = o.insert(o.end(), uniq(x));
  ASSERT_EQ(it, o.begin());
  ASSERT_EQ(ssrc(o), x);
  ASSERT_THROW(o.insert(o.end(), uniq(x)), std::length_error); // not empty
}

TEST(Optional, SequenceContainerInsertPNT) {
  using namespace ka;
  using O = opt_t<Y>;
  int const a = 5;
  bool const b = true;
  char const c = 'm';
  Y const y{a, b, c};
  O o;
  ASSERT_THROW(o.insert(o.begin()+4, 1, y), std::runtime_error); // invalid iterator
  ASSERT_THROW(o.insert(o.begin(), 2, y), std::length_error); // n > 1
  O::const_iterator it;
  it = o.insert(o.end() + 10, 0, y); // insert 0 element
  ASSERT_EQ(it, o.end() + 10);
  it = o.insert(o.end(), 1, y);
  ASSERT_EQ(it, o.begin());
  ASSERT_EQ(src(o).a, a);
  ASSERT_EQ(src(o).b, b);
  ASSERT_EQ(src(o).c, c);
  ASSERT_THROW(o.insert(o.end(), 1, y), std::length_error); // not empty
}

TEST(Optional, SequenceContainerInsertPIJ) {
  using namespace ka;
  using O = opt_t<Y>;
  int const a = 5;
  bool const b = true;
  char const c = 'm';
  Y const y{a, b, c};
  O::const_iterator i = &y;
  O o;
  ASSERT_THROW(o.insert(o.begin()+4, i, i+1), std::runtime_error); // invalid iterator
  ASSERT_THROW(o.insert(o.begin(), i, i+2), std::length_error); // distance(i, j) > 1
  O::const_iterator it;
  it = o.insert(o.end() + 10, i, i); // insert 0 element (i == i)
  ASSERT_EQ(it, o.end() + 10);
  it = o.insert(o.end(), i, i+1);
  ASSERT_EQ(it, o.begin());
  ASSERT_EQ(src(o).a, a);
  ASSERT_EQ(src(o).b, b);
  ASSERT_EQ(src(o).c, c);
  ASSERT_THROW(o.insert(o.end(), i, i+1), std::length_error); // not empty
}

TEST(Optional, SequenceContainerInsertPIl) {
  using namespace ka;
  using O = opt_t<Y>;
  int const a = 5;
  bool const b = true;
  char const c = 'm';
  Y const y{a, b, c};
  O o;
  ASSERT_THROW(o.insert(o.begin()+4, {y}), std::runtime_error); // invalid iterator
  ASSERT_THROW(o.insert(o.begin(), {y, y}), std::length_error); // l.size() > 1
  O::const_iterator it;
  it = o.insert(o.end() + 10, {}); // insert 0 element
  ASSERT_EQ(it, o.end() + 10);
  it = o.insert(o.end(), {y});
  ASSERT_EQ(it, o.begin());
  ASSERT_EQ(src(o).a, a);
  ASSERT_EQ(src(o).b, b);
  ASSERT_EQ(src(o).c, c);
  ASSERT_THROW(o.insert(o.end(), {y}), std::length_error); // not empty
}

TEST(Optional, SequenceContainerEraseQ) {
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_THROW(o.erase(o.begin()), std::length_error); // empty
  int const x = 5;
  o.set(x);
  ASSERT_EQ(src(o), x);
  O::const_iterator e = o.end();
  ASSERT_THROW(o.erase(o.begin() + 1), std::runtime_error); // q != begin()
  ASSERT_EQ(o.erase(o.begin()), e);
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerEraseQ1Q2) {
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_EQ(o.erase(o.begin(), o.begin()), o.begin());
  ASSERT_THROW(o.erase(o.begin(), o.begin() + 2), std::length_error); // distance(q1, q2) > 1
  int const x = 5;
  o.set(x);
  ASSERT_EQ(src(o), x);
  auto const b = o.begin();
  auto const e = b + 1;
  ASSERT_EQ(o.erase(b, e), e);
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerClear) {
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_TRUE(o.empty());
  o.clear();
  ASSERT_TRUE(o.empty());
  int const x = 5;
  o.set(x);
  ASSERT_EQ(src(o), x);
  ASSERT_FALSE(o.empty());
  o.clear();
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerAssignNT) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  O o;
  ASSERT_TRUE(o.empty()) ;
  ASSERT_NO_THROW(o.assign(0, x)); // assign 0 element
  ASSERT_TRUE(o.empty()) ;
  ASSERT_THROW(o.assign(2, x), std::length_error); // n > 1
  o.assign(1, x);
  ASSERT_EQ(src(o), x);
  o.assign(1, x+1);
  ASSERT_EQ(src(o), x+1);
  o.assign(0, x);
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerAssignIJ) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  O::const_iterator i = &x;
  int const y = 18;
  O::const_iterator j = &y;
  O o;
  ASSERT_TRUE(o.empty()) ;
  ASSERT_NO_THROW(o.assign(i, i)); // assign 0 element
  ASSERT_TRUE(o.empty()) ;
  ASSERT_THROW(o.assign(i, i+2), std::length_error); // n > 1
  o.assign(i, i+1);
  ASSERT_EQ(src(o), x);
  o.assign(j, j+1);
  ASSERT_EQ(src(o), y);
  o.assign(j, j);
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerAssignIl) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  int const y = 18;
  O o;
  ASSERT_TRUE(o.empty()) ;
  ASSERT_NO_THROW(o.assign({})); // assign 0 element
  ASSERT_TRUE(o.empty()) ;
  ASSERT_THROW(o.assign({x, x}), std::length_error); // n > 1
  o.assign({x});
  ASSERT_EQ(src(o), x);
  o.assign({y});
  ASSERT_EQ(src(o), y);
  o.assign({});
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerFront) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  O o;
  {
    o.set(x);
    ASSERT_EQ(o.front(), x);
    o.front() = x+1;
    ASSERT_EQ(o.front(), x+1);
  }
  {
    O const& p = o;
    ASSERT_EQ(p.front(), x+1);
  }
}

TEST(Optional, SequenceContainerBack) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  O o;
  {
    o.set(x);
    ASSERT_EQ(o.back(), x);
    o.front() = x+1;
    ASSERT_EQ(o.back(), x+1);
  }
  {
    O const& p = o;
    ASSERT_EQ(p.back(), x+1);
  }
}

namespace {

struct emplace_front_t {
  template<typename O, typename... Args>
  void operator()(O& o, Args&&... args) const {
    o.emplace_front(ka::fwd<Args>(args)...);
  }
};

struct emplace_back_t {
  template<typename O, typename... Args>
  void operator()(O& o, Args&&... args) const {
    o.emplace_back(ka::fwd<Args>(args)...);
  }
};

} // namespace

template<typename T>
struct OptionalEmplaceFrontBack : testing::Test {};

using emplace_types = testing::Types<emplace_front_t, emplace_back_t>;

TYPED_TEST_SUITE(OptionalEmplaceFrontBack, emplace_types);

TYPED_TEST(OptionalEmplaceFrontBack, SequenceContainer) {
  TypeParam emplace;
  using namespace ka;
  using O = opt_t<X>;
  int const a = 5;
  bool const b = true;
  char const c = 'j';
  O o;
  emplace(o, a, b, c);
  ASSERT_EQ(src(o).a, a);
  ASSERT_EQ(src(o).b, b);
  ASSERT_EQ(src(o).c, c);
  ASSERT_THROW(emplace(o, a, b, c), std::length_error); // not empty
}

namespace {

struct push_front_t {
  template<typename O, typename U>
  void operator()(O& o, U&& u) const {
    o.push_front(ka::fwd<U>(u));
  }
};

struct push_back_t {
  template<typename O, typename U>
  void operator()(O& o, U&& u) const {
    o.push_back(ka::fwd<U>(u));
  }
};

} // namespace

template<typename T>
struct OptionalPushFrontBack : testing::Test {};

using push_types = testing::Types<push_front_t, push_back_t>;

TYPED_TEST_SUITE(OptionalPushFrontBack, push_types);

TYPED_TEST(OptionalPushFrontBack, SequenceContainer) {
  TypeParam push;
  using namespace ka;
  using namespace functional_ops;
  using O = opt_t<std::unique_ptr<int>>;
  auto uniq = [](int x) {
    return std::unique_ptr<int>(new int(x));
  };
  int const x = 5;
  auto ssrc = src_t{} * src_t{};
  O o;
  push(o, uniq(x));
  ASSERT_EQ(ssrc(o), x);
  ASSERT_THROW(push(o, uniq(x)), std::length_error); // not empty
}

namespace {

struct pop_front_t {
  template<typename O>
  void operator()(O& o) const {
    o.pop_front();
  }
};

struct pop_back_t {
  template<typename O>
  void operator()(O& o) const {
    o.pop_back();
  }
};

} // namespace

template<typename T>
struct OptionalPopFrontBack : testing::Test {};

using pop_types = testing::Types<pop_front_t, pop_back_t>;

TYPED_TEST_SUITE(OptionalPopFrontBack, pop_types);

TYPED_TEST(OptionalPopFrontBack, SequenceContainer) {
  TypeParam pop;
  using namespace ka;
  using O = opt_t<int>;
  O o;
  ASSERT_THROW(pop(o), std::length_error); // empty
  int const x = 5;
  o.set(x);
  ASSERT_EQ(src(o), x);
  ASSERT_FALSE(o.empty());
  pop(o);
  ASSERT_TRUE(o.empty());
}

TEST(Optional, SequenceContainerBrackets) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  O o;
  ASSERT_NO_THROW(o[0]); // out of bounds, but no check
  {
    o.set(x);
    ASSERT_EQ(o[0], x);
    o[0] = x+1;
    ASSERT_EQ(o[0], x+1);
  }
  {
    O const& p = o;
    ASSERT_EQ(p[0], x+1);
  }
}

TEST(Optional, SequenceContainerAt) {
  using namespace ka;
  using O = opt_t<int>;
  int const x = 5;
  O o;
  ASSERT_THROW(o.at(0), std::out_of_range);
  {
    o.set(x);
    ASSERT_EQ(o.at(0), x);
    o.at(0) = x+1;
    ASSERT_EQ(o.at(0), x+1);
  }
  {
    O const& p = o;
    ASSERT_EQ(p.at(0), x+1);
  }
  ASSERT_THROW(o.at(1), std::out_of_range);
}

namespace {
  auto const g = [](ka::test::C c) {
    return ka::test::D{2 * c.value};
  };
  auto const g_void = [](ka::test::C) -> void {
  };
}

TEST(Optional, FunctorBoostOptional) {
  using namespace ka::test;
  using OptC = boost::optional<C>;
  using OptD = boost::optional<D>;
  auto c = C{3};

  // g: C -> D
  EXPECT_EQ(OptD(g(c)), ka::fmap(g, OptC(c))); // not empty
  EXPECT_EQ(OptD(), ka::fmap(g, OptC())); // empty
}

TEST(Optional, FunctorKaOptMemberFmapNonVoidToNonVoid) {
  using namespace ka;
  using namespace ka::test;
  auto c = C{3};

  // g: C -> D
  EXPECT_EQ(opt(g(c)), opt(c).fmap(g)); // not empty
  EXPECT_EQ(opt_t<D>(), opt_t<C>().fmap(g)); // empty
}

TEST(Optional, FunctorKaOptMemberFmapNonVoidToVoid) {
  using namespace ka;
  using namespace ka::test;
  auto c = C{3};

  // g_void: C -> void
  EXPECT_EQ(opt_t<void>().set(), opt(c).fmap(g_void)); // not empty
  EXPECT_EQ(opt_t<void>(), opt_t<C>().fmap(g_void)); // empty
}

// "Mapping on void" implies executing a side-effect procedure.
TEST(Optional, FunctorKaOptMemberFmapVoidToVoid) {
  using namespace ka;
  using namespace ka::test;
  using test::A;

  int i = 0;
  std::vector<A> v;
  auto h = [&](void) -> void {
    v.push_back(A{i++}); // side-effect
  };

  EXPECT_TRUE(v.empty());
  EXPECT_EQ(opt(), opt().fmap(h));    // not empty
  EXPECT_EQ(v, std::vector<A>{A{0}}); // procedure executed

  v.clear();
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(opt_t<void>(), opt_t<void>().fmap(h)); // empty
  EXPECT_TRUE(v.empty());                          // procedure not executed
}

TEST(Optional, FunctorKaOptMemberFmapVoidToNonVoid) {
  using namespace ka;
  using ka::test::A;
  auto h = [](void) -> A {
    return A{4};
  };
  EXPECT_EQ(opt(A{4}), opt().fmap(h));    // not empty
  EXPECT_EQ(opt_t<A>(), opt_t<void>().fmap(h)); // empty
}

TEST(Optional, FunctorAppKaOptNonVoidToNonVoid) {
  using namespace ka;
  using namespace ka::test;
  auto const h = [](ka::test::C c, ka::test::D d) -> ka::test::E {
    return ka::test::E{c.value * d.value};
  };
  auto c = C{3};
  auto d = D{4};

  // h: C x D -> E
  EXPECT_EQ(opt(h(c, d)), fmap(h, opt(c), opt(d))); // not empty
  EXPECT_EQ(opt_t<E>(), fmap(h, opt_t<C>(), opt(d))); // first empty
  EXPECT_EQ(opt_t<E>(), fmap(h, opt(c), opt_t<D>())); // second empty
  EXPECT_EQ(opt_t<E>(), fmap(h, opt_t<C>(), opt_t<D>())); // both empty
}

TEST(Optional, FunctorAppKaOptNonVoidToVoid) {
  using namespace ka;
  using namespace ka::test;
  std::vector<A> v;
  auto const h_void = [&](ka::test::C c, ka::test::D d) -> void {
    v.push_back(A{c.value * d.value}); // side-effect
  };
  auto c = C{3};
  auto d = D{4};

  // h_void: C x D -> void
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(opt_t<void>().set(), fmap(h_void, opt(c), opt(d))); // not empty
  EXPECT_EQ(v, std::vector<A>{A{c.value * d.value}}); // procedure executed
  v.clear();
  EXPECT_EQ(opt_t<void>(), fmap(h_void, opt_t<C>(), opt(d))); // first empty
  EXPECT_TRUE(v.empty()); // procedure not executed
  EXPECT_EQ(opt_t<void>(), fmap(h_void, opt(c), opt_t<D>())); // second empty
  EXPECT_TRUE(v.empty()); // procedure not executed
  EXPECT_EQ(opt_t<void>(), fmap(h_void, opt_t<C>(), opt_t<D>())); // both empty
  EXPECT_TRUE(v.empty()); // procedure not executed
}

namespace {
  template<template<typename> class O>
  struct opt_traits;

  template<>
  struct opt_traits<boost::optional> {
    template<typename T> using type = boost::optional<T>;
    template<typename T>
    static type<ka::Decay<T>> make(T&& t) {
      return { ka::fwd<T>(t) };
    }
  };

  template<>
  struct opt_traits<ka::opt_t> {
    template<typename T> using type = ka::opt_t<T>;
    template<typename T>
    static type<ka::Decay<T>> make(T&& t) {
      return ka::opt(ka::fwd<T>(t));
    }
  };
}

template<typename Traits>
struct OptionalFlatten : testing::Test {
  using traits = Traits;
  template<typename T>
  using Opt = typename traits::template type<T>;
};

using opt_types = testing::Types<opt_traits<boost::optional>, opt_traits<ka::opt_t>>;

TYPED_TEST_SUITE(OptionalFlatten, opt_types);

// Internal optional is extracted.
TYPED_TEST(OptionalFlatten, NonEmpty) {
  using namespace ka;
  using F = TestFixture;
  using test::A;
  using OptA = typename F::template Opt<A>;
  using OptOptA = typename F::template Opt<OptA>;

  OptOptA o0 = F::traits::make(F::traits::make(A{934}));
  OptA const o1 = flatten(o0);
  ASSERT_FALSE(empty(o0));
  EXPECT_EQ(o1, *o0);
  ASSERT_FALSE(empty(o1));
  EXPECT_EQ(A{934}, *o1);
}

// Empty optional is extracted.
TYPED_TEST(OptionalFlatten, Empty) {
  using namespace ka;
  using F = TestFixture;
  using test::A;
  using OptA = typename F::template Opt<A>;
  using OptOptA = typename F::template Opt<OptA>;

  OptOptA o0;
  EXPECT_TRUE(empty(o0));
  OptA const o1 = flatten(o0);
  EXPECT_TRUE(empty(o1));
}
