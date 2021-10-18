#include <array>
#include <string>
#include <gtest/gtest.h>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <ka/functional.hpp>
#include <ka/errorhandling.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/scoped.hpp>
#include <ka/testutils.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>

struct boost_error_t : boost::exception {
  std::string msg;
  boost_error_t(std::string const& s = std::string{}) : msg(s) {
  }
  std::ostream& operator<<(std::ostream& o) const {
    return o << "boost_error_t{\"" << msg << "\"}";
  }
};

namespace {
  auto const std_code = 55;
  auto const boost_code = 56;
  auto const unknown_code = 57;
  ka::exception_value_t<int> const exception_code{std_code, boost_code, unknown_code};

  template<typename T>
  int code() {
    return unknown_code;
  }

  template<>
  int code<std::runtime_error>() {
    return std_code;
  }

  template<>
  int code<boost_error_t>() {
    return boost_code;
  }

  static const std::string std_message = "youpi";
  static const std::string boost_message = "youpa";
  static const std::string unknown_message = ka::exception_message_t::unknown_error();
}

TEST(TestErrorHandling, ExceptionValueBasic) {
  using namespace ka;
  ASSERT_EQ(std_code, exception_code(std::runtime_error{""}));
  ASSERT_EQ(boost_code, exception_code(boost_error_t{""}));
  ASSERT_EQ(unknown_code, exception_code());
}

TEST(TestErrorHandling, ExceptionValueRegular) {
  using namespace ka;
  using F = exception_value_t<int>;
  // This test is partial but representative.
  ASSERT_TRUE(is_regular(
    {F{0, 0, 0}, F{0, 0, 1}, F{0, 1, 0}, F{1, 0, 0}, F{0, 1, 1}, F{1, 1, 1}}));
}

namespace test_error_handling {
  // Constructs a `ka::handle_exception_rethrow_t`.
  struct handle_exception_rethrow_t {
    template<typename T>
    auto operator()(T&& t) const -> ka::handle_exception_rethrow_t<ka::Decay<T>> {
      return {ka::fwd<T>(t)};
    }
  };

  // Calls `ka::handle_exception_rethrow`.
  struct handle_exception_rethrow_fn_t {
    template<typename... T>
    auto operator()(T&&... t) const -> decltype(ka::handle_exception_rethrow(ka::fwd<T>(t)...)) {
      return ka::handle_exception_rethrow(ka::fwd<T>(t)...);
    }
  };
} // namespace test_error_handling

template<typename T>
struct TestErrorHandlingHandleExceptionRethrow : testing::Test {
};

using handle_exception_rethrow_types = testing::Types<
  test_error_handling::handle_exception_rethrow_t,
  test_error_handling::handle_exception_rethrow_fn_t
>;

TYPED_TEST_SUITE(TestErrorHandlingHandleExceptionRethrow, handle_exception_rethrow_types);

TYPED_TEST(TestErrorHandlingHandleExceptionRethrow, ExceptionHandleExceptionRegular) {
  using namespace ka;
  auto handle_exception_rethrow = TypeParam{};
  auto h = handle_exception_rethrow(constant_function_t<void>{});
  ASSERT_TRUE(is_regular({h}));
}

template<typename T>
struct TestErrorHandlingExceptionParam : testing::Test {};

using exceptions = testing::Types<std::runtime_error, boost_error_t, std::string>;

TYPED_TEST_SUITE(TestErrorHandlingExceptionParam, exceptions);

TYPED_TEST(TestErrorHandlingExceptionParam, ExceptionHandleExceptionRethrow) {
  using Exception = TypeParam;
  using namespace ka;
  auto rethrow = handle_exception_rethrow(constant_function_t<void>{});
  auto f = [=] {
    try {
      throw Exception{""};
    } catch (Exception const& e) {
      rethrow(e);
    }
  };
  EXPECT_THROW(f(), Exception);
}

// It is possible to set the codomain (i.e. return type) of `handle_exception_rethrow`.
TYPED_TEST(TestErrorHandlingExceptionParam, ExceptionHandleExceptionRethrowCodomain) {
  using Exception = TypeParam;
  using namespace ka;
  using ka::test::A;
  using ka::test::B;
  { // Default case: `void`.
    auto rethrow = handle_exception_rethrow(constant_function());
    using Proc = decltype(rethrow);
    static_assert(Equal<void, CodomainFor<Proc, Exception>>::value, "");
  }
  { // Custom case: `B`
    auto rethrow = handle_exception_rethrow(constant_function(), type_t<B>{});
    using Proc = decltype(rethrow);
    static_assert(Equal<B, CodomainFor<Proc, Exception>>::value, "");
  }
}

struct append_t {
  std::string& log;
// PolymorphicFunction<T (std::exception), T (boost::exception), T ()>:
  void operator()(std::exception const& e) const {
    log += e.what();
  }
  void operator()(boost::exception const& e) const {
    log += boost::diagnostic_information(e);
  }
  void operator()() const {
    log += "unknown";
  }
};

TYPED_TEST(TestErrorHandlingHandleExceptionRethrow, HandleExceptionRethrowLogsStdException) {
  using namespace ka;
  auto handle_exception_rethrow = TypeParam{};
  std::string log;
  auto rethrow = handle_exception_rethrow(append_t{log});
  std::string const msg = "abcdefghijkl";
  auto f = [=] {
    try {
      throw std::runtime_error{msg};
    } catch (std::exception const& e) {
      rethrow(e);
    }
  };
  ASSERT_THROW(f(), std::runtime_error);
  ASSERT_EQ(msg, log);
}

namespace test_error_handling {
  // Directly calls `ka::invoke_catch`.
  struct invoke_catch_t {
    template<typename... T>
    auto operator()(T&&... t) const -> decltype(ka::invoke_catch(ka::fwd<T>(t)...)) {
      return ka::invoke_catch(ka::fwd<T>(t)...);
    }
  };

  // Constructs a `ka::invoke_catch_fn_t` and calls it.
  struct invoke_catch_fn_t {
    template<typename Proc, typename F, typename... T>
    auto operator()(Proc&& proc, F&& f, T&&... t) const
        -> decltype(ka::invoke_catch_fn(ka::fwd<Proc>(proc), ka::fwd<F>(f))(ka::fwd<T>(t)...)) {
      using ka::fwd;
      return ka::invoke_catch_fn(fwd<Proc>(proc), fwd<F>(f))(fwd<T>(t)...);
    }
  };
} // namespace test_error_handling

template<typename T>
struct TestErrorHandlingInvokeCatch : testing::Test {
};

using invoke_catch_types = testing::Types<
  test_error_handling::invoke_catch_t,
  test_error_handling::invoke_catch_fn_t
>;

TYPED_TEST_SUITE(TestErrorHandlingInvokeCatch, invoke_catch_types);

TYPED_TEST(TestErrorHandlingInvokeCatch, NoException) {
  using namespace ka;
  auto invoke_catch = TypeParam{};
  auto const twice = [](int i) {
    return 2 * i;
  };
  auto const n = invoke_catch(exception_code, twice, 3);
  ASSERT_EQ(6, n);
}

TYPED_TEST(TestErrorHandlingExceptionParam, InvokeCatchCodeValue) {
  using Exception = TypeParam;
  using namespace ka;
  auto const twice = [](int) -> int {
    throw Exception{""};
  };
  auto const n = invoke_catch(exception_code, twice, 3);
  ASSERT_EQ(code<Exception>(), n);
}

TYPED_TEST(TestErrorHandlingExceptionParam, InvokeCatchHandleExceptionAndRethrow) {
  using Exception = TypeParam;
  using namespace ka;
  auto const f = [](int) {
    throw Exception{""};
  };
  handle_exception_rethrow_t<constant_function_t<void>> rethrow;
  EXPECT_THROW(invoke_catch(rethrow, f, 3), Exception);
}

TYPED_TEST(TestErrorHandlingInvokeCatch, InvokeCatchHandleExceptionAndRethrowLogStdException) {
  using namespace ka;
  auto invoke_catch = TypeParam{};
  std::string log;
  std::string const msg = "abcdefghijkl";
  auto const f = [&](int) {
    throw std::runtime_error{msg};
  };
  handle_exception_rethrow_t<append_t> rethrow{append_t{log}};
  ASSERT_THROW(invoke_catch(rethrow, f, 3), std::runtime_error);
  ASSERT_EQ(msg, log);
}

TYPED_TEST(TestErrorHandlingInvokeCatch, InvokeCatchExceptionMessage) {
  using namespace ka;
  auto invoke_catch = TypeParam{};
  exception_message_t f;
  boost_error_t const boost_error{boost_message};
  ASSERT_EQ(std_message, invoke_catch(f, []() -> std::string {throw std::runtime_error{std_message};}));
  ASSERT_EQ(boost::diagnostic_information(boost_error), invoke_catch(f, [=]() -> std::string {throw boost_error;}));
  ASSERT_EQ(unknown_message, invoke_catch(f, []() -> std::string {throw 5;}));
}

TEST(TestErrorHandling, ExceptionMessageBasic) {
  using namespace ka;
  exception_message_t f;
  boost_error_t const boost_error{boost_message};
  ASSERT_EQ(std_message, f(std::runtime_error{std_message}));
  {
    ASSERT_EQ(boost::diagnostic_information(boost_error), f(boost_error));
  }
  ASSERT_EQ(unknown_message, f());
}

TEST(TestErrorHandling, ExceptionMessageRegular) {
  ASSERT_TRUE(ka::is_regular({ka::exception_message_t{}}));
}
