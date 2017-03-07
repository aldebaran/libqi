#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <qi/type/proxysignal.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>

const std::chrono::milliseconds usualTimeout{100};

struct ObjectWithVariousSignals
{
  qi::Signal<void> voidSignal;
};

QI_REGISTER_OBJECT(ObjectWithVariousSignals, voidSignal)

TEST(TestProxySignal, TransmitVoid)
{
  auto object = boost::make_shared<ObjectWithVariousSignals>();
  qi::AnyObject anyObject{object};
  qi::ProxySignal<void()> proxy{anyObject, "voidSignal"};
  std::promise<void> signalReceived;
  proxy.connect([&signalReceived]() mutable { signalReceived.set_value(); });
  QI_EMIT object->voidSignal();
  ASSERT_EQ(std::future_status::ready, signalReceived.get_future().wait_for(usualTimeout));
}
