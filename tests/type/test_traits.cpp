/*
** Copyright (C) 2016 Aldebaran Robotics
*/

#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qi/type/traits.hpp>
#include <boost/function.hpp>
#include <boost/config/suffix.hpp>
#include <qi/macro.hpp>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <boost/container/small_vector.hpp>
#include <boost/container/slist.hpp>

struct A {};

TEST(TestTraits, RemoveRef)
{
  using namespace qi::traits;

  static_assert(Equal<RemoveRef<int>, int>::value, "");
  static_assert(Equal<RemoveRef<int&>, int>::value, "");
  static_assert(Equal<RemoveRef<int const&>, int const>::value, "");
  static_assert(Equal<RemoveRef<int*>, int*>::value, "");
  static_assert(Equal<RemoveRef<A*&>, A*>::value, "");
  static_assert(Equal<RemoveRef<int&&>, int>::value, "");

  // member functions
  static_assert(Equal<RemoveRef<int (A::*)()>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) const>, int (A::*)(float const&) const>::value, "");

#if QI_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // member functions (lvalue ref)
  static_assert(Equal<RemoveRef<int (A::*)() &>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) &>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) const &>, int (A::*)(float const&) const>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) volatile &>, int (A::*)(float const&) volatile>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) const volatile &>, int (A::*)(float const&) const volatile>::value, "");
  static_assert(Equal<RemoveRef<int const& (A::*)(float const&) const volatile &>, int const& (A::*)(float const&) const volatile>::value, "");

  // member functions (rvalue ref)
  static_assert(Equal<RemoveRef<int (A::*)() &&>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) &&>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) const &&>, int (A::*)(float const&) const>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) volatile &&>, int (A::*)(float const&) volatile>::value, "");
  static_assert(Equal<RemoveRef<int (A::*)(float const&) const volatile &&>, int (A::*)(float const&) const volatile>::value, "");
  static_assert(Equal<RemoveRef<int const& (A::*)(float const&) const volatile &&>, int const& (A::*)(float const&) const volatile>::value, "");
#endif
}

TEST(TestTraits, RemoveCv)
{
  using namespace qi::traits;

  // simple types
  static_assert(Equal<RemoveCv<int>, int>::value, "");
  static_assert(Equal<RemoveCv<int&>, int&>::value, "");
  static_assert(Equal<RemoveCv<int const>, int>::value, "");
  static_assert(Equal<RemoveCv<int volatile>, int>::value, "");
  static_assert(Equal<RemoveCv<int const volatile>, int>::value, "");
  static_assert(Equal<RemoveCv<int const&>, int const&>::value, "");
  static_assert(Equal<RemoveCv<int const*>, int const*>::value, "");
  static_assert(Equal<RemoveCv<int const* const>, int const*>::value, "");

  // member functions
  static_assert(Equal<RemoveCv<int (A::*)()>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&)>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) const>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) volatile>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) const volatile>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int const& (A::*)(float const&) const volatile>, int const& (A::*)(float const&)>::value, "");

#if QI_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // member functions (lvalue ref)
  static_assert(Equal<RemoveCv<int (A::*)() &>, int (A::*)() &>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) &>, int (A::*)(float const&) &>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) const &>, int (A::*)(float const&) const &>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) volatile &>, int (A::*)(float const&) volatile &>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) const volatile &>, int (A::*)(float const&) const volatile &>::value, "");
  static_assert(Equal<RemoveCv<int const& (A::*)(float const&) const volatile &>, int const& (A::*)(float const&) const volatile &>::value, "");

  // member functions (rvalue ref)
  static_assert(Equal<RemoveCv<int (A::*)() &&>, int (A::*)() &&>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) &&>, int (A::*)(float const&) &&>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) const &&>, int (A::*)(float const&) const &&>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) volatile &&>, int (A::*)(float const&) volatile &&>::value, "");
  static_assert(Equal<RemoveCv<int (A::*)(float const&) const volatile &&>, int (A::*)(float const&) const volatile &&>::value, "");
  static_assert(Equal<RemoveCv<int const& (A::*)(float const&) const volatile &&>, int const& (A::*)(float const&) const volatile &&>::value, "");
#endif
}

TEST(TestTraits, RemoveCvRef)
{
  using namespace qi::traits;

  // simple types
  static_assert(Equal<RemoveCvRef<int>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int&>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int const>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int volatile>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int const volatile>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int const&>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int const*>, int const*>::value, "");
  static_assert(Equal<RemoveCvRef<int const* const>, int const*>::value, "");

  // member functions
  static_assert(Equal<RemoveCvRef<int (A::*)()>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&)>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) const>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) volatile>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) const volatile>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int const& (A::*)(float const&) const volatile>, int const& (A::*)(float const&)>::value, "");

#if QI_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // member functions (lvalue ref)
  static_assert(Equal<RemoveCvRef<int (A::*)() &>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) &>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) const &>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) volatile &>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) const volatile &>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int const& (A::*)(float const&) const volatile &>, int const& (A::*)(float const&)>::value, "");

  // member functions (rvalue ref)
  static_assert(Equal<RemoveCvRef<int (A::*)() &&>, int (A::*)()>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) &&>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) const &&>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) volatile &&>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (A::*)(float const&) const volatile &&>, int (A::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int const& (A::*)(float const&) const volatile &&>, int const& (A::*)(float const&)>::value, "");
#endif

  static_assert(Equal<RemoveCvRef<int>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int&>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int const&>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int*>, int*>::value, "");
  static_assert(Equal<RemoveCvRef<A*&>, A*>::value, "");
  static_assert(Equal<RemoveCvRef<int&&>, int>::value, "");
}

TEST(TestTraits, HasMemberOperatorCallLambda)
{
  using namespace qi::traits;
  { // without return nor parameter
    auto f = []() {};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // no capture
    auto f = []() {return 0;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // capture
    int i{0};
    auto f = [=]() {return i;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // mutable
    int i{0};
    auto f = [=]() mutable {return i++;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // BOOST_NOEXCEPT instead of noexcept because VS2013 does not support noexcept.
    auto f = []() BOOST_NOEXCEPT {return 0;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // parameter by value
    auto f = [](int i) {return i;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // parameter by reference
    auto f = [](int& i) {++i; return i;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // parameter by constant reference
    auto f = [](int const& i) {return i * 2;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // multiple parameters
    auto f = [](int const& i, float* f) {return i * (*f);};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
  { // multiple parameters mutable
    int n{2};
    auto f = [&](int const& i, float* f) mutable {return i * (n++) * (*f);};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
}

struct NoOpCall {
};

struct VoidVoid {
  void operator()() {}
};

struct IntVoid {
  int operator()() {return 0;}
};

struct IntVoidConst {
  int operator()() const {return 0;}
};

struct IntVoidVolatile {
  int operator()() volatile {return 0;}
};

struct IntVoidConstVolatile {
  int operator()() const volatile {return 0;}
};

struct VirtualIntVoidConst {
  virtual int operator()() const {return 0;}
};

struct IntVoidIntRefAConst {
  int operator()(int&, A) const {return 0;}
};

struct IntFloatMutableAndConst {
  int operator()(float) {return 0;}
  int operator()(float) const {return 0;}
};

struct IntConstPtrFloatConst {
  int const* operator()(float) const {static const int i{0}; return &i;}
};

TEST(TestTraits, HasMemberOperatorCallClass)
{
  using namespace qi::traits;
  static_assert(!HasMemberOperatorCall<NoOpCall>::value, "");
  static_assert( HasMemberOperatorCall<VoidVoid>::value, "");
  static_assert( HasMemberOperatorCall<IntVoid>::value, "");
  static_assert( HasMemberOperatorCall<IntVoidConst>::value, "");
  static_assert( HasMemberOperatorCall<IntVoidVolatile>::value, "");
  static_assert( HasMemberOperatorCall<IntVoidConstVolatile>::value, "");
  static_assert( HasMemberOperatorCall<VirtualIntVoidConst>::value, "");
  static_assert( HasMemberOperatorCall<IntVoidIntRefAConst>::value, "");
  // This must fail because the type must have only one operator call.
  static_assert(!HasMemberOperatorCall<IntFloatMutableAndConst>::value, "");
  static_assert( HasMemberOperatorCall<boost::function<void (void)>>::value, "");
  static_assert( HasMemberOperatorCall<boost::function<void (int)>>::value, "");
  static_assert( HasMemberOperatorCall<boost::function<int const& (int, A*, float const&, double&)>>::value, "");
}

TEST(TestTraits, HasMemberOperatorCallBuiltin)
{
  using namespace qi::traits;

  // functions
  static_assert(!HasMemberOperatorCall<void (void)>::value, "");
  static_assert(!HasMemberOperatorCall<void (int&)>::value, "");
  static_assert(!HasMemberOperatorCall<A (int&, float, double*)>::value, "");

  // function pointers
  static_assert(!HasMemberOperatorCall<void (*)(int&)>::value, "");
  static_assert(!HasMemberOperatorCall<void (*)(int&)>::value, "");
  static_assert(!HasMemberOperatorCall<A (*)(int&, float, double*)>::value, "");

  // member function pointers
  static_assert(!HasMemberOperatorCall<void (A::*)()>::value, "");
  static_assert(!HasMemberOperatorCall<void (A::*)() const>::value, "");
  static_assert(!HasMemberOperatorCall<void (A::*)() volatile>::value, "");
  static_assert(!HasMemberOperatorCall<void (A::*)() const volatile>::value, "");
  static_assert(!HasMemberOperatorCall<float (A::*)(int)>::value, "");
  static_assert(!HasMemberOperatorCall<float (A::*)(int, double&, A const&)>::value, "");
}

TEST(TestTraits, IsFunctionObjectLambda)
{
  using namespace qi::traits;
  { // without return nor parameter
    auto f = []() {};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // no capture
    auto f = []() {return 0;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // capture
    int i{0};
    auto f = [=] {return i;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // mutable
    int i{0};
    auto f = [=]() mutable {return i++;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // parameter by value
    auto f = [](int i) {return i;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // parameter by reference
    auto f = [](int& i) {++i; return i;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // parameter by constant reference
    auto f = [](int const& i) {return i * 2;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // multiple parameters
    auto f = [](int const& i, float* f) {return i * (*f);};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
  { // multiple parameters mutable
    int n{2};
    auto f = [&](int const& i, float* f) mutable {return i * (n++) * (*f);};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
}

TEST(TestTraits, IsFunctionObjectClass)
{
  using namespace qi::traits;
  static_assert(!IsFunctionObject<NoOpCall>::value, "");
  static_assert( IsFunctionObject<VoidVoid>::value, "");
  static_assert( IsFunctionObject<IntVoid>::value, "");
  static_assert( IsFunctionObject<IntVoidConst>::value, "");
  static_assert( IsFunctionObject<IntVoidVolatile>::value, "");
  static_assert( IsFunctionObject<IntVoidConstVolatile>::value, "");
  static_assert( IsFunctionObject<VirtualIntVoidConst>::value, "");
  static_assert( IsFunctionObject<IntVoidIntRefAConst>::value, "");
  // This must fail because the type must have only one operator call.
  static_assert(!IsFunctionObject<IntFloatMutableAndConst>::value, "");
  static_assert( IsFunctionObject<boost::function<void (void)>>::value, "");
  static_assert( IsFunctionObject<boost::function<void (int)>>::value, "");
  static_assert( IsFunctionObject<boost::function<int const& (int, A*, float const&, double&)>>::value, "");
}

TEST(TestTraits, IsFunctionObjectBuiltin)
{
  using namespace qi::traits;

  // functions
  static_assert(!IsFunctionObject<void (void)>::value, "");
  static_assert(!IsFunctionObject<void (int&)>::value, "");
  static_assert(!IsFunctionObject<A (int&, float, double*)>::value, "");

  // function pointers
  static_assert(!IsFunctionObject<void (*)(int&)>::value, "");
  static_assert(!IsFunctionObject<void (*)(int&)>::value, "");
  static_assert(!IsFunctionObject<A (*)(int&, float, double*)>::value, "");

  // member function pointers
  static_assert(!IsFunctionObject<void (A::*)()>::value, "");
  static_assert(!IsFunctionObject<void (A::*)() const>::value, "");
  static_assert(!IsFunctionObject<void (A::*)() volatile>::value, "");
  static_assert(!IsFunctionObject<void (A::*)() const volatile>::value, "");
  static_assert(!IsFunctionObject<float (A::*)(int)>::value, "");
  static_assert(!IsFunctionObject<float (A::*)(int, double&, A const&)>::value, "");
}

TEST(TestTraits, FunctionLambda)
{
  using namespace qi::traits;
  {
    auto f = []() {};
    static_assert(Equal<Function<decltype(f)>, void (void)>::value, "");
  }
  {
    auto f = [](int) {};
    static_assert(Equal<Function<decltype(f)>, void (int)>::value, "");
  }
  {
    auto f = [](int&) {};
    static_assert(Equal<Function<decltype(f)>, void (int&)>::value, "");
  }
  {
    auto f = [](int const&) {};
    static_assert(Equal<Function<decltype(f)>, void (int const&)>::value, "");
  }
  {
    auto f = [](int, float) {};
    static_assert(Equal<Function<decltype(f)>, void (int, float)>::value, "");
  }
  {
    auto f = [](int*, float&) {};
    static_assert(Equal<Function<decltype(f)>, void (int*, float&)>::value, "");
  }
  {
    auto f = [](int*, float&) mutable {};
    static_assert(Equal<Function<decltype(f)>, void (int*, float&)>::value, "");
  }
  {
    auto f = [](int*, float&) mutable {return 0;};
    static_assert(Equal<Function<decltype(f)>, int (int*, float&)>::value, "");
  }
}

TEST(TestTraits, FunctionClass)
{
  using namespace qi::traits;
  using boost::function;
  static_assert( Equal<Function<VoidVoid>, void (void)>::value, "");
  static_assert( Equal<Function<IntVoid>, int (void)>::value, "");
  static_assert( Equal<Function<IntVoidConst>, int (void)>::value, "");
  static_assert( Equal<Function<IntVoidVolatile>, int (void)>::value, "");
  static_assert( Equal<Function<IntVoidConstVolatile>, int (void)>::value, "");
  static_assert( Equal<Function<VirtualIntVoidConst>, int (void)>::value, "");
  static_assert( Equal<Function<IntVoidIntRefAConst>, int (int&, A)>::value, "");
  static_assert( Equal<Function<function<void (void)>>, void (void)>::value, "");
  static_assert( Equal<Function<function<void (int)>>, void (int)>::value, "");
  static_assert( Equal<Function<function<int const& (int, A*, float const&, double&)>>,
                    int const& (int, A*, float const&, double&)>::value, "");
}

TEST(TestTraits, FunctionBuiltin)
{
  using namespace qi::traits;
  using boost::function;

  // functions
  static_assert( Equal<Function<void (void)>, void (void)>::value, "");
  static_assert( Equal<Function<int (void)>, int (void)>::value, "");
  static_assert( Equal<Function<int (int&, A)>, int (int&, A)>::value, "");
  static_assert( Equal<Function<float const& (int*, A)>, float const& (int*, A)>::value, "");

  // function pointers
  static_assert( Equal<Function<void (*)(void)>, void (void)>::value, "");
  static_assert( Equal<Function<int (*)(void)>, int (void)>::value, "");
  static_assert( Equal<Function<int (*)(int&, A)>, int (int&, A)>::value, "");
  static_assert( Equal<Function<float const& (*)(int*, A)>, float const& (int*, A)>::value, "");
}

struct MyContiguousContainer
{
};

namespace qi{ namespace traits{ namespace detail{
  template<>
  struct IsContiguous<MyContiguousContainer>
  {
    using type = True;
  };
}}}

TEST(TestTraits, IsContiguous)
{
  using namespace qi::traits;

  // non containers
  static_assert(!IsContiguous<void>::value, "");
  static_assert(!IsContiguous<int>::value, "");
  static_assert(!IsContiguous<int*>::value, "");
  static_assert(!IsContiguous<int&>::value, "");
  static_assert(!IsContiguous<A>::value, "");

  // std "containers"
  static_assert( IsContiguous<std::string>::value, "");
  static_assert( IsContiguous<std::array<int, 3>>::value, "");
  static_assert( IsContiguous<std::vector<int>>::value, "");
  static_assert(!IsContiguous<std::list<int>>::value, "");
  static_assert(!IsContiguous<std::forward_list<int>>::value, "");
  static_assert(!IsContiguous<std::deque<int>>::value, "");
  static_assert(!IsContiguous<std::map<int, char>>::value, "");
  static_assert(!IsContiguous<std::multimap<int, char>>::value, "");
  static_assert(!IsContiguous<std::unordered_multimap<int, char>>::value, "");
  static_assert(!IsContiguous<std::set<int>>::value, "");
  static_assert(!IsContiguous<std::multiset<int>>::value, "");
  static_assert(!IsContiguous<std::unordered_set<int>>::value, "");

  // boost containers
  static_assert( IsContiguous<boost::container::static_vector<int, 3>>::value, "");
  static_assert( IsContiguous<boost::container::small_vector<int, 3>>::value, "");
  static_assert(!IsContiguous<boost::container::slist<int>>::value, "");
  static_assert( IsContiguous<boost::container::flat_set<int>>::value, "");
  static_assert( IsContiguous<boost::container::flat_multiset<int>>::value, "");
  static_assert( IsContiguous<boost::container::flat_map<int, char>>::value, "");
  static_assert( IsContiguous<boost::container::flat_multimap<int, char>>::value, "");

  // custom
  static_assert( IsContiguous<MyContiguousContainer>::value, "");
}

TEST(TestTraits, IsContiguousLike)
{
  using namespace qi::traits;

  // non containers
  static_assert(!IsContiguousLike<void>::value, "");
  static_assert(!IsContiguousLike<int>::value, "");
  static_assert(!IsContiguousLike<int*>::value, "");
  static_assert(!IsContiguousLike<int&>::value, "");
  static_assert(!IsContiguousLike<A>::value, "");

  // std "containers"
  static_assert( IsContiguousLike<std::string>::value, "");
  static_assert( IsContiguousLike<std::array<int, 3>>::value, "");
  static_assert( IsContiguousLike<std::vector<int>>::value, "");
  static_assert(!IsContiguousLike<std::list<int>>::value, "");
  static_assert(!IsContiguousLike<std::forward_list<int>>::value, "");
  static_assert( IsContiguousLike<std::deque<int>>::value, "");
  static_assert(!IsContiguousLike<std::map<int, char>>::value, "");
  static_assert(!IsContiguousLike<std::multimap<int, char>>::value, "");
  static_assert(!IsContiguousLike<std::unordered_multimap<int, char>>::value, "");
  static_assert(!IsContiguousLike<std::set<int>>::value, "");
  static_assert(!IsContiguousLike<std::multiset<int>>::value, "");
  static_assert(!IsContiguousLike<std::unordered_set<int>>::value, "");

  // boost containers
  static_assert( IsContiguousLike<boost::container::static_vector<int, 3>>::value, "");
  static_assert( IsContiguousLike<boost::container::small_vector<int, 3>>::value, "");
  static_assert(!IsContiguousLike<boost::container::slist<int>>::value, "");
  static_assert( IsContiguousLike<boost::container::flat_set<int>>::value, "");
  static_assert( IsContiguousLike<boost::container::flat_multiset<int>>::value, "");
  static_assert( IsContiguousLike<boost::container::flat_map<int, char>>::value, "");
  static_assert( IsContiguousLike<boost::container::flat_multimap<int, char>>::value, "");

  // custom
  static_assert( IsContiguousLike<MyContiguousContainer>::value, "");
}

struct MyList
{
};

namespace qi{ namespace traits{ namespace detail{
  template<>
  struct IsList<MyList>
  {
    using type = True;
  };
}}}

TEST(TestTraits, IsList)
{
  using namespace qi::traits;

  // Non "containers"
  static_assert(!IsList<void>::value, "");
  static_assert(!IsList<int>::value, "");
  static_assert(!IsList<int*>::value, "");
  static_assert(!IsList<int&>::value, "");
  static_assert(!IsList<A>::value, "");

  // std "containers"
  static_assert(!IsList<std::string>::value, "");
  static_assert(!IsList<std::array<int, 3>>::value, "");
  static_assert(!IsList<std::vector<int>>::value, "");
  static_assert( IsList<std::list<int>>::value, "");
  static_assert( IsList<std::forward_list<int>>::value, "");
  static_assert(!IsList<std::deque<int>>::value, "");
  static_assert(!IsList<std::map<int, char>>::value, "");
  static_assert(!IsList<std::multimap<int, char>>::value, "");
  static_assert(!IsList<std::unordered_multimap<int, char>>::value, "");
  static_assert(!IsList<std::set<int>>::value, "");
  static_assert(!IsList<std::multiset<int>>::value, "");
  static_assert(!IsList<std::unordered_set<int>>::value, "");

  // boost containers
  static_assert(!IsList<boost::container::static_vector<int, 3>>::value, "");
  static_assert(!IsList<boost::container::small_vector<int, 3>>::value, "");
  static_assert( IsList<boost::container::slist<int>>::value, "");
  static_assert(!IsList<boost::container::flat_set<int>>::value, "");
  static_assert(!IsList<boost::container::flat_multiset<int>>::value, "");
  static_assert(!IsList<boost::container::flat_map<int, char>>::value, "");
  static_assert(!IsList<boost::container::flat_multimap<int, char>>::value, "");

  // custom
  static_assert( IsList<MyList>::value, "");
}
