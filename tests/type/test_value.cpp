/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <functional>
#include <tuple>
#include <optional>
#include <gtest/gtest.h>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <ka/functional.hpp>
#include <ka/testutils.hpp>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/jsoncodec.hpp>
#include <qi/path.hpp>

using namespace qi;
qiLogCategory("test");

TEST(Value, Ref)
{
  std::string s("foo");
  AutoAnyReference r(s);
  r.update("bar");
  ASSERT_EQ("bar", s);
  ASSERT_EQ("bar", r.toString());
  ASSERT_ANY_THROW(r.update(5));
  double d = 12;
  AutoAnyReference rd(d);
  rd.update(15);
  ASSERT_EQ(d, 15);
  AnyReference p = AnyReference::fromPtr(&d);
  AutoAnyReference vr(p);
  vr.update(16);
  ASSERT_EQ(d, 16);
}

TEST(Value, InPlaceSet)
{
  std::string s("foo");
  AutoAnyReference v(s);
  v.setString("bar");
  ASSERT_EQ("bar", s);
  v.setString("pifpouf");
  ASSERT_EQ("pifpouf", s);
  double d = 12;
  v = AutoAnyReference(d);
  v.setDouble(15.5);
  ASSERT_EQ(15.5, d);
  v.setInt(16);
  ASSERT_EQ(16.0, d);
  int i = 14;
  v = AutoAnyReference(i);
  v.setInt(13);
  ASSERT_EQ(13, i);
}

TEST(Value, Update)
{
  std::string s("foo");
  AutoAnyReference v(s);
  v.update(AutoAnyReference("bar"));
  ASSERT_EQ("bar", s);
  v.update(AutoAnyReference(std::string("baz")));
  ASSERT_EQ("baz", s);
  ASSERT_ANY_THROW(v.update(AutoAnyReference(42)));
  double d = 5.0;
  v = AutoAnyReference(d);
  v.update(AutoAnyReference(42));
  ASSERT_EQ(42, d);
  v.update(AutoAnyReference(42.42));
  ASSERT_DOUBLE_EQ(42.42, d);
  ASSERT_ANY_THROW(v.update(AutoAnyReference("bar")));
}

TEST(Value, Conversion)
{
  EXPECT_NO_THROW(qi::AnyValue::from(18).to<float>());
  EXPECT_ANY_THROW(qi::AnyValue::from(18).to<bool>());
  EXPECT_ANY_THROW(qi::AnyValue::from(18).to<std::string>());

  EXPECT_NO_THROW(qi::AnyValue::from(18.0).to<int>());
  EXPECT_ANY_THROW(qi::AnyValue::from(18.0).to<bool>());
  EXPECT_ANY_THROW(qi::AnyValue::from(18.0).to<std::string>());

  EXPECT_EQ(1, qi::AnyValue::from(true).to<int>());
  EXPECT_EQ(1.0, qi::AnyValue::from(true).to<float>());

  EXPECT_ANY_THROW(qi::AnyValue(qi::typeOf<void>()).to<int>());
  EXPECT_ANY_THROW(qi::AnyValue(qi::typeOf<void>()).to<std::string>());
  EXPECT_ANY_THROW(qi::AnyValue(qi::typeOf<void>()).to<float>());
  EXPECT_ANY_THROW(qi::AnyValue(qi::typeOf<void>()).to<bool>());

  EXPECT_ANY_THROW(qi::AnyValue::from("bla").to<int>());
  EXPECT_ANY_THROW(qi::AnyValue::from("bla").to<float>());
  EXPECT_ANY_THROW(qi::AnyValue::from("bla").to<bool>());
}

TEST(Value, As)
{
  std::string s("foo");
  AutoAnyReference v(s);
  ASSERT_EQ(&v.asString(), &s);
  ASSERT_ANY_THROW(v.asDouble());
  double d = 1.5;
  v = AutoAnyReference(d);
  ASSERT_EQ(&d, &v.asDouble());
  ASSERT_ANY_THROW(v.asInt32());
  qi::uint32_t ui = 2; // vcxx uint32_t unqualified is ambiguous.
  v = AutoAnyReference(ui);
  ASSERT_EQ(&ui, &v.asUInt32());
  ASSERT_ANY_THROW(v.asInt32());
  ASSERT_ANY_THROW(v.asInt16());
}

TEST(Value, InvalidValue)
{
  AnyValue v;
  EXPECT_FALSE(v.isValid());
  EXPECT_EQ(AnyValue{}, v);
}

TEST(Value, InvalidReference)
{
  AnyValue v;
  auto r = v.asReference();
  EXPECT_FALSE(r.isValid());
  EXPECT_EQ(AnyReference{}, r);
}

TEST(Value, Int)
{
  AnyReference v;
  int twelve = 12;
  v = AutoAnyReference(twelve);
  ASSERT_TRUE(v.type() != 0);
  ASSERT_TRUE(v.rawValue() != 0);
  ASSERT_EQ(v.toInt(), 12);
  ASSERT_EQ(v.toFloat(), 12.0f);
  ASSERT_EQ(v.toDouble(), 12.0);
}

TEST(Value, Float)
{
  AnyReference v;
  double five = 5.0;
  v = AutoAnyReference(five);
  ASSERT_EQ(v.toInt(), 5);
  ASSERT_EQ(v.toFloat(), 5.0f);
  ASSERT_EQ(v.toDouble(), 5.0);
}

TEST(Value, String)
{
  AnyReference v;
  auto foo = "foo";
  v = AutoAnyReference(foo);
  ASSERT_EQ(foo, v.toString());
}

TEST(Value, SizeThrowsOnIncorrectUsage)
{
  EXPECT_ANY_THROW(AnyValue{}.size());
  EXPECT_ANY_THROW(AnyValue{}.asReference().size());
  EXPECT_ANY_THROW(AnyReference(AutoAnyReference(12)).size());
  EXPECT_ANY_THROW(AnyReference(AutoAnyReference(5.0)).size());
  EXPECT_ANY_THROW(AnyReference(AutoAnyReference("foo")).size());
}

namespace {
  using InstrumentedReg = ka::instrumented_regular_t<
    int,
    std::function<void (ka::regular_op_t)>
  >;
} // namespace

QI_TYPE_STRUCT_REGISTER(InstrumentedReg, value);

TEST(ValueCounts, CopyCopiesUnderlyingValue)
{
  using std::get;
  ka::regular_counters_t counters {{}};
  InstrumentedReg original;
  init_with_counters(&original, 5, &counters);

  AnyValue v0 = AnyValue::from(original);
  ASSERT_EQ(original, v0.as<InstrumentedReg>());

  // Check that a copy causes the copy count to be incremented by 1.
  ka::reset(counters);
  AnyValue v1{v0};
  EXPECT_EQ(1, counters[ka::regular_op_copy]);
  EXPECT_EQ(0, counters[ka::regular_op_move]);
  EXPECT_EQ(0, counters[ka::regular_op_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_move_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_destroy]);
  EXPECT_EQ(original, v0.as<InstrumentedReg>());
  EXPECT_EQ(original, v1.as<InstrumentedReg>());
}

TEST(ValueCounts, AssignCopiesUnderlyingValue)
{
  using std::get;
  ka::regular_counters_t counters {{}};
  InstrumentedReg original;
  init_with_counters(&original, 6, &counters);

  AnyValue v0 = AnyValue::from(original);
  ASSERT_EQ(original, v0.as<InstrumentedReg>());

  // Check that an assignment causes the copy count to be incremented by 1
  // (it seems it would make more sense to call the assignment operator of the
  // contained value, but this is the current behavior).
  ka::reset(counters);
  AnyValue v1;
  v1 = v0;
  EXPECT_EQ(1, counters[ka::regular_op_copy]);
  EXPECT_EQ(0, counters[ka::regular_op_move]);
  EXPECT_EQ(0, counters[ka::regular_op_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_move_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_destroy]);
  EXPECT_EQ(original, v0.as<InstrumentedReg>());
  EXPECT_EQ(original, v1.as<InstrumentedReg>());
}

TEST(ValueCounts, MoveDoesNotAffectUnderlyingValue)
{
  using std::get;
  ka::regular_counters_t counters {{}};
  InstrumentedReg original;
  init_with_counters(&original, 7, &counters);

  AnyValue v0 = AnyValue::from(original);
  ASSERT_EQ(original, v0.as<InstrumentedReg>());

  auto* type0 = v0.type();
  auto* raw0 = v0.rawValue();

  // Check that a move does not perform any regular operation on the contained
  // value. This is because, internally, `AnyValue` copies pointers only.
  ka::reset(counters);
  AnyValue v1{std::move(v0)};
  EXPECT_EQ(0, counters[ka::regular_op_copy]);
  EXPECT_EQ(0, counters[ka::regular_op_move]);
  EXPECT_EQ(0, counters[ka::regular_op_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_move_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_destroy]);
  EXPECT_EQ(original, v1.as<InstrumentedReg>());

  // The new instance has the right value.
  ASSERT_EQ(original, v1.as<InstrumentedReg>());

  // It has the original instance resources.
  ASSERT_EQ(type0, v1.type());
  ASSERT_EQ(raw0, v1.rawValue());

  // And the original instance resources have been nullified.
  ASSERT_EQ(nullptr, v0.type());
  ASSERT_EQ(nullptr, v0.rawValue());
}

TEST(ValueCounts, MoveAssignDoesNotAffectUnderlyingValue)
{
  using std::get;
  ka::regular_counters_t counters {{}};
  InstrumentedReg original;
  init_with_counters(&original, 8, &counters);

  AnyValue v0 = AnyValue::from(original);
  ASSERT_EQ(original, v0.as<InstrumentedReg>());

  auto* type0 = v0.type();
  auto* raw0 = v0.rawValue();

  // Check that a move assignment does not perform any regular operation on the
  // contained value. This is because, internally, `AnyValue` copies pointers
  // only.
  ka::reset(counters);
  AnyValue v1;
  v1 = std::move(v0);
  EXPECT_EQ(0, counters[ka::regular_op_copy]);
  EXPECT_EQ(0, counters[ka::regular_op_move]);
  EXPECT_EQ(0, counters[ka::regular_op_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_move_assign]);
  EXPECT_EQ(0, counters[ka::regular_op_destroy]);
  EXPECT_EQ(original, v1.as<InstrumentedReg>());

  // The new instance has the right value.
  ASSERT_EQ(original, v1.as<InstrumentedReg>());

  // It has the original instance resources.
  ASSERT_EQ(type0, v1.type());
  ASSERT_EQ(raw0, v1.rawValue());

  // And the original instance resources have been nullified.
  ASSERT_EQ(nullptr, v0.type());
  ASSERT_EQ(nullptr, v0.rawValue());
}

TEST(Value, Map)
{
  std::map<std::string, double> map;
  map["foo"] = 1;
  map["bar"] = 2;
  map["baz"] = 3;
  AutoAnyReference v(map);
  ASSERT_EQ(3u, v.size());

  ASSERT_EQ(v["foo"].toInt(), 1);
  ASSERT_EQ(v[std::string("bar")].toInt(), 2);

  v["baz"].setInt(4);
  ASSERT_EQ(v["baz"].toInt(), 4);

  // write to ref
  v["baz"].update(5);
  ASSERT_EQ(v["baz"].toInt(), 5);

  // Create a new element
  qiLogDebug() << "Insert bimm";
  ASSERT_ANY_THROW(v[AnyValue::from("bimm")].setString("foo"));
  v["bimm"].setInt(42);
  qiLogDebug() << "Check bimm";
  ASSERT_EQ(v["bimm"].toInt(), 42);
  ASSERT_EQ(v[std::string("bimm")].toInt(), 42);


  // Create a new element of an existing string length
  qiLogDebug() << "Insert pif";
  v[AnyValue::from("pif")].setInt(43);
  qiLogDebug() << "Check pif";
  ASSERT_EQ(v["pif"].toInt(), 43);
  ASSERT_EQ(v[std::string("pif")].toInt(), 43);

  // insert existing
  v.insert("pif", 63);
  ASSERT_EQ(v["pif"].toInt(), 63);

  // insert new
  v.insert("pouf", 65);
  ASSERT_EQ(v["pouf"].toInt(), 65);

  ASSERT_EQ(6u, v.size());
  ASSERT_TRUE(!v.find("nokey").type());
  // wrong keytype
  ASSERT_ANY_THROW(v.find(42));
  // append on map
  ASSERT_ANY_THROW(v.append("foo"));
}

TEST(Value, Map_at)
{
  std::map<std::string, double> map;
  map["foo"] = 1;
  map["bar"] = 2;
  AutoAnyReference v(map);
  const AutoAnyReference vc(map);

  // at(T)
  {
    AnyReference val1 = v.at("foo");
    EXPECT_EQ(1, val1.toInt());
    AnyReference valInvalid = v.at("nokey");
    EXPECT_EQ(nullptr, valInvalid.type());
  }
  // at(T) const
  {
    const AnyReference val1 = vc.at("bar");
    EXPECT_EQ(2, val1.toInt());
    const AnyReference valInvalid = vc.at("nokey");
    EXPECT_EQ(nullptr, valInvalid.type());
  }
  // at(AnyReference)
  {
    AnyReference val1 = v.at(AnyReference::from("foo"));
    EXPECT_EQ(1, val1.toInt());
    AnyReference valInvalid = v.at(AnyReference::from("nokey"));
    EXPECT_EQ(nullptr, valInvalid.type());
  }
  // at(AnyReference) const
  {
    const AnyReference val1 = vc.at(AnyReference::from("bar"));
    EXPECT_EQ(2, val1.toInt());
    const AnyReference valInvalid = vc.at(AnyReference::from("nokey"));
    EXPECT_EQ(nullptr, valInvalid.type());
  }
}

static bool triggered = false;
static void nothing(GenericObject*) {triggered = true;}

TEST(Value, AnyObject)
{
  {
    // GenericObject uses intrusive refcount and must be valid
    // Also object does some typechecking
    qi::DynamicObjectBuilder dynBuild;
    qi::AnyObject ori = dynBuild.object(&nothing);
    qi::GenericObject& go = *ori.asGenericObject();

    AnyReference v = AnyReference::from(ori);
    qi::detail::ManagedObjectPtr mo = v.to<qi::detail::ManagedObjectPtr>();
    ASSERT_TRUE(!!mo);
    ASSERT_EQ(mo.get(), &go);
    qi::AnyObject out = v.to<AnyObject>();
    ASSERT_TRUE(out);
    ASSERT_TRUE(out == ori);
    out = v.toObject();
    ASSERT_TRUE(out);
    ASSERT_EQ(out.asGenericObject(), mo.get());
  }
  ASSERT_TRUE(triggered);
}


TEST(Value, list)
{
  int one = 1;
  AnyReference v = AnyReference::from(one);
  v.set(5);
  ASSERT_ANY_THROW(v.set("foo"));
  ASSERT_ANY_THROW(v.set(std::vector<int>()));
  std::vector<int> vint;
  vint.push_back(12);
  v = AnyReference::from(vint);
  v.append(7);
  ASSERT_EQ(7, v[1].toInt());
  ASSERT_EQ(7, v[1].toFloat());
  ASSERT_EQ(7, v.element<int>(1));
  v[1].setInt(8);
  ASSERT_EQ(8, v[1].toInt());
  v.element<int>(1) = 9;
  ASSERT_EQ(9, v[1].toInt());
  ASSERT_ANY_THROW(v.element<double>(1)); // wrong type
  ASSERT_ANY_THROW(v.element<int>(17));   // out of bound
  EXPECT_EQ(v.as<std::vector<int> >().size(), v.size());
}

TEST(Value, list_at)
{
  std::vector<int> v{2, 5, 7};
  AnyReference list = AnyReference::from(v);
  const AnyReference listc = AnyReference::from(v);

  // at(T)
  {
    AnyReference val = list.at(1);
    EXPECT_EQ(5, val.toInt());
    AnyReference valInvalid = list.at(4);
    EXPECT_EQ(nullptr, valInvalid.type());
  }
  // at(T) const
  {
    const AnyReference val = listc.at(1);
    EXPECT_EQ(5, val.toInt());
    const AnyReference valInvalid = listc.at(4);
    EXPECT_EQ(nullptr, valInvalid.type());
  }
}

TEST(Value, set)
{
  std::vector<int> v, v2;
  std::set<int> s;
  v.push_back(1);
  v.push_back(3);
  v.push_back(2);
  s = AnyReference::from(v).to<std::set<int> >();
  v2.insert(v2.end(), s.begin(), s.end());
  ASSERT_EQ(3u, v2.size());
  EXPECT_EQ(1, v2[0]);
  EXPECT_EQ(2, v2[1]);
  EXPECT_EQ(3, v2[2]);
  v2 = AnyReference::from(s).to<std::vector<int> >();
  ASSERT_EQ(3u, v2.size());
  EXPECT_EQ(1, v2[0]);
  EXPECT_EQ(2, v2[1]);
  EXPECT_EQ(3, v2[2]);
}

struct TStruct
{
  double d;
  std::string s;
  bool operator ==(const TStruct& b) const { return d == b.d && s == b.s;}
};
struct Point2D
{
  int x,y;
  bool operator ==(const Point2D& b) const { return x==b.x && y == b.y;}
};

QI_TYPE_STRUCT(TStruct, d, s);
QI_TYPE_STRUCT(Point2D, x, y);

TEST(Value, Tuple)
{
  // Create a Dynamic tuple from vector
  AnyValueVector v;
  AutoAnyReference gv(v);
  gv.append(AnyValue::from(12.0));
  gv.append(AnyValue::from("foo")); // cstring not std::string
  AnyValue gtuple = gv.toTuple(false);
  TStruct t;
  t.d = 12.0;
  t.s = "foo";
  TStruct gtupleconv = gtuple.to<TStruct>();
  ASSERT_EQ(t, gtupleconv);
  gtuple[0].setDouble(15);
  ASSERT_EQ(15, gtuple.to<TStruct>().d);

  // create a static tuple
  std::vector<double> vd;
  vd.push_back(1);
  gv = AnyReference::from(vd);
  gv.append(2);
  gtuple = gv.toTuple(true);
  Point2D p;
  p.x = 1;
  p.y = 2;
  ASSERT_TRUE(p == p);
  ASSERT_EQ(p , gtuple.to<Point2D>());
  p.x = 3;
  gtuple[0].setDouble(gtuple[0].toDouble() + 2);
  ASSERT_EQ(p, gtuple.to<Point2D>());
}

TEST(Value, StructFromAndToMap)
{
  std::map<std::string, int> asMap;
  asMap["x"] = 1;
  asMap["y"] = 2;
  auto value = AnyValue::from(asMap);

  Point2D expectedStruct;
  expectedStruct.x = 1;
  expectedStruct.y = 2;

  auto asStruct = value.to<Point2D>();
  EXPECT_EQ(expectedStruct, asStruct);
}

struct Point2
{
  Point2()
    : x(0)
    , y(0)
    , z(0)
    , a(0)
    , b(0)
  {}
  const double& getY() { return y;}
  double& getZ() { return z;}
  const double& getA() const { return a;}
  double x,y, z, a, b;
  std::string s;
private:
  Point2(double x, double y, const std::string& s,
    double z, double a
    , double b
    )
  : x(x), y(y), z(z), a(a)
  , b(b), s(s)
  {}
  friend class ::qi::TypeImpl<Point2>;
};

const double& get_point2_b(Point2* ptr)
{
  return ptr->b;
}

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(Point2,
  ("x", x),
  ("y", getY),
  ("str", s),
  ("z", getZ),
  QI_STRUCT_FIELD("a", getA),
  QI_STRUCT_HELPER("b", get_point2_b)
  );

TEST(Value, Tuple2)
{
  Point2 p;
  StructTypeInterface* t = static_cast<qi::StructTypeInterface*>(qi::typeOf<Point2>());
  ASSERT_EQ(6u, t->memberTypes().size());
  EXPECT_EQ("(ddsddd)<Point2,x,y,str,z,a,b>", t->signature());
  AnyValueVector vd;
  vd.push_back(AnyValue(AutoAnyReference(1.5)));
  vd.push_back(AnyValue(AutoAnyReference(2.5)));
  vd.push_back(AnyValue(AutoAnyReference("coin")));
  vd.push_back(AnyValue(AutoAnyReference(1.5)));
  vd.push_back(AnyValue(AutoAnyReference(1.5)));
  vd.push_back(AnyValue(AutoAnyReference(3.5)));
  p = AnyReference::from(vd).toTuple(true).to<Point2>();
  EXPECT_EQ(1.5, p.x);
  EXPECT_EQ(2.5, p.y);
  EXPECT_EQ("coin", p.s);
  EXPECT_EQ(3.5, p.b);
  AnyReferenceVector pcomps = t->values(&p);
  EXPECT_EQ(1.5, pcomps[0].toDouble());
  EXPECT_EQ(2.5, pcomps[1].toDouble());
  EXPECT_EQ("coin", pcomps[2].toString());
  EXPECT_EQ(3.5, pcomps[5].toDouble());
}

TEST(Value, DefaultMap)
{ // this one has tricky code and deserves a test)
  TypeInterface* dmt = TypeInterface::fromSignature(qi::Signature("{si}"));
  AnyValue val = AnyValue(AnyReference(dmt), false, true);
  ASSERT_EQ(0u, val.size());
  val["foo"].update(12);
  ASSERT_EQ(1u, val.size());
  ASSERT_EQ(12, val.find("foo").toInt());
  ASSERT_EQ(0, val.find("bar").type());
  val.insert(std::string("bar"), 13);
  ASSERT_EQ(13, val.element<int>("bar"));
  val.element<int>("foo") = 10;
  ASSERT_EQ(10, val.find("foo").toInt());
  AnyIterator b = val.begin();
  AnyIterator end = val.end();
  qi::int64_t sum = 0;
  while (b != end)
  {
    AnyReference deref = *b;
    sum += deref[1].toInt();
    ++b;
  }
  ASSERT_EQ(23, sum);

  AnyIterator b2 = val.begin();
  AnyIterator end2 = val.end();
  sum = 0;
  while (b2 != end2)
  {
    AnyReference deref = *b2;
    AnyIterator it = b2;
    sum += deref[1].toInt();
    ASSERT_EQ(b2++, it);
  }
  ASSERT_EQ(23, sum);

  AnyReference valCopy = val.clone();
  ASSERT_EQ(13, valCopy.element<int>("bar"));
  ASSERT_EQ(2u, valCopy.size());
  // reset val, checks valCopy still works
  val.reset();
  val = AnyValue::from(5);
  ASSERT_EQ(13, valCopy.element<int>("bar"));
  ASSERT_EQ(2u, valCopy.size());
  valCopy.destroy();
}


TEST(Value, STL)
{
  std::vector<int> v;
  AutoAnyReference gv(v);
  gv.append(1);
  gv.append(3);
  gv.append(2);
  std::vector<int> w;
#if defined(_MSC_VER) && _MSC_VER <= 1500
  // Yup, it's broken.
  for (unsigned i=0; i<3; ++i)
    v.push_back(gv[i].toInt());
#else
  // seems there are overloads for push_back, need to explicitly cast to signature
  std::for_each(gv.begin(), gv.end(),
    boost::lambda::bind((void (std::vector<int>::*)(const int&))&std::vector<int>::push_back,
      boost::ref(w),
      boost::lambda::bind(&AnyReference::toInt, boost::lambda::_1)));
#endif
  ASSERT_EQ(3u, w.size());
  ASSERT_EQ(v, w);
  AnyIterator mine = std::min_element(gv.begin(), gv.end());
  ASSERT_EQ(1, (*mine).toInt());
  mine = std::find_if(gv.begin(), gv.end(),
    boost::lambda::bind(&AnyReference::toInt, boost::lambda::_1) == 3);
  ASSERT_EQ(3, (*mine).toInt());
  (*mine).setInt(4);
  ASSERT_EQ(4, v[1]);
  mine = std::find_if(gv.begin(), gv.end(),
    boost::lambda::bind(&AnyReference::toInt, boost::lambda::_1) == 42);
  ASSERT_EQ(mine, gv.end());

//TODO: we do not want that anymore?
  std::vector<int> v2;
  v2.push_back(10);
  v2.push_back(1);
  v2.push_back(100);
  // v has correct size
  for (unsigned int i = 0; i < v2.size(); ++i) {
    gv[i].update(v2[i]);
  }
  //std::copy(v2.begin(), v2.end(), gv.begin());
  ASSERT_EQ(v2, v);
  // copy other-way-round requires cast from AnyReference to int


  AnyReferenceVector vg;
  for (unsigned i=0; i<v.size(); ++i)
    vg.push_back(AnyReference::from(v[i]));
  std::sort(vg.begin(), vg.end());
  ASSERT_EQ(321, vg[0].toInt() + vg[1].toInt()*2 + vg[2].toInt() * 3);
}

TEST(Value, DISABLED_MetaObjectSignature)
{
  std::cerr << qi::typeOf<qi::MetaObject>()->signature().toString() << std::endl;
}

TEST(Value, Overflow)
{
  {
    long long test = 0xFFFF11223344LL; // paint the stack with nonzero
    (void)test; //remove unused warning
  }
  long twelve = 12;
  AnyReference::from(12).to<char>();
  ASSERT_EQ(12,AnyReference::from(twelve).to<qi::int64_t>());
  AnyReference::from(127).to<char>();
  ASSERT_ANY_THROW(AnyReference::from(128).to<char>());
  AnyReference::from(-128).to<char>();
  ASSERT_ANY_THROW(AnyReference::from(254).to<char>());
  AnyReference::from(255).to<unsigned char>();
  AnyReference::from(0xFF11223344).to<qi::int64_t>();
  AnyReference::from(0xFF11223344).to<qi::uint64_t>();
  ASSERT_ANY_THROW(AnyReference::from(256).to<unsigned char>());
  ASSERT_ANY_THROW(AnyReference::from(-1).to<unsigned char>());
  // check correct sign propagation
  ASSERT_EQ(-120, AnyReference::from((char)-120).to<int>());
  AnyReference::from(12).to<double>();
  AnyReference::from(13).to<float>();
  ASSERT_ANY_THROW(AnyReference::from(-5.0).to<unsigned int>());
  ASSERT_ANY_THROW(AnyReference::from(-5.0f).to<unsigned int>());
  ASSERT_ANY_THROW(AnyReference::from(-256.0).to<char>());
  ASSERT_ANY_THROW(AnyReference::from(256.0).to<unsigned char>());
  ASSERT_ANY_THROW(AnyReference::from(1.0e80).to<qi::uint64_t>());
  ASSERT_TRUE(AnyReference::from((qi::uint32_t)0xFF223344).to<double>() > 0);
  ASSERT_TRUE(AnyReference::from((qi::int32_t)0xFF223344).to<double>() < 0);
  ASSERT_TRUE(AnyReference::from(0xFF22334455667788ULL).to<double>() > 0);
  ASSERT_TRUE(AnyReference::from((qi::int64_t)0xFF22334455667788).to<double>() < 0);
  // check other access path
  ASSERT_ANY_THROW(AnyValue::make<char>().setInt(128));
  ASSERT_ANY_THROW(AnyValue::make<char>().update(AnyReference::from(128)));
}

TEST(Value, Convert_ListToTuple)
{
  qi::TypeInterface *type = qi::TypeInterface::fromSignature("(fsf[s])");
  qi::AnyValue gv1 = qi::decodeJSON("[42, \"plop\", 1.42, [\"a\", \"b\"]]");
  qi::AnyValue gv2 = qi::decodeJSON("[42, \"plop\", 1.42, [\"a\", 42]]");

  auto res1 = gv1.convert(type);
  auto res2 = gv2.convert(type);

  ASSERT_FALSE(res2->type());
  ASSERT_TRUE(res1->type() != 0);
  ASSERT_EQ(res1->type()->info(), type->info());
  ASSERT_EQ(gv1.size(), res1->size());
  ASSERT_STREQ("b", (*res1)[3][1].asString().c_str());

  qi::TypeInterface *dest3 = qi::TypeInterface::fromSignature("(fffI)");
  qi::AnyValue gv3 = qi::decodeJSON("[1.1, 2.2, 3.3, \"42\"]");
  auto res3 = gv3.convert(dest3);
  ASSERT_FALSE(res3->type());
}

TEST(Value, Convert_ListToMap)
{
  qi::TypeInterface *type1= qi::TypeInterface::fromSignature("{if}");
  qi::AnyValue gv1 = qi::decodeJSON("[[10.10, 42.42], [20, 43], [30, 44.44]]");
  auto res1 = gv1.convert(type1);
  ASSERT_TRUE(res1->type() != 0);
  ASSERT_EQ(res1->type()->info(), type1->info());
  ASSERT_EQ(gv1.size(), res1->size());
  ASSERT_EQ(44.44f, (*res1)[30].asFloat());

  qi::TypeInterface *type2 = qi::TypeInterface::fromSignature("{if}");
  qi::AnyValue gv2 = qi::decodeJSON("[[10.10, 42.42], [20, 43], [\"plop\", 44.44]]");
  auto res2 = gv2.convert(type2);
  ASSERT_FALSE(res2->type());
}

struct EasyStruct
{
  int x;
  bool y;
  std::vector<std::string> strings;
};
QI_TYPE_STRUCT_REGISTER(EasyStruct, x, y, strings);

TEST(Value, Convert_StructToMap)
{
  const EasyStruct es{1, false, {"toto", "tata"}};
  qi::AnyReference ref;
  ASSERT_NO_THROW(ref = qi::AnyReference::from(es));
  std::map<std::string, qi::AnyValue> val = ref.toMap<std::string, qi::AnyValue>();
  EXPECT_EQ(es.x, val["x"].toInt());
  EXPECT_EQ(es.y, val["y"].to<bool>());
  EXPECT_EQ(es.strings, val["strings"].toList<std::string>());
}

namespace
{
struct Foo
{
  std::string str;
  double dbl;

  friend bool operator<(const Foo& a, const Foo&b)
  {
    return std::make_tuple(a.str, a.dbl) < std::make_tuple(b.str, b.dbl);
  }
};

struct Oof
{
  double dbl;
  std::string str;
};
} // anonymous

QI_TYPE_STRUCT_REGISTER(Foo, str, dbl);
QI_TYPE_STRUCT_REGISTER(Oof, dbl, str);

TEST(Struct, FieldSwap)
{
  Foo f;
  f.str = "foo";
  f.dbl = 42.42;
  Oof o = qi::AnyReference::from(f).to<Oof>();
  EXPECT_EQ(42.42, o.dbl);
  EXPECT_EQ("foo", o.str);
  f = qi::AnyReference::from(o).to<Foo>();
  EXPECT_EQ(42.42, f.dbl);
  EXPECT_EQ("foo", f.str);
}

struct FooBase
{
  int x;
  int y;
  int z;
};
QI_TYPE_STRUCT_REGISTER(FooBase, x, y, z);
struct OtherBase
{
  int x;
  int z;
};
QI_TYPE_STRUCT_REGISTER(OtherBase, x, z);
struct FooEx
{
  int x;
  int y;
};
QI_TYPE_STRUCT_EXTENSION_DROPPED_FIELDS(FooEx, "z");
QI_TYPE_STRUCT_REGISTER(FooEx, x, y);

TEST(Struct, ExtendDrop)
{
  FooBase e = { 1, 2, 3 };
  FooEx f = qi::AnyReference::from(e).to<FooEx>();
  EXPECT_EQ(1, f.x);
  EXPECT_EQ(2, f.y);
  EXPECT_ANY_THROW(qi::AnyReference::from(e).to<OtherBase>());

  FooBase e2 = qi::AnyReference::from(f).to<FooBase>();
  EXPECT_EQ(1, e2.x);
  EXPECT_EQ(2, e2.y);
  EXPECT_EQ(0, e2.z); // filled with default value

  OtherBase b = { 1, 3 };
  EXPECT_ANY_THROW(qi::AnyReference::from(b).to<FooEx>());
}

struct FooEx2
{
  int x;
  int y;
  std::string s;
};


QI_TYPE_STRUCT_EXTENSION_DROPPED_FIELDS(FooEx2, "z");
QI_TYPE_STRUCT_EXTENSION_ADDED_FIELDS(FooEx2, "s");
QI_TYPE_STRUCT_REGISTER(FooEx2, x, y, s);

TEST(Struct, ExtendFillDrop)
{
  FooBase f = { 1, 2, 3 };
  FooEx2 f2 = qi::AnyReference::from(f).to<FooEx2>();
  EXPECT_EQ(1, f2.x);
  EXPECT_EQ(2, f2.y);
  EXPECT_TRUE(f2.s.empty());

  FooEx f1 = { 1, 2 };
  f2 = qi::AnyReference::from(f1).to<FooEx2>();
  EXPECT_EQ(1, f2.x);
  EXPECT_EQ(2, f2.y);
  EXPECT_TRUE(f2.s.empty());
}

struct FooExPub
{
  FooEx2 fooex2;
};

static FooEx2* get_priv(FooExPub* pub)
{
  return &pub->fooex2;
}

QI_TYPE_STRUCT_BOUNCE_REGISTER(FooExPub, FooEx2, get_priv);

TEST(Struct, ExtendFillBounce)
{
  FooBase f = { 1, 2, 3 };
  FooExPub f2 = qi::AnyReference::from(f).to<FooExPub>();
  EXPECT_EQ(1, f2.fooex2.x);
  EXPECT_EQ(2, f2.fooex2.y);
  EXPECT_TRUE(f2.fooex2.s.empty());
  FooEx f1 = { 1, 2 };
  f2 = qi::AnyReference::from(f1).to<FooExPub>();
  EXPECT_EQ(1, f2.fooex2.x);
  EXPECT_EQ(2, f2.fooex2.y);
  EXPECT_TRUE(f2.fooex2.s.empty());
}

struct Color
{
  int r,g,b;
};
// No inheritance, we emulate Color and ColorA being the same struct in
// two different CUs or SOs.
struct ColorA
{
  int r,g,b,a;
};

// only allow drop of a if it equals 1
bool colorDropHandler(std::map<std::string, ::qi::AnyValue>& /*fields*/,
                      const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                      const std::map<std::string, ::qi::AnyReference>& dropfields)
{
  try
  {
    if (!missing.empty())
      return false;
    if (dropfields.size() != 1 || dropfields.begin()->first != "a")
      return false;
    return dropfields.begin()->second.toInt() == 1;
  }
  catch (...)
  {
    return false;
  }
}

bool colorFillHandler(std::map<std::string, ::qi::AnyValue>& fields,
                      const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                      const std::map<std::string, ::qi::AnyReference>& dropfields)
{
  if (!dropfields.empty())
    return false;
  if (missing.size() != 1 || std::get<0>(missing.front()) != "a")
    return false;
  fields["a"] = qi::AnyValue::from(1);
  return true;
}

QI_TYPE_STRUCT_REGISTER(Color, r, g, b);
QI_TYPE_STRUCT_EXTENSION_CONVERT_HANDLERS(ColorA, colorFillHandler, colorDropHandler);
QI_TYPE_STRUCT_REGISTER(ColorA, r, g, b, a);

TEST(Struct, DropHandler)
{
  ColorA a0 = { 0, 1, 2, 0 }, a1 = { 0, 1, 2, 1 };
  Color c;
  ASSERT_NO_THROW(c = qi::AnyReference::from(a1).to<Color>());
  EXPECT_EQ(a1.r, c.r);
  EXPECT_EQ(a1.g, c.g);
  EXPECT_EQ(a1.b, c.b);
  ASSERT_NO_THROW(a1 = qi::AnyReference::from(c).to<ColorA>());
  EXPECT_EQ(c.r, a1.r);
  EXPECT_EQ(c.g, a1.g);
  EXPECT_EQ(c.b, a1.b);
  EXPECT_EQ(1, a1.a);
  EXPECT_ANY_THROW(qi::AnyReference::from(a0).to<Color>());
}

TEST(Struct, ComplexType)
{
  std::pair<std::vector<Foo>, std::list<FooEx> > p;
  AnyValue::from(p);
  std::pair<std::vector<int>, std::list<std::map<int, Foo> > > p2;
  AnyValue::from(p2);
}

TEST(Append, AppendInvalid)
{
  std::vector<std::string> textArgs;
  textArgs.emplace_back("real");
  textArgs.emplace_back("magic");
  auto args = qi::AnyValue::from(textArgs);

  ASSERT_ANY_THROW(args.append(10));
}

TEST(Insert, InsertInvalid)
{
  std::map<int, std::string> map;
  map[0] = "real";
  auto anyMap = qi::AnyValue::from(map);

  ASSERT_ANY_THROW(anyMap.insert("false", 0));
  ASSERT_ANY_THROW(anyMap.insert(1, 10));
}

TEST(Reference, invalidAsReference)
{
  qi::AnyValue v;
  auto r = v.asReference();
  EXPECT_FALSE(r.isValid());
}

TEST(Reference, referenceFromInvalidWrapsIt)
{
  qi::AnyValue v;
  auto r = qi::AnyReference::from(v);
  EXPECT_TRUE(r.isValid());
  EXPECT_FALSE(r.unwrap().isValid());
  EXPECT_FALSE(r.content().isValid());
}

TEST(QiPath, valueIsPreserved)
{
  auto tmpPath = qi::Path{qi::os::tmp()} / qi::Path{boost::filesystem::unique_path()};
  auto transformedPath = qi::AnyValue::from(tmpPath).to<std::string>();
  EXPECT_EQ(tmpPath.str(), transformedPath);
}

TEST(QiPath, emptyValueIsPreserved)
{
  auto tmpPath = qi::Path{};
  auto transformedPath = qi::AnyValue::from(tmpPath).to<std::string>();
  EXPECT_EQ(tmpPath.str(), transformedPath);
}

template <typename T>
struct ConvertWithTypeInterface: ::testing::Test {
  using TypeInterface = T;
};

using TypeInterfaces = ::testing::Types<
   ListTypeInterface,
   StructTypeInterface,
   MapTypeInterface,
   IntTypeInterface,
   FloatTypeInterface,
   RawTypeInterface,
   StringTypeInterface,
   PointerTypeInterface,
   DynamicTypeInterface>;

TYPED_TEST_SUITE(ConvertWithTypeInterface, TypeInterfaces);

TYPED_TEST(ConvertWithTypeInterface, convertInvalidToNullTypeInterfaceYieldsInvalid)
{
  auto result = qi::AnyValue{}.convert(static_cast<typename TestFixture::TypeInterface*>(nullptr));
  EXPECT_FALSE(result->isValid());
  EXPECT_FALSE(result.ownsReference());
}

template <typename T>
struct ConvertWithTypes: ::testing::Test {
  using Type = T;
};

#define QI_INTERNAL_NON_ANYREFERENCE_TYPES \
  int,                                     \
  float,                                   \
  double,                                  \
  std::string,                             \
  std::vector<int>,                        \
  std::map<int, int>,                      \
  Foo,                                     \
  Foo*,                                    \
  void*,                                   \
  qi::AnyObject,                           \
  boost::optional<int>,                    \
  boost::optional<double>,                 \
  boost::optional<std::string>,            \
  boost::optional<std::vector<int>>,       \
  boost::optional<std::map<int, int>>,     \
  boost::optional<qi::Buffer>,             \
  boost::optional<Foo>
using NonAnyReferenceTypes = testing::Types<QI_INTERNAL_NON_ANYREFERENCE_TYPES>;
using Types = testing::Types<QI_INTERNAL_NON_ANYREFERENCE_TYPES, qi::AnyValue>;
#undef QI_INTERNAL_NON_ANYREFERENCE_TYPES

TYPED_TEST_SUITE(ConvertWithTypes, Types);

TYPED_TEST(ConvertWithTypes, convertInvalidToOtherTypeInterfaceIsYieldsInvalid)
{
  auto result = qi::AnyValue{}.convert(qi::typeOf<typename TestFixture::Type>());
  EXPECT_FALSE(result->isValid());
  EXPECT_FALSE(result.ownsReference());
}

TYPED_TEST(ConvertWithTypes, convertInvalidAsReferenceToOtherTypeInterfaceYieldsInvalid)
{
  auto result = qi::AnyValue{}.asReference().convert(qi::typeOf<typename TestFixture::Type>());
  EXPECT_FALSE(result->isValid());
  EXPECT_FALSE(result.ownsReference());
}

TYPED_TEST(ConvertWithTypes, convertReferenceFromInvalidToOtherTypeInterfaceIsSafe)
{
  qi::AnyReference::from(qi::AnyValue{}).convert(qi::typeOf<typename TestFixture::Type>());
  // depending on the type, the result may be invalid or not, let's not test the result
}

template<typename O>
struct ValueOptional : ::testing::Test {};

struct StdOptionalTag {};
struct BoostOptionalTag {};

using OptionalTypes = ::testing::Types<StdOptionalTag, BoostOptionalTag>;
TYPED_TEST_SUITE(ValueOptional, OptionalTypes);

template<typename T>
auto make(StdOptionalTag, T t) -> std::optional<T> { return {std::move(t)}; }

template<typename T>
auto make(StdOptionalTag) -> std::optional<T> { return {}; }

template<typename T>
auto make(BoostOptionalTag, T t) -> boost::optional<T> { return {std::move(t)}; }

template<typename T>
auto make(BoostOptionalTag) -> boost::optional<T> { return {}; }

TYPED_TEST(ValueOptional, SetResetState)
{
  auto optTag = TypeParam{};
  AnyValue v{ make<int>(optTag) };
  EXPECT_FALSE(v.optionalHasValue());
  v.set(make(optTag, 42));
  EXPECT_TRUE(v.optionalHasValue());
  v.resetOptional();
  EXPECT_FALSE(v.optionalHasValue());
}

TYPED_TEST(ValueOptional, SetInvalidTypeToUnsetOptionalDoesNotSetIt)
{
  auto optTag = TypeParam{};
  AnyValue v{ make<int>(optTag) };
  EXPECT_FALSE(v.optionalHasValue());
  EXPECT_THROW(v.set(make(optTag, std::string{"sarah connor ?"})), std::exception);
  EXPECT_FALSE(v.optionalHasValue());
}

TYPED_TEST(ValueOptional, Int)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, 42) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(42, v.content().toInt());

  // different type of integer is allowed
  v.set(make(optTag, static_cast<unsigned short>(4323)));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(4323, v.content().toInt());

  // as long as it fits
  EXPECT_THROW(v.set(0x100000000), std::exception);

  // conversion from double is allowed
  v.set(make(optTag, 33.2));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(33, v.content().toInt()); // result is rounded

  // optional of same type is allowed
  v.set(make(optTag, 51));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(51, v.content().toInt());

  // conversion from a totally different type is not allowed
  EXPECT_THROW(v.set(make(optTag, std::string{"foo"})), std::exception);

  // it did not affect the value
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(51, v.content().toInt());
}

TYPED_TEST(ValueOptional, Double)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, 3.14) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_DOUBLE_EQ(3.14, v.content().toDouble());

  // conversion from float is allowed
  v.set(make(optTag, 8.94f));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_DOUBLE_EQ(static_cast<double>(8.94f), v.content().toDouble());

  // conversion from int is allowed
  v.set(make(optTag, 67));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_DOUBLE_EQ(67., v.content().toDouble());

  // conversion from a totally different type is not allowed
  EXPECT_THROW(v.set(make(optTag, std::string{"cupcakes"})), std::exception);

  // it did not affect the value
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_DOUBLE_EQ(67., v.content().toDouble());
}

TYPED_TEST(ValueOptional, Bool)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, true) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_TRUE(v.content().to<bool>());

  v.set(make(optTag, false));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_FALSE(v.content().to<bool>());

  // optional with a different type of int is allowed as long as it's 0 or 1.
  EXPECT_NO_THROW(v.set(make(optTag, 0)));
  EXPECT_NO_THROW(v.set(make(optTag, 1)));
  EXPECT_THROW(v.set(make(optTag, 1337)), std::exception);

  // conversion from a float is technically allowed as long as it's (close to) 0 or 1.
  EXPECT_NO_THROW(v.set(make(optTag, 1.f)));
  EXPECT_NO_THROW(v.set(make(optTag, 0.f)));
  EXPECT_THROW(v.set(make(optTag, 3.14f)), std::exception);

  // conversion from a non numeric type is not allowed
  EXPECT_THROW(v.set(make(optTag, std::string{"cookies"})), std::exception);

  // it did not affect the value
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_FALSE(v.content().to<bool>());
}

TYPED_TEST(ValueOptional, String)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, std::string{"foo"}) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ("foo", v.content().toString());

  v.set(make(optTag, std::string{"lolcats"}));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ("lolcats", v.content().toString());

  // conversion from a totally different type is not allowed
  EXPECT_THROW(v.set(make(optTag, 67)), std::exception);
  EXPECT_THROW(v.set(make(optTag, 33.2)), std::exception);

  // it did not affect the value
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ("lolcats", v.content().toString());
}

TYPED_TEST(ValueOptional, List)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, std::vector<int>{{1, 1, 2, 3, 5}}) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(5u, v.content().size());
  EXPECT_EQ(3, v.content().element<int>(3));

  v.content().append(8);
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(6u, v.content().size());
  EXPECT_EQ(8, v.content().element<int>(5));

  // conversion from a totally different type is not allowed
  EXPECT_THROW(v.set(make(optTag, 67)), std::exception);
  EXPECT_THROW(v.set(make(optTag, 33.2)), std::exception);
  EXPECT_THROW(v.set(make(optTag, std::string{"muffins"})), std::exception);

  // it did not affect the value
  ASSERT_TRUE(v.optionalHasValue());

  // can be converted to list
  const std::vector<int> expected{1, 1, 2, 3, 5, 8};
  EXPECT_EQ(v.content().toList<int>(), expected);
}

TYPED_TEST(ValueOptional, Map)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, std::map<int, std::string>{
      { { 1, "one" }, { 3, "three" }, { 42, "the answer" } } }) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(3u, v.content().size());
  EXPECT_EQ("the answer", v.content().element<std::string>(42));

  v.content().insert(8, "31ght");
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(4u, v.content().size());
  EXPECT_EQ("31ght", v.content().element<std::string>(8));

  // conversion from a totally different type is not allowed
  EXPECT_THROW(v.set(make(optTag, 67)), std::exception);
  EXPECT_THROW(v.set(make(optTag, 33.2)), std::exception);
  EXPECT_THROW(v.set(make(optTag, std::string{"donuts"})), std::exception);

  // it did not affect the value
  ASSERT_TRUE(v.optionalHasValue());

  // can be converted to map
  const auto convertedSize = v.content().toMap<int, std::string>().size();
  EXPECT_EQ(4u, convertedSize);
}

TYPED_TEST(ValueOptional, Tuple)
{
  auto optTag = TypeParam{};
  AnyValue v{ make(optTag, Foo()) };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(2u, v.content().size());
  EXPECT_EQ(2u, v.content().membersType().size());

  AnyReferenceVector refVec;
  std::string str("cornflakes");
  refVec.push_back(AutoAnyReference(str)); // has to explicitly be a std::string, a string literal won't do
  double dbl = 6.9847;
  refVec.push_back(AutoAnyReference(dbl));
  v.content().setTuple(refVec);
  EXPECT_EQ(str, v.content().element<std::string>(0));

  Foo foo = v.content().to<Foo>();
  EXPECT_DOUBLE_EQ(6.9847, foo.dbl);
  EXPECT_EQ(str, foo.str);
}

TYPED_TEST(ValueOptional, ToOptional)
{
  auto optTag = TypeParam{};
  auto opt = make(optTag, std::string{"cupcakes"});
  AnyValue v{ opt };
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ("cupcakes", *v.toOptional<std::string>());
  EXPECT_EQ("cupcakes", *v.to<decltype(opt)>());
}

TYPED_TEST(ValueOptional, RawBuffer)
{
  auto optTag = TypeParam{};
  const std::string data = "kikoolol";
  qi::Buffer buffer;
  buffer.write(data.c_str(), data.size() + 1);

  AnyValue v{ make(optTag, buffer) };
  ASSERT_TRUE(v.optionalHasValue());

  auto readBuffer = v.toOptional<qi::Buffer>();
  ASSERT_TRUE(readBuffer);
  EXPECT_EQ(*readBuffer, buffer);
}

TYPED_TEST(ValueOptional, AnyValue)
{
  auto optTag = TypeParam{};
  AnyValue v{ make<AnyValue>(optTag) };

  ASSERT_NO_THROW(v.set(make(optTag, AnyValue{ "marshmallows" })));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ("marshmallows", v.toOptional<AnyValue>()->to<std::string>());
  EXPECT_EQ("marshmallows", v.toOptional<std::string>().value());

  ASSERT_NO_THROW(v.set(make(optTag, AnyValue{ 42329 })));
  ASSERT_TRUE(v.optionalHasValue());
  EXPECT_EQ(42329, v.toOptional<AnyValue>()->to<int>());
  EXPECT_EQ(42329, v.toOptional<int>().value());
  EXPECT_ANY_THROW(v.toOptional<std::string>());
}

TEST(Value, NullPtrThrowsExceptionWhenDereferenced)
{
  int* p = nullptr;
  auto v = AnyValue::from(p);
  EXPECT_THROW(*v, NullPtrException);
  EXPECT_THROW(v.content(), NullPtrException);
}

class TypeParameterizedAutoAnyReference : public ::testing::TestWithParam<TypeInterface*> {};
INSTANTIATE_TEST_SUITE_P(
    MostCommonInterfaces,
    TypeParameterizedAutoAnyReference,
    ::testing::Values(
      makeTypeOfKind(TypeKind_Void),
      makeTypeOfKind(TypeKind_Int),
      makeTypeOfKind(TypeKind_Float),
      makeTypeOfKind(TypeKind_String),
      makeTypeOfKind(TypeKind_Object),
      typeOf<AnyValue>(),
      makeListType(typeOf<AnyValue>()),
      makeMapType(typeOf<AnyValue>(), typeOf<AnyValue>()),
      makeTupleType({ typeOf<AnyValue>() }),
      makeOptionalType(typeOf<AnyValue>())
   )
);

TEST_P(TypeParameterizedAutoAnyReference, AutoAnyReferenceFromAnyReferenceSharesItsType)
{
  const auto itf = GetParam();

  { // From AnyReference
    auto ref = ka::scoped(AnyReference{ itf }, [](auto ref){ ref.destroy(); });
    AutoAnyReference autoRef(ref.value);
    EXPECT_EQ(itf->kind(), autoRef.kind());
  }
  { // From AnyValue
    AnyValue value{ itf };
    AutoAnyReference autoRef(value);
    EXPECT_EQ(itf->kind(), autoRef.kind());
  }
  { // From AutoAnyReference
    auto ref = ka::scoped(AnyReference{ itf }, [](auto ref){ ref.destroy(); });
    AutoAnyReference autoRef(ref.value);
    AutoAnyReference autoAutoRef(autoRef);
    EXPECT_EQ(itf->kind(), autoAutoRef.kind());
  }
}

template<typename T>
class TypedAutoAnyReference : public ::testing::Test {};
TYPED_TEST_SUITE(TypedAutoAnyReference, NonAnyReferenceTypes);

TYPED_TEST(TypedAutoAnyReference, AutoAnyReferenceFromValueSharesItsType)
{
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4068, pragmas)
KA_WARNING_DISABLE(, missing-field-initializers)
  AutoAnyReference autoRef(TypeParam{});
KA_WARNING_POP()
  EXPECT_EQ(typeOf<TypeParam>()->kind(), autoRef.kind());
}
