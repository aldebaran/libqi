/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>

#include <ka/macro.hpp>
#include <boost/algorithm/string.hpp>

#include <gtest/gtest.h>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
#include <qi/trackable.hpp>
KA_WARNING_POP()

#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/detail/traceanalyzer.hpp>

qiLogCategory("test");

class ObjectEvent : public ::testing::Test
{
public:
  virtual void SetUp()
  {
    lastPayload = 0;
  }

  void onFire(int pl);

  int lastPayload;
  qi::Promise<int> pPayload;
};

void ObjectEvent::onFire(int pl)
{
  lastPayload = pl;
  pPayload.setValue(pl);
}

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.

/// A wrapper around the deprecated `qi::bind` functions so that we don't have to repeat the
/// warning disabling macro calls everytime it's used.
// TODO: Remove this once the deprecated `qi::bind` functions are removed.
template <typename R, typename AF, typename Arg0, typename... Args>
auto deprecatedBind(AF&& fun, Arg0&& arg0, Args&&... args)
  -> decltype(qi::bind<R>(ka::fwd<AF>(fun), ka::fwd<Arg0>(arg0), ka::fwd<Args>(args)...))
{
  return qi::bind<R>(ka::fwd<AF>(fun), ka::fwd<Arg0>(arg0), ka::fwd<Args>(args)...);
}

KA_WARNING_POP()

TEST_F(ObjectEvent, Simple)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal<int>("fire");
  qi::AnyObject obj(ob.object());
  EXPECT_LE(1U, obj.metaObject().signalMap().size());
  qi::SignalLink linkId = obj.connect("fire", deprecatedBind<void(int)>(&ObjectEvent::onFire, this, _1)).value();
  ASSERT_TRUE(qi::isValidSignalLink(linkId));
  obj.post("fire", 42);
  EXPECT_TRUE(pPayload.future().wait() != qi::FutureState_Running);
  EXPECT_EQ(42, lastPayload);
  pPayload = qi::Promise<int>();
  obj.post("fire", 51);
  EXPECT_TRUE(pPayload.future().wait() != qi::FutureState_Running);
  EXPECT_EQ(51, lastPayload);
  pPayload = qi::Promise<int>();
  obj.disconnect(linkId);
  obj.post("fire", 42);
  EXPECT_FALSE(pPayload.future().wait(0) != qi::FutureState_Running);
  EXPECT_EQ(51, lastPayload);
}

void readString(const std::string&)
{
}

TEST_F(ObjectEvent, ConnectBind)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseSignal<int>("fire");
  ob.advertiseSignal<int, int>("fire2");
  qi::AnyObject obj(ob.object());
  qi::SignalLink link = obj.connect("fire", deprecatedBind<void(int)>(&ObjectEvent::onFire, this, _1)).value();
  ASSERT_TRUE(qi::isValidSignalLink(link));
  obj.post("fire", 42);
  EXPECT_TRUE(pPayload.future().wait() != qi::FutureState_Running);
  EXPECT_EQ(42, lastPayload);
  obj.disconnect(link);
  // The boost bind without _1 gives us a void (void) signature that does not match fire
  EXPECT_ANY_THROW(
    obj.connect("fire", boost::bind<void>(&ObjectEvent::onFire, this, 51)).value()
  );
  // Argument type mismatch
  EXPECT_ANY_THROW(
    obj.connect("fire", boost::function<void(const std::string&)>(boost::bind<void>(&readString, _1))).value()
  );
  link = obj.connect("fire2", deprecatedBind<void(int, int)>(&ObjectEvent::onFire, this, _2)).value();
  EXPECT_TRUE(link != 0);
  pPayload = qi::Promise<int>();
  obj.post("fire2", 40, 41);
  EXPECT_TRUE(pPayload.future().wait() != qi::FutureState_Running);
  EXPECT_EQ(41, lastPayload);
  obj.disconnect(link);
}

TEST_F(ObjectEvent, EmitMethod)
{
  lastPayload = 0;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("fire", deprecatedBind<void(int)>(&ObjectEvent::onFire, this, _1));
  qi::AnyObject obj(ob.object());
  pPayload = qi::Promise<int>();
  obj.post("fire", 23);
  EXPECT_TRUE(pPayload.future().wait() != qi::FutureState_Running);
  EXPECT_EQ(23, pPayload.future().value());
}

using qi::EventTrace;

inline qi::os::timeval ts(qi::int64_t v)
{
  qi::os::timeval res;
  res.tv_sec = v / 1000000;
  res.tv_usec = v % 1000000;
  return res;
}

std::string sort(const std::string& input)
{
  std::vector<std::string> s;
  boost::algorithm::split(s, input, boost::algorithm::is_any_of("\n"));
  if (s.back().empty())
    s.pop_back();
  std::sort(s.begin(), s.end());
  return boost::algorithm::join(s, "\n") + "\n";
}

TEST(TestTraceAnalyzer, Basic)
{
  qi::TraceAnalyzer ta;
  qi::AnyValue noargs;
  EXPECT_EQ("", ta.dumpTraces());
  ta.addTrace(qi::EventTrace(10, EventTrace::Event_Call,   100, noargs, ts(10), 0, 0, 50, 50), 1);
  ta.addTrace(qi::EventTrace(10, EventTrace::Event_Result, 100, noargs, ts(11), 0, 0, 50, 50), 1);
  EXPECT_EQ("50 10:1.100\n", ta.dumpTraces());
  ta.clear();
  ta.addTrace(qi::EventTrace(10, EventTrace::Event_Result, 100, noargs, ts(11), 0, 0, 50, 50), 1);
  ta.addTrace(qi::EventTrace(10, EventTrace::Event_Call,   100, noargs, ts(10), 0, 0, 50, 50), 1);
  EXPECT_EQ("50 10:1.100\n", ta.dumpTraces());
  ta.addTrace(qi::EventTrace(11, EventTrace::Event_Result, 100, noargs, ts(13), 0, 0, 50, 50), 1);
  ta.addTrace(qi::EventTrace(11, EventTrace::Event_Call,   100, noargs, ts(12), 0, 0, 50, 50), 1);
  EXPECT_EQ("50 10:1.100 11:1.100\n", ta.dumpTraces());
  ta.addTrace(qi::EventTrace(12, EventTrace::Event_Result, 100, noargs, ts(11), 0, 0, 51, 51), 1);
  ta.addTrace(qi::EventTrace(12, EventTrace::Event_Call,   100, noargs, ts(10), 0, 0, 51, 51), 1);
  EXPECT_EQ("51 12:1.100\n50 10:1.100 11:1.100\n", ta.dumpTraces());
  ta.clear();
}

TEST(TestTraceAnalyzer, Children)
{
  qi::TraceAnalyzer ta;
  qi::AnyValue noargs;
  // try all possible orders
  std::vector<qi::EventTrace> v;
  v.push_back(qi::EventTrace(10, EventTrace::Event_Result, 100, noargs, ts(20), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(10, EventTrace::Event_Call,   100, noargs, ts(10), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(11, EventTrace::Event_Result, 101, noargs, ts(14), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(11, EventTrace::Event_Call,   101, noargs, ts(12), 0, 0, 50, 50));
  int permutator[] = { 0,1,2,3};
  unsigned count = 0;
  do
  {
    ++count;
    ta.clear();
    for (unsigned i=0; i<4; ++i)
      ta.addTrace(v[permutator[i]], 1);
    std::set<qi::TraceAnalyzer::FlowLink> fl;
    ta.analyze(fl);
    EXPECT_EQ(ta.dumpTraces(), "50 10:1.100< 11:1.101>\n");
  } while (std::next_permutation(permutator, permutator + 4));
  EXPECT_EQ(24U, count);
}

TEST(TestTraceAnalyzer, AsyncChildren)
{
  qi::TraceAnalyzer ta;
  qi::AnyValue noargs;
  std::vector<qi::EventTrace> v;
  v.push_back(qi::EventTrace(10, EventTrace::Event_Result, 100, noargs, ts(20), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(10, EventTrace::Event_Call,   100, noargs, ts(10), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(11, EventTrace::Event_Result, 101, noargs, ts(25), 0, 0, 50, 51));
  v.push_back(qi::EventTrace(11, EventTrace::Event_Call,   101, noargs, ts(21), 0, 0, 50, 51, ts(12)));
  int permutator[] = { 0,1,2,3};
  unsigned count = 0;
  do
  {
    ++count;
    ta.clear();
    qiLogDebug() << "New iteration";
    for (unsigned i=0; i<4; ++i)
      ta.addTrace(v[permutator[i]], 1);
    std::set<qi::TraceAnalyzer::FlowLink> fl;
    ta.analyze(fl);
    ASSERT_EQ(sort(ta.dumpTraces()), "50 10:1.100{11,}\n51 11:1.101\n");
  } while (std::next_permutation(permutator, permutator + 4));
  EXPECT_EQ(24U, count);
}


TEST(TestTraceAnalyzer, BogusChildren)
{
  qi::TraceAnalyzer ta;
  qi::AnyValue noargs;
  std::vector<qi::EventTrace> v;
  v.push_back(qi::EventTrace(10, EventTrace::Event_Result, 100, noargs, ts(20), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(10, EventTrace::Event_Call,   100, noargs, ts(10), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(11, EventTrace::Event_Result, 101, noargs, ts(30), 0, 0, 50, 50));
  v.push_back(qi::EventTrace(11, EventTrace::Event_Call,   101, noargs, ts(21), 0, 0, 50, 50));
  int permutator[] = { 0,1,2,3};
  unsigned count = 0;
  do
  {
    ++count;
    ta.clear();
    qiLogDebug() << "New iteration";
    for (unsigned i=0; i<4; ++i)
      ta.addTrace(v[permutator[i]], 1);
    std::set<qi::TraceAnalyzer::FlowLink> fl;
    ta.analyze(fl);
    ASSERT_EQ(sort(ta.dumpTraces()), "50 10:1.100 11:1.101\n");
  } while (std::next_permutation(permutator, permutator + 4));
  EXPECT_EQ(24U, count);
}
