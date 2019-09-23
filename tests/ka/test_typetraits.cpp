#include <gtest/gtest.h>
#include <ka/typetraits.hpp>
#include <boost/function.hpp>
#include <boost/config/suffix.hpp>
#include <ka/macro.hpp>
#include <string>
#include <array>
#include <iterator>
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
#include <ka/mutablestore.hpp>
#include <ka/utility.hpp>
#include <ka/macroregular.hpp>
#include "../ka/test_functional_common.hpp"

struct a_t {};

TEST(TypeTraits, RemoveRef) {
  using namespace ka;

  static_assert(Equal<RemoveRef<int>, int>::value, "");
  static_assert(Equal<RemoveRef<int&>, int>::value, "");
  static_assert(Equal<RemoveRef<int const&>, int const>::value, "");
  static_assert(Equal<RemoveRef<int*>, int*>::value, "");
  static_assert(Equal<RemoveRef<a_t*&>, a_t*>::value, "");
  static_assert(Equal<RemoveRef<int&&>, int>::value, "");

  // member functions
  static_assert(Equal<RemoveRef<int (a_t::*)()>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) const>, int (a_t::*)(float const&) const>::value, "");

#if KA_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // member functions (lvalue ref)
  static_assert(Equal<RemoveRef<int (a_t::*)() &>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) &>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) const &>, int (a_t::*)(float const&) const>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) volatile &>, int (a_t::*)(float const&) volatile>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) const volatile &>, int (a_t::*)(float const&) const volatile>::value, "");
  static_assert(Equal<RemoveRef<int const& (a_t::*)(float const&) const volatile &>, int const& (a_t::*)(float const&) const volatile>::value, "");

  // member functions (rvalue ref)
  static_assert(Equal<RemoveRef<int (a_t::*)() &&>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) &&>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) const &&>, int (a_t::*)(float const&) const>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) volatile &&>, int (a_t::*)(float const&) volatile>::value, "");
  static_assert(Equal<RemoveRef<int (a_t::*)(float const&) const volatile &&>, int (a_t::*)(float const&) const volatile>::value, "");
  static_assert(Equal<RemoveRef<int const& (a_t::*)(float const&) const volatile &&>, int const& (a_t::*)(float const&) const volatile>::value, "");
#endif
}

TEST(TypeTraits, RemoveCv) {
  using namespace ka;

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
  static_assert(Equal<RemoveCv<int (a_t::*)()>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&)>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) const>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) volatile>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) const volatile>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCv<int const& (a_t::*)(float const&) const volatile>, int const& (a_t::*)(float const&)>::value, "");

#if KA_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // member functions (lvalue ref)
  static_assert(Equal<RemoveCv<int (a_t::*)() &>, int (a_t::*)() &>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) &>, int (a_t::*)(float const&) &>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) const &>, int (a_t::*)(float const&) const &>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) volatile &>, int (a_t::*)(float const&) volatile &>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) const volatile &>, int (a_t::*)(float const&) const volatile &>::value, "");
  static_assert(Equal<RemoveCv<int const& (a_t::*)(float const&) const volatile &>, int const& (a_t::*)(float const&) const volatile &>::value, "");

  // member functions (rvalue ref)
  static_assert(Equal<RemoveCv<int (a_t::*)() &&>, int (a_t::*)() &&>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) &&>, int (a_t::*)(float const&) &&>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) const &&>, int (a_t::*)(float const&) const &&>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) volatile &&>, int (a_t::*)(float const&) volatile &&>::value, "");
  static_assert(Equal<RemoveCv<int (a_t::*)(float const&) const volatile &&>, int (a_t::*)(float const&) const volatile &&>::value, "");
  static_assert(Equal<RemoveCv<int const& (a_t::*)(float const&) const volatile &&>, int const& (a_t::*)(float const&) const volatile &&>::value, "");
#endif
}

TEST(TypeTraits, RemoveCvRef) {
  using namespace ka;

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
  static_assert(Equal<RemoveCvRef<int (a_t::*)()>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&)>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) const>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) volatile>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) const volatile>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int const& (a_t::*)(float const&) const volatile>, int const& (a_t::*)(float const&)>::value, "");

#if KA_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
  // member functions (lvalue ref)
  static_assert(Equal<RemoveCvRef<int (a_t::*)() &>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) &>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) const &>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) volatile &>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) const volatile &>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int const& (a_t::*)(float const&) const volatile &>, int const& (a_t::*)(float const&)>::value, "");

  // member functions (rvalue ref)
  static_assert(Equal<RemoveCvRef<int (a_t::*)() &&>, int (a_t::*)()>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) &&>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) const &&>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) volatile &&>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int (a_t::*)(float const&) const volatile &&>, int (a_t::*)(float const&)>::value, "");
  static_assert(Equal<RemoveCvRef<int const& (a_t::*)(float const&) const volatile &&>, int const& (a_t::*)(float const&)>::value, "");
#endif

  static_assert(Equal<RemoveCvRef<int>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int&>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int const&>, int>::value, "");
  static_assert(Equal<RemoveCvRef<int*>, int*>::value, "");
  static_assert(Equal<RemoveCvRef<a_t*&>, a_t*>::value, "");
  static_assert(Equal<RemoveCvRef<int&&>, int>::value, "");
}

TEST(TypeTraits, HasMemberOperatorCallLambda) {
  using namespace ka;
  { // without return nor parameter
    auto f = []() {};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // no capture
    auto f = []() {return 0;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // capture
    int i{0};
    auto f = [=]() {return i;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // mutable
    int i{0};
    auto f = [=]() mutable {return i++;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // BOOST_NOEXCEPT instead of noexcept because VS2013 does not support noexcept.
    auto f = []() BOOST_NOEXCEPT {return 0;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // parameter by value
    auto f = [](int i) {return i;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // parameter by reference
    auto f = [](int& i) {++i; return i;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // parameter by constant reference
    auto f = [](int const& i) {return i * 2;};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // multiple parameters
    auto f = [](int const& i, float* f) {return i * (*f);};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  } { // multiple parameters mutable
    int n{2};
    auto f = [&](int const& i, float* f) mutable {return i * (n++) * (*f);};
    static_assert(HasMemberOperatorCall<decltype(f)>::value, "");
  }
}

struct no_op_call_t {
};

struct void_void_t {
  void operator()() {}
};

struct int_void_t {
  int operator()() {return 0;}
};

struct int_void_const_t {
  int operator()() const {return 0;}
};

struct int_void_volatile_t {
  int operator()() volatile {return 0;}
};

struct int_void_const_volatile_t {
  int operator()() const volatile {return 0;}
};

struct virtual_int_void_const_t {
  virtual int operator()() const {return 0;}
};

struct int_void_int_ref_a_const_t {
  int operator()(int&, a_t) const {return 0;}
};

struct int_float_mutable_and_const_t {
  int operator()(float) {return 0;}
  int operator()(float) const {return 0;}
};

struct int_const_ptr_float_const_t {
  int const* operator()(float) const {static const int i{0}; return &i;}
};

TEST(TypeTraits, HasMemberOperatorCallClass) {
  using namespace ka;
  static_assert(!HasMemberOperatorCall<no_op_call_t>::value, "");
  static_assert( HasMemberOperatorCall<void_void_t>::value, "");
  static_assert( HasMemberOperatorCall<int_void_t>::value, "");
  static_assert( HasMemberOperatorCall<int_void_const_t>::value, "");
  static_assert( HasMemberOperatorCall<int_void_volatile_t>::value, "");
  static_assert( HasMemberOperatorCall<int_void_const_volatile_t>::value, "");
  static_assert( HasMemberOperatorCall<virtual_int_void_const_t>::value, "");
  static_assert( HasMemberOperatorCall<int_void_int_ref_a_const_t>::value, "");
  // This must fail because the type must have only one operator call.
  static_assert(!HasMemberOperatorCall<int_float_mutable_and_const_t>::value, "");
  static_assert( HasMemberOperatorCall<boost::function<void (void)>>::value, "");
  static_assert( HasMemberOperatorCall<boost::function<void (int)>>::value, "");
  static_assert( HasMemberOperatorCall<boost::function<int const& (int, a_t*, float const&, double&)>>::value, "");
}

TEST(TypeTraits, HasMemberOperatorCallBuiltin) {
  using namespace ka;

  // functions
  static_assert(!HasMemberOperatorCall<void (void)>::value, "");
  static_assert(!HasMemberOperatorCall<void (int&)>::value, "");
  static_assert(!HasMemberOperatorCall<a_t (int&, float, double*)>::value, "");

  // function pointers
  static_assert(!HasMemberOperatorCall<void (*)(int&)>::value, "");
  static_assert(!HasMemberOperatorCall<void (*)(int&)>::value, "");
  static_assert(!HasMemberOperatorCall<a_t (*)(int&, float, double*)>::value, "");

  // member function pointers
  static_assert(!HasMemberOperatorCall<void (a_t::*)()>::value, "");
  static_assert(!HasMemberOperatorCall<void (a_t::*)() const>::value, "");
  static_assert(!HasMemberOperatorCall<void (a_t::*)() volatile>::value, "");
  static_assert(!HasMemberOperatorCall<void (a_t::*)() const volatile>::value, "");
  static_assert(!HasMemberOperatorCall<float (a_t::*)(int)>::value, "");
  static_assert(!HasMemberOperatorCall<float (a_t::*)(int, double&, a_t const&)>::value, "");
}

TEST(TypeTraits, IsFunctionObjectLambda) {
  using namespace ka;
  { // without return nor parameter
    auto f = []() {};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // no capture
    auto f = []() {return 0;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // capture
    int i{0};
    auto f = [=] {return i;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // mutable
    int i{0};
    auto f = [=]() mutable {return i++;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // parameter by value
    auto f = [](int i) {return i;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // parameter by reference
    auto f = [](int& i) {++i; return i;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // parameter by constant reference
    auto f = [](int const& i) {return i * 2;};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // multiple parameters
    auto f = [](int const& i, float* f) {return i * (*f);};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  } { // multiple parameters mutable
    int n{2};
    auto f = [&](int const& i, float* f) mutable {return i * (n++) * (*f);};
    static_assert(IsFunctionObject<decltype(f)>::value, "");
  }
}

TEST(TypeTraits, IsFunctionObjectClass) {
  using namespace ka;
  static_assert(!IsFunctionObject<no_op_call_t>::value, "");
  static_assert( IsFunctionObject<void_void_t>::value, "");
  static_assert( IsFunctionObject<int_void_t>::value, "");
  static_assert( IsFunctionObject<int_void_const_t>::value, "");
  static_assert( IsFunctionObject<int_void_volatile_t>::value, "");
  static_assert( IsFunctionObject<int_void_const_volatile_t>::value, "");
  static_assert( IsFunctionObject<virtual_int_void_const_t>::value, "");
  static_assert( IsFunctionObject<int_void_int_ref_a_const_t>::value, "");
  // This must fail because the type must have only one operator call.
  static_assert(!IsFunctionObject<int_float_mutable_and_const_t>::value, "");
  static_assert( IsFunctionObject<boost::function<void (void)>>::value, "");
  static_assert( IsFunctionObject<boost::function<void (int)>>::value, "");
  static_assert( IsFunctionObject<boost::function<int const& (int, a_t*, float const&, double&)>>::value, "");
}

TEST(TypeTraits, IsFunctionObjectBuiltin) {
  using namespace ka;

  // functions
  static_assert(!IsFunctionObject<void (void)>::value, "");
  static_assert(!IsFunctionObject<void (int&)>::value, "");
  static_assert(!IsFunctionObject<a_t (int&, float, double*)>::value, "");

  // function pointers
  static_assert(!IsFunctionObject<void (*)(int&)>::value, "");
  static_assert(!IsFunctionObject<void (*)(int&)>::value, "");
  static_assert(!IsFunctionObject<a_t (*)(int&, float, double*)>::value, "");

  // member function pointers
  static_assert(!IsFunctionObject<void (a_t::*)()>::value, "");
  static_assert(!IsFunctionObject<void (a_t::*)() const>::value, "");
  static_assert(!IsFunctionObject<void (a_t::*)() volatile>::value, "");
  static_assert(!IsFunctionObject<void (a_t::*)() const volatile>::value, "");
  static_assert(!IsFunctionObject<float (a_t::*)(int)>::value, "");
  static_assert(!IsFunctionObject<float (a_t::*)(int, double&, a_t const&)>::value, "");
}

TEST(TypeTraits, FunctionLambda) {
  using namespace ka;
  {
    auto f = []() {};
    static_assert(Equal<Function<decltype(f)>, void (void)>::value, "");
  } {
    auto f = [](int) {};
    static_assert(Equal<Function<decltype(f)>, void (int)>::value, "");
  } {
    auto f = [](int&) {};
    static_assert(Equal<Function<decltype(f)>, void (int&)>::value, "");
  } {
    auto f = [](int const&) {};
    static_assert(Equal<Function<decltype(f)>, void (int const&)>::value, "");
  } {
    auto f = [](int, float) {};
    static_assert(Equal<Function<decltype(f)>, void (int, float)>::value, "");
  } {
    auto f = [](int*, float&) {};
    static_assert(Equal<Function<decltype(f)>, void (int*, float&)>::value, "");
  } {
    auto f = [](int*, float&) mutable {};
    static_assert(Equal<Function<decltype(f)>, void (int*, float&)>::value, "");
  } {
    auto f = [](int*, float&) mutable {return 0;};
    static_assert(Equal<Function<decltype(f)>, int (int*, float&)>::value, "");
  }
}

TEST(TypeTraits, FunctionClass) {
  using namespace ka;
  using boost::function;
  static_assert( Equal<Function<void_void_t>, void (void)>::value, "");
  static_assert( Equal<Function<int_void_t>, int (void)>::value, "");
  static_assert( Equal<Function<int_void_const_t>, int (void)>::value, "");
  static_assert( Equal<Function<int_void_volatile_t>, int (void)>::value, "");
  static_assert( Equal<Function<int_void_const_volatile_t>, int (void)>::value, "");
  static_assert( Equal<Function<virtual_int_void_const_t>, int (void)>::value, "");
  static_assert( Equal<Function<int_void_int_ref_a_const_t>, int (int&, a_t)>::value, "");
  static_assert( Equal<Function<function<void (void)>>, void (void)>::value, "");
  static_assert( Equal<Function<function<void (int)>>, void (int)>::value, "");
  static_assert( Equal<Function<function<int const& (int, a_t*, float const&, double&)>>,
                    int const& (int, a_t*, float const&, double&)>::value, "");
}

TEST(TypeTraits, FunctionBuiltin) {
  using namespace ka;
  using boost::function;

  // functions
  static_assert( Equal<Function<void (void)>, void (void)>::value, "");
  static_assert( Equal<Function<int (void)>, int (void)>::value, "");
  static_assert( Equal<Function<int (int&, a_t)>, int (int&, a_t)>::value, "");
  static_assert( Equal<Function<float const& (int*, a_t)>, float const& (int*, a_t)>::value, "");

  // function pointers
  static_assert( Equal<Function<void (*)(void)>, void (void)>::value, "");
  static_assert( Equal<Function<int (*)(void)>, int (void)>::value, "");
  static_assert( Equal<Function<int (*)(int&, a_t)>, int (int&, a_t)>::value, "");
  static_assert( Equal<Function<float const& (*)(int*, a_t)>, float const& (int*, a_t)>::value, "");
}

struct my_contiguous_container_t {
};

namespace ka{ namespace detail{
  template<>
  struct IsContiguous<my_contiguous_container_t> {
    using type = true_t;
  };
}}

TEST(TypeTraits, IsContiguous) {
  using namespace ka;

  // non containers
  static_assert(!IsContiguous<void>::value, "");
  static_assert(!IsContiguous<int>::value, "");
  static_assert(!IsContiguous<int*>::value, "");
  static_assert(!IsContiguous<int&>::value, "");
  static_assert(!IsContiguous<a_t>::value, "");

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
  static_assert( IsContiguous<my_contiguous_container_t>::value, "");
}

TEST(TypeTraits, IsContiguousLike) {
  using namespace ka;

  // non containers
  static_assert(!IsContiguousLike<void>::value, "");
  static_assert(!IsContiguousLike<int>::value, "");
  static_assert(!IsContiguousLike<int*>::value, "");
  static_assert(!IsContiguousLike<int&>::value, "");
  static_assert(!IsContiguousLike<a_t>::value, "");

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
  static_assert( IsContiguousLike<my_contiguous_container_t>::value, "");
}

struct my_list_t {
};

namespace ka{ namespace detail{
  template<>
  struct IsList<my_list_t> {
    using type = true_t;
  };
}}

TEST(TypeTraits, IsList) {
  using namespace ka;

  // Non "containers"
  static_assert(!IsList<void>::value, "");
  static_assert(!IsList<int>::value, "");
  static_assert(!IsList<int*>::value, "");
  static_assert(!IsList<int&>::value, "");
  static_assert(!IsList<a_t>::value, "");

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
  static_assert( IsList<my_list_t>::value, "");
}

namespace test_typetraits {
  enum class ctor_t {
    default_, // `default` is not a valid name, as it is a C++ keyword.
    copy,
    custom
  };

  std::ostream& operator<<(std::ostream& o, ctor_t x) {
    switch (x) {
    case ctor_t::default_: return o << "Ctor::Default";
    case ctor_t::copy: return o << "Ctor::Copy";
    case ctor_t::custom: return o << "Ctor::Custom";
    }
    return o << "...";
  }

  struct base_t {
    ctor_t ctor;
    base_t() : ctor(ctor_t::default_) {
    }
    base_t(const base_t&) : ctor(ctor_t::copy) {
    }
    template<typename T>
    base_t(T&&, ka::EnableIfNotBaseOf<base_t, T>* = {}) : ctor(ctor_t::custom) {
    }
  };

  struct derived_0_t : base_t {
  };

  struct derived_1_t : derived_0_t {
  };
} // namespace test_typetraits

TEST(TypeTraits, EnableIfNotBaseOf) {
  using namespace ka;
  using namespace test_typetraits;
  {
    int x;
    ASSERT_EQ(ctor_t::custom, base_t(x).ctor);
  } {
    base_t b;
    base_t x(b);
    ASSERT_EQ(ctor_t::copy, x.ctor);
  } {
    base_t x(derived_0_t{});
    ASSERT_EQ(ctor_t::copy, x.ctor);
  } {
    base_t x(derived_1_t{});
    ASSERT_EQ(ctor_t::copy, x.ctor);
  } {
    base_t b;
    base_t x(std::move(b));
    ASSERT_EQ(ctor_t::copy, x.ctor);
  }
}

TEST(TypeTraits, Conditional) {
  using namespace ka;
  static_assert(Equal<Conditional<true, int, float>, int>::value, "");
  static_assert(Equal<Conditional<false, int, float>, float>::value, "");
}

struct custom_true_t  { static const bool value = true; };
struct custom_false_t { static const bool value = false; };

template<typename IntCst, typename Base>
void checkIntConstant() {
  static_assert(IntCst::value == Base::value, "");
  static_assert(std::is_base_of<Base, IntCst>::value, "");
}

TEST(TypeTraits, Negation) {
  using namespace ka;
  checkIntConstant<Negation<true_t>, false_t>();
  checkIntConstant<Negation<false_t>, true_t>();
  checkIntConstant<Negation<custom_true_t>, false_t>();
  checkIntConstant<Negation<custom_false_t>, true_t>();
}

TEST(TypeTraits, Conjunction) {
  using namespace ka;
  checkIntConstant<Conjunction<>, true_t>();
  checkIntConstant<Conjunction<true_t>, true_t>();
  checkIntConstant<Conjunction<false_t>, false_t>();
  checkIntConstant<Conjunction<true_t, false_t>, false_t>();
  checkIntConstant<Conjunction<false_t, true_t>, false_t>();
  checkIntConstant<Conjunction<true_t, true_t>, true_t>();
  checkIntConstant<Conjunction<custom_true_t>, custom_true_t>();
  checkIntConstant<Conjunction<custom_false_t>, custom_false_t>();
  checkIntConstant<Conjunction<custom_true_t, custom_false_t>, custom_false_t>();
  checkIntConstant<Conjunction<custom_false_t, custom_true_t>, custom_false_t>();
  checkIntConstant<Conjunction<custom_false_t, false_t>, custom_false_t>();
  checkIntConstant<Conjunction<custom_true_t, custom_true_t>, custom_true_t>();
  checkIntConstant<Conjunction<true_t, custom_true_t>, custom_true_t>();
}

TEST(TypeTraits, Disjunction) {
  using namespace ka;
  checkIntConstant<Disjunction<>, false_t>();
  checkIntConstant<Disjunction<true_t>, true_t>();
  checkIntConstant<Disjunction<false_t>, false_t>();
  checkIntConstant<Disjunction<true_t, false_t>, true_t>();
  checkIntConstant<Disjunction<false_t, true_t>, true_t>();
  checkIntConstant<Disjunction<true_t, true_t>, true_t>();
  checkIntConstant<Disjunction<custom_true_t>, custom_true_t>();
  checkIntConstant<Disjunction<custom_false_t>, custom_false_t>();
  checkIntConstant<Disjunction<custom_true_t, custom_false_t>, custom_true_t>();
  checkIntConstant<Disjunction<custom_false_t, custom_true_t>, custom_true_t>();
  checkIntConstant<Disjunction<false_t, custom_false_t>, custom_false_t>();
  checkIntConstant<Disjunction<custom_true_t, custom_true_t>, custom_true_t>();
  checkIntConstant<Disjunction<custom_true_t, true_t>, custom_true_t>();
}

namespace {
  struct without_op_star_t {
    int i;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(without_op_star_t, i)
  };

  struct with_op_star_t {
    int i;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(with_op_star_t, i)
    int operator*() const {
      return i;
    }
  };
}

TEST(TypeTraits, HasOperatorStar) {
  using namespace ka;
  using ka::HasOperatorStar;
  static_assert( HasOperatorStar<int*>::value, "");
  static_assert( HasOperatorStar<std::vector<int>::iterator>::value, "");
  static_assert( HasOperatorStar<std::list<int>::const_iterator>::value, "");
  static_assert( HasOperatorStar<int**>::value, "");
  static_assert( HasOperatorStar<const int*>::value, "");
  static_assert( HasOperatorStar<int* const>::value, "");
  static_assert( HasOperatorStar<const int* const>::value, "");
  static_assert( HasOperatorStar<int [2]>::value, "");
  static_assert( HasOperatorStar<int (&)[2]>::value, "");
  static_assert( HasOperatorStar<bool (int)>::value, "");
  static_assert( HasOperatorStar<bool (&)(int)>::value, "");
  static_assert( HasOperatorStar<bool (*)(int)>::value, "");
  static_assert( HasOperatorStar<with_op_star_t>::value, "");
  static_assert( HasOperatorStar<mutable_store_t<int, int*>>::value, "");

  static_assert(!HasOperatorStar<int>::value, "");
  static_assert(!HasOperatorStar<int&>::value, "");
  static_assert(!HasOperatorStar<without_op_star_t>::value, "");
}

namespace {
  bool f(bool x) {
    return !x;
  }

  KA_GENERATE_TRAITS_HAS(HasTransfoF, T, ka::declref<T>() = f(ka::declcref<T>()))

  struct without_f_t {
  };

  struct with_f {
    friend with_f f(with_f x) {
      return x;
    }
  };

  struct with_external_f_t {
  };

  with_external_f_t f(with_external_f_t x) {
    return x;
  }
}

TEST(TypeTraits, HasTransfoF) {
  static_assert( HasTransfoF<bool>::value, "");
  static_assert( HasTransfoF<int>::value, ""); // because `int` is convertible to `bool`...
  static_assert( HasTransfoF<float>::value, ""); // idem `float`...
  static_assert( HasTransfoF<with_f>::value, "");
  static_assert( HasTransfoF<with_external_f_t>::value, "");
  static_assert(!HasTransfoF<without_f_t>::value, "");
  static_assert(!HasTransfoF<std::string>::value, "");
}

TEST(TypeTraits, IsRetract) {
  using namespace ka;
  using namespace test_functional;
  static_assert( IsRetract<one_, true_>::value, "");
  static_assert(!IsRetract<true_, one_>::value, "");

  static_assert( IsRetract<f_t, f_inv_t>::value, "");
  static_assert( IsRetract<f_inv_t, f_t>::value, "");

  static_assert( IsRetract<g_t, g_inv_t>::value, "");
  static_assert( IsRetract<g_inv_t, g_t>::value, "");

  static_assert(!IsRetract<f_t, g_inv_t>::value, "");
  static_assert(!IsRetract<g_inv_t, f_t>::value, "");
}

namespace my {

struct iterator_t {};
struct non_iterator_t {};

template<typename T, typename = ka::EnableIfInputIterator<T>>
iterator_t f(T);

template<typename T, typename = ka::EnableIfNotInputIterator<T>>
non_iterator_t f(T);

} // namespace my

TEST(TypeTraits, EnableIfInputIterator) {
  using ka::Equal;
  using my::iterator_t;
  using my::non_iterator_t;

  { // InputIterator
    std::istream_iterator<int> x;
    static_assert(Equal<iterator_t, decltype(my::f(x))>::value, "");
  }
  { // ForwardIterator
    std::forward_list<int> x;
    static_assert(Equal<iterator_t, decltype(my::f(x.begin()))>::value, "");
  }
  { // BidirectionalIterator
    std::list<int> x;
    static_assert(Equal<iterator_t, decltype(my::f(x.begin()))>::value, "");
  }
  { // RandomAccessIterator (0)
    std::vector<int> x;
    static_assert(Equal<iterator_t, decltype(my::f(x.begin()))>::value, "");
  }
  { // RandomAccessIterator (1)
    int* x;
    static_assert(Equal<iterator_t, decltype(my::f(x))>::value, "");
  }
  { // RandomAccessIterator (2)
    int* x;
    static_assert(Equal<iterator_t, decltype(my::f(x))>::value, "");
  }
  { // Non-iterator (0)
    std::string x;
    static_assert(Equal<non_iterator_t, decltype(my::f(x))>::value, "");
  }
  { // Non-iterator (1)
    std::vector<int> x;
    static_assert(Equal<non_iterator_t, decltype(my::f(x))>::value, "");
  }
  { // Non-iterator (2)
    float x;
    static_assert(Equal<non_iterator_t, decltype(my::f(x))>::value, "");
  }
}

TEST(TypeTraits, ConstantVoid) {
  using namespace ka;
  using testing::StaticAssertTypeEq;
  StaticAssertTypeEq<void, ConstantVoid<>>();
  StaticAssertTypeEq<void, ConstantVoid<short>>();
  StaticAssertTypeEq<void, ConstantVoid<int, char>>();
  StaticAssertTypeEq<void, ConstantVoid<std::string, double, float>>();
}

namespace {
  using namespace ka;
  template<typename T, typename = void>
  struct IsIntAndFloatAssignable : false_t {};

  template <typename T>
  struct IsIntAndFloatAssignable<
    T, ConstantVoid<decltype(declref<T>() = 1), decltype(declref<T>() = 1.1f)>>
    : true_t {};

  struct int_float_assignable_t {
    int value;
    void operator=(int x) {value = x;}
    void operator=(float x) {value = x;}
  };

  struct string_assignable_t {
    std::string value;
    void operator=(std::string x) {value = std::move(x);}
  };

  template<typename T>
  void try_set_impl(T& t, int i, true_t /* is int/float-assignable */) {
    t = i;
  }

  template<typename T>
  void try_set_impl(T& t, int i, false_t /* is NOT int/float-assignable */) {
    // nothing.
  }

  template<typename T>
  void try_set(T& t, int i) {
    try_set_impl(t, i, typename IsIntAndFloatAssignable<T>::type{});
  }
} // namespace

TEST(TypeTraits, ConstantVoidInTypePredicate) {
  using namespace ka;
  static_assert(!IsIntAndFloatAssignable<std::vector<int>>::value, "");
  static_assert( IsIntAndFloatAssignable<int_float_assignable_t>::value, "");
}

TEST(TypeTraits, ConstantVoidInSfinaeAssignable) {
  using namespace ka;
  auto n = int_float_assignable_t{0};
  EXPECT_EQ(0, n.value);
  try_set(n, 5);
  EXPECT_EQ(5, n.value);
}

TEST(TypeTraits, ConstantVoidInSfinaeNotAssignable) {
  auto n = string_assignable_t{"abc"};
  EXPECT_EQ("abc", n.value);
  try_set(n, 5);
  EXPECT_EQ("abc", n.value);
}

TEST(Rebind, Basic) {
  using namespace ka;
  using namespace std;
  static_assert(Equal<array<bool, 3>, Rebind<array<int, 3>, bool>>::value, "");
  static_assert(Equal<vector<bool>, Rebind<vector<int>, bool>>::value, "");
  static_assert(Equal<deque<bool>, Rebind<deque<int>, bool>>::value, "");
  static_assert(Equal<forward_list<bool>, Rebind<forward_list<int>, bool>>::value, "");
  static_assert(Equal<list<bool>, Rebind<list<int>, bool>>::value, "");
  static_assert(Equal<set<bool>, Rebind<set<int>, bool>>::value, "");
  static_assert(Equal<multiset<bool>, Rebind<multiset<int>, bool>>::value, "");
  static_assert(Equal<map<char, bool>, Rebind<map<char, int>, bool>>::value, "");
  static_assert(Equal<multimap<char, bool>, Rebind<multimap<char, int>, bool>>::value, "");
  static_assert(Equal<unordered_set<bool>,
    Rebind<unordered_set<int>, bool>>::value, "");
  static_assert(Equal<unordered_multiset<bool>,
    Rebind<unordered_multiset<int>, bool>>::value, "");
  static_assert(Equal<unordered_map<char, bool>, Rebind<unordered_map<char, int>, bool>>::value, "");
  static_assert(Equal<unordered_multimap<char, bool>, Rebind<unordered_multimap<char, int>, bool>>::value, "");
}

namespace {
  struct NoMemberFmap {
  };
  struct MemberFmap {
    void fmap(int) const {
    }
  };
  struct NonConstMemberFmap {
    void fmap(int) {
    }
  };
  struct NullaryMemberFmap {
    void fmap() const {
    }
  };
  struct BinaryMemberFmap {
    void fmap(int, bool) const {
    }
  };
  struct PolymorphicMemberFmap {
    template<typename F>
    void fmap(F) const {
    }
  };
}

TEST(HasMemberFmap, Basic) {
  using namespace ka;
  static_assert(!HasMemberFmap<NoMemberFmap>::value, "");
  static_assert( HasMemberFmap<MemberFmap>::value, "");
  static_assert(!HasMemberFmap<NonConstMemberFmap>::value, "");
  static_assert(!HasMemberFmap<NullaryMemberFmap>::value, "");
  static_assert(!HasMemberFmap<BinaryMemberFmap>::value, "");
  static_assert( HasMemberFmap<PolymorphicMemberFmap>::value, "");
}
