#include <gtest/gtest.h>
#include <boost/thread.hpp>

#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
#include <qi/trackable.hpp>
KA_WARNING_POP()

#include <qi/future.hpp>
#include <qi/actor.hpp>
#include "test_future.hpp"

qiLogCategory("test");

int exchange(int& target, int value)
{
  int old = target;
  target = value;
  return old;
}

// wrap a call, waiting before, and notifying state
void wrap(boost::function<void()> op, qi::MilliSeconds delay, std::atomic<int>& notify)
{
  notify = 1;
  if (delay != qi::MilliSeconds::zero())
    boost::this_thread::sleep_for(delay);
  notify = 2;
  try {
    op();
  }
  catch (const qi::PointerLockException&)
  {}
  notify = 3;
}

TEST(TestBind, Simple)
{
  int v = 0;
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads
  qi::bind<void(int)>(&exchange, std::ref(v), _1)(15);
  EXPECT_EQ(15, v);
  qi::bind<void(void)>(&exchange, std::ref(v), 16)();
KA_WARNING_POP()
  EXPECT_EQ(16, v);
  qi::bind(&exchange, std::ref(v), _1)(15);
  EXPECT_EQ(15, v);
  qi::bind(&exchange, std::ref(v), 16)();
  EXPECT_EQ(16, v);
}

TEST(TestBind, MemFun)
{
  std::atomic<int> v{0};
  SetValue s1(v);
  qi::bind(&SetValue::exchange, &s1, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind(&SetValue::exchange, &s1, 2)();
  EXPECT_EQ(2, v);
  qi::bind(&SetValue::exchange, boost::ref(s1), _1)(3);
  EXPECT_EQ(3, v);
  qi::bind(&SetValue::exchange, boost::ref(s1), 4)();
  EXPECT_EQ(4, v);
}

TEST(TestBind, SharedPtr)
{
  std::atomic<int> v{0};
  boost::shared_ptr<SetValue> s(new SetValue(v));
  qi::bind(&SetValue::exchange, s, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind(&SetValue::exchange, s, 2)();
  EXPECT_EQ(2, v);

  boost::function<void(void)> f =  qi::bind(&SetValue::exchange, s, 3);
  s.reset();
  f();
  EXPECT_EQ(3, v);
}

TEST(TestBind, WeakPtr)
{
  std::atomic<int> v{0};
  boost::shared_ptr<SetValue> s(new SetValue(v));
  boost::weak_ptr<SetValue> w(s);
  qi::bind(&SetValue::exchange, w, _1)(1);
  EXPECT_EQ(1, v);
  qi::bind(&SetValue::exchange, w, 2)();
  EXPECT_EQ(2, v);

  boost::function<void(void)> f =  qi::bind(&SetValue::exchange, w, 3);
  s.reset();
  EXPECT_ANY_THROW(f());
  EXPECT_EQ(2, v);
}

TEST(TestBind, Trackable)
{
  std::atomic<int> v{0};
  {
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads
    SetValue2 s1(v);
    qi::bind<void(int)>(&SetValue2::exchange, &s1, _1)(1);
    EXPECT_EQ(1, v);
    qi::bind<void(void)>(&SetValue2::exchange, &s1, 2)();
    EXPECT_EQ(2, v);
    qi::bind<void(int)>(&SetValue2::exchange, boost::ref(s1), _1)(3);
    EXPECT_EQ(3, v);
    qi::bind<void(void)>(&SetValue2::exchange, boost::ref(s1), 4)();
    EXPECT_EQ(4, v);
KA_WARNING_POP()
  }
  v = 0;
  {
    SetValue2 s1(v);
    qi::bind(&SetValue2::exchange, &s1, _1)(1);
    EXPECT_EQ(1, v);
    qi::bind(&SetValue2::exchange, &s1, 2)();
    EXPECT_EQ(2, v);
    qi::bind(&SetValue2::exchange, boost::ref(s1), _1)(3);
    EXPECT_EQ(3, v);
    qi::bind(&SetValue2::exchange, boost::ref(s1), 4)();
    EXPECT_EQ(4, v);
  }

  boost::function<void(void)> f;
  {
    SetValue2 s1(v);
    f = qi::bind(&SetValue2::exchange, &s1, 5);
  }
  EXPECT_ANY_THROW(f()); // s1 is trackable, bound and deleted: callback not executed
  EXPECT_EQ(4, v);

  // check waiting behavior of destroy
  std::atomic<int> notify{0};
  {
    SetValue2 s1(v);
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads
    boost::thread(wrap,
      qi::bind<void(void)>(&SetValue2::delayExchange, &s1, qi::MilliSeconds{100}, 10),
      qi::MilliSeconds{ 0 },
      std::ref(notify));
KA_WARNING_POP()
    // wait enough for operation to start
    while (!notify)
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    // exit scope, deleting s1, which should block until operation terminates
  }
  EXPECT_EQ(10, v);

  // check disable-call behavior again in our more complex threaded setup
  {
    notify = 0;
    SetValue2 s1(v);
    boost::thread(wrap,
      qi::bind(&SetValue2::delayExchange, &s1, qi::MilliSeconds{100}, 11),
      qi::MilliSeconds{ 50 },
      std::ref(notify));
  }
  while (notify != 3)
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  EXPECT_EQ(10, v); //call not made
}

TEST(TestBind, BindLambda)
{
  auto f = qi::bind<int>([](int i){ return i; }, 18);
  ASSERT_EQ(18, f());
}

namespace {

  using SomeData = std::vector<int>;

  struct ActorType : public qi::Actor
  {
    ~ActorType()
    {
      qi::Actor::joinTasks();
    }

    // must return a future for the test to reproduce real common use case
    qi::Future<int> run(const SomeData&) { return qi::Future<int>{42}; }
  };

  struct NotActorType
  {
    // must return a future for the test to reproduce real common use case
    qi::Future<int> run(const SomeData&) { return qi::Future<int>{42}; }
  };

  template<typename T>
  struct TestBindActor : ::testing::Test {};

  using ObjectTypeList = ::testing::Types<ActorType, NotActorType>;
  TYPED_TEST_SUITE(TestBindActor, ObjectTypeList);

}

TYPED_TEST(TestBindActor, BindObjectWrappedFunc)
{
  auto p = std::make_shared<TypeParam>();
  const SomeData data;
  static auto f = [](TypeParam* self, const SomeData& data) { return self->run(data); };
  using ReturnType = decltype(f(p.get(), data));
  static std::function<ReturnType(TypeParam*, const SomeData&)> sf{ f };

  auto fu = qi::bind(sf, p.get(), data);
  auto result = qi::detail::tryUnwrap(qi::async(fu));
  static_assert(std::is_same<decltype(result), ::qi::Future<int>>::value, "");
}



