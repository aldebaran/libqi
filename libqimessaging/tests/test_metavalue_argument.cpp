/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/servicedirectory.hpp>


qi::GenericValue v;
static qi::Promise<int> *payload;

void onFire(const int& pl)
{
  std::cout << "onFire:" << pl << std::endl;
  std::cout.flush();
  payload->setValue(pl);
}

void value(qi::GenericValue mv)
{
  v = mv.clone();
  payload->setValue(0);
}

void valueList(std::vector<qi::GenericValue> mv)
{
  v = qi::toValue(mv).clone();
  payload->setValue(0);
}

class TestObject: public ::testing::Test
{
public:
  TestObject()
  {
    qi::GenericObjectBuilder ob;
    ob.advertiseEvent<void (*)(const int&)>("fire");
    ob.advertiseMethod("value", &value);
    ob.advertiseMethod("value", &valueList);
    oserver = ob.object();
  }

protected:
  void SetUp()
  {
    ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
    ASSERT_TRUE(session.connect(sd.listenUrl()));
    ASSERT_TRUE(session.listen("tcp://0.0.0.0:0"));
    ASSERT_GT(session.registerService("coin", oserver).wait(), 0);
    EXPECT_EQ(1U, session.services(qi::Session::ServiceLocality_Local).value().size());

    ASSERT_TRUE(sclient.connect(sd.listenUrl()));
    std::vector<qi::ServiceInfo> services = sclient.services();
    EXPECT_EQ(2U, services.size());
    oclient = sclient.service("coin");
    payload = &prom;
  }

  void TearDown()
  {
    payload = 0;
    sclient.close();
    session.close();
    sd.close();
  }

public:
  qi::Promise<int>     prom;
  qi::ServiceDirectory sd;
  qi::Session          session;
  qi::ObjectPtr        oserver;
  qi::Session          sclient;
  qi::ObjectPtr        oclient;
};


TEST_F(TestObject, meta)
{
  using namespace qi;
  qi::int64_t time = os::ustime();
  // Remote test
  ObjectPtr target = oclient;
  {
    /* WATCH OUT, qi::AutoGenericValue(12) is what call expects!
    * So call(AutoGenericValue(12)) will *not* call with the value
    * "a metavalue containing 12", it will call with "12".
    */
  target->call<void>("value", 12).wait();
  ASSERT_EQ(v.asDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>("value", myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>("value", GenericValue(AutoGenericValue(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0f))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue("foo"))).wait();
  ASSERT_EQ(v.asString(), "foo");
  target->call<void>("value", "foo").wait();
  ASSERT_EQ(v.asString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>("value", in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValue(12));
  args.push_back(AutoGenericValue("foo"));
  args.push_back(AutoGenericValue(in));
  target->call<void>("value", args).wait();
  ASSERT_EQ(v.kind(), Type::List);
  GenericList l = v.asList();
  GenericListIterator i = l.begin();
  // iterate
  ASSERT_EQ(12, (*i).asDouble());
  i++;
  ASSERT_EQ("foo", (*i).asString());
  i++;
  ASSERT_EQ(in, (*i).as<std::vector<double> >());
  }
  qiLogVerbose("test") << "remote us: " << os::ustime() - time;
  time = os::ustime();

  // Plugin copy test
  target = oserver;
  {
  target->call<void>("value", 12).wait();
  ASSERT_EQ(v.asDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>("value", myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>("value", GenericValue(AutoGenericValue(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0f))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue("foo"))).wait();
  ASSERT_EQ(v.asString(), "foo");
  target->call<void>("value", "foo").wait();
  ASSERT_EQ(v.asString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>("value", in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>("value", in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValue(12));
  args.push_back(AutoGenericValue("foo"));
  args.push_back(AutoGenericValue(in));
  target->call<void>("value", args).wait();
  ASSERT_EQ(v.kind(), Type::List);
  GenericList l = v.asList();
  GenericListIterator i = l.begin();
  // iterate
  ASSERT_EQ(12, (*i).asDouble());
  i++;
  ASSERT_EQ("foo", (*i).asString());
  i++;
  ASSERT_EQ(in, (*i).as<std::vector<double> >());
  }
  qiLogVerbose("test") << "plugin async us: " << os::ustime() - time;
  time = os::ustime();
  // plugin direct test
  target->moveToEventLoop(0);
  {
  target->call<void>("value", 12).wait();
  ASSERT_EQ(v.asDouble(), 12);
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>("value", myint);
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  {
    int myint = 12;
    qi::Future<void> fut = target->call<void>("value", GenericValue(AutoGenericValue(myint)));
    myint = 5;
    fut.wait();
    ASSERT_EQ(v.asDouble(), 12);
  }
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(12.0f))).wait();
  ASSERT_EQ(v.asDouble(), 12);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue("foo"))).wait();
  ASSERT_EQ(v.asString(), "foo");
  target->call<void>("value", "foo").wait();
  ASSERT_EQ(v.asString(), "foo");
  std::vector<double> in;
  in.push_back(1); in.push_back(2);
  target->call<void>("value", qi::GenericValue(qi::AutoGenericValue(in))).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  target->call<void>("value", in).wait();
  ASSERT_EQ(v.as<std::vector<double> >(), in);
  std::vector<GenericValue> args;
  args.push_back(AutoGenericValue(12));
  args.push_back(AutoGenericValue("foo"));
  args.push_back(AutoGenericValue(in));
  target->call<void>("value", args).wait();
  ASSERT_EQ(v.kind(), Type::List);
  GenericList l = v.asList();
  GenericListIterator i = l.begin();
  // iterate
  ASSERT_EQ(12, (*i).asDouble());
  i++;
  ASSERT_EQ("foo", (*i).asString());
  i++;
  ASSERT_EQ(in, (*i).as<std::vector<double> >());
  }
  qiLogVerbose("test") << "plugin sync us: " << os::ustime() - time;
  time = os::ustime();
}


std::pair<qi::GenericValue, bool> c1 (const qi::GenericValue& src, qi::Type* dst)
{
  return src.convert(dst);
}

template<typename D, typename S>
qi::uint64_t benchConvOne(const S& src,
  std::pair<qi::GenericValue, bool> (*conv)(const qi::GenericValue&, qi::Type*)
  , unsigned niter = 10000)
{
  using namespace qi;
  qi::uint64_t start = os::ustime();
  for (unsigned i=0; i<niter;++i)
  {
    GenericValue gsrc = toValue(src);
    Type* tdst = typeOf<D>();
    std::pair<GenericValue, bool> result = conv(gsrc, tdst);
    if (result.second)
      result.first.destroy();
  }
  return os::ustime() - start;
}

template<typename D, typename S> void benchNative(const S& src, unsigned int niter)
{
  for (unsigned i=0; i<niter;++i)
  {
    D value;
    value = src;
    (void)value;
  }
}

template<typename D, typename S> void benchNative(const std::vector<S>& src, unsigned int niter)
{
  for (unsigned i=0; i<niter;++i)
  {
    D dst;
    for (unsigned i=0; i<src.size(); ++i)
      dst.push_back(src[i]);
    (void)dst;
  }
}

template<typename D, typename K, typename V> void benchNative(const std::map<K,V>& src, unsigned int niter)
{
  for (unsigned i=0; i<niter;++i)
  {
    D dst;
    typename std::map<K, V>::const_iterator it;
    for (it = src.begin(); it != src.end(); ++it)
      dst.insert(typename D::value_type(it->first, it->second));

    (void)dst;
  }
}

template<typename D, typename S> void benchConv(const std::string& what, const S& src, unsigned niter = 10000)
{
  using namespace qi;
  qi::uint64_t res;
  res = benchConvOne<D>(src, &c1, niter);
  qi::uint64_t start = os::ustime();
  benchNative<D>(src, niter);
  qi::uint64_t res2 = os::ustime()-start;
  qiLogInfo("qi.test") << what <<" " << res << " native " << res2 << std::endl;
}

#define BENCH(type, typestring, val)               \
  benchConv<int>(typestring "->i", val);           \
  benchConv<unsigned int>(typestring "->I", val);  \
  benchConv<long long>(typestring "->ll", val);    \
  benchConv<float>(typestring "->f", val);         \
  benchConv<double>(typestring "->d", val);        \
  benchConv<char>(typestring "->c", val);


TEST(GenericValue, converters)
{
  std::vector<int> vi;
  for (unsigned i=0; i<10; ++i)
    vi.push_back(i);
  benchConv<std::vector<unsigned int> >("v[i] -> v[I]", vi);
  benchConv<float>("int" "->f", 12);
  BENCH(int, "i", 12);
  BENCH(float, "f", 1.0f);
  BENCH(double, "d", 1.0);
  BENCH(char, "c", 1.0);
  BENCH(unsigned long, "L", 1.0);

  // STRING BENCH
  benchConv<std::string>("s -> s", std::string("foo"));
  benchConv<std::string>("[c] -> s", "foo");
  // VECTOR BENCH
  benchConv<std::vector<int> >("v[i] -> v[i]", vi);
  benchConv<std::vector<unsigned int> >("v[i] -> v[I]", vi);
  benchConv<std::vector<double> >("v[i] -> v[d]", vi);
  benchConv<std::vector<float> >("v[i] -> v[f]", vi);
  benchConv<std::list<int> >("v[i] -> l[i]", vi);
  benchConv<std::list<unsigned int> >("v[i] -> l[I]", vi);
  benchConv<std::list<double> >("v[i] -> l[d]", vi);
  for (unsigned i=0; i<100; ++i)
    vi.push_back(i);
  benchConv<std::vector<int> >("v[i] -> v[i]", vi);
  benchConv<std::vector<unsigned int> >("v[i] -> v[I]", vi);
  benchConv<std::vector<double> >("v[i] -> v[d]", vi);
  benchConv<std::vector<float> >("v[i] -> v[f]", vi);
  benchConv<std::list<int> >("v[i] -> l[i]", vi);
  benchConv<std::list<unsigned int> >("v[i] -> l[I]", vi);
  benchConv<std::list<double> >("v[i] -> l[d]", vi);

  // MAP BENCH
  std::map<std::string, unsigned int> map;
  for (unsigned i=0; i<10; ++i)
    map[boost::lexical_cast<std::string>(i)] = i;

  benchConv<std::map<std::string, unsigned int> >("m[sI] -> m[sI]", map);
  benchConv<std::map<std::string, int> >("m[sI] -> m[si]", map);
  benchConv<std::map<std::string, float> >("m[sI] -> m[sf]", map);
  benchConv<std::map<std::string, double> >("m[sI] -> m[sd]", map);

  for (unsigned i=0; i<100; ++i)
    map[boost::lexical_cast<std::string>(i)] = i;

  benchConv<std::map<std::string, unsigned int> >("m[sI] -> m[sI]", map);
  benchConv<std::map<std::string, int> >("m[sI] -> m[si]", map);
  benchConv<std::map<std::string, float> >("m[sI] -> m[sf]", map);
  benchConv<std::map<std::string, double> >("m[sI] -> m[sd]", map);

  /*
  for (unsigned i=0; i<1000; ++i)
    map[boost::lexical_cast<std::string>(i)] = i;

  benchConv<std::map<std::string, unsigned int> >("m[sI] -> m[sI]", map);
  benchConv<std::map<std::string, int> >("m[sI] -> m[si]", map);
  benchConv<std::map<std::string, float> >("m[sI] -> m[sf]", map);
  benchConv<std::map<std::string, double> >("m[sI] -> m[sd]", map);
  */
}
int main(int argc, char *argv[])
{
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
