/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/jsoncodec.hpp>

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

TEST(Value, Basic)
{
  AnyReference v;
  int twelve = 12;
  v = AutoAnyReference(twelve);
  ASSERT_TRUE(v.type() != 0);
  ASSERT_TRUE(v.rawValue() != 0);
  ASSERT_EQ(v.toInt(), 12);
  ASSERT_EQ(v.toFloat(), 12.0f);
  ASSERT_EQ(v.toDouble(), 12.0);
  double five = 5.0;
  v = AutoAnyReference(five);
  ASSERT_EQ(v.toInt(), 5);
  ASSERT_EQ(v.toFloat(), 5.0f);
  ASSERT_EQ(v.toDouble(), 5.0);
  v = AutoAnyReference("foo");
  ASSERT_EQ("foo", v.toString());
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
struct Point
{
  int x,y;
  bool operator ==(const Point& b) const { return x==b.x && y == b.y;}
};

QI_TYPE_STRUCT(TStruct, d, s);
QI_TYPE_STRUCT(Point, x, y);

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
  Point p;
  p.x = 1;
  p.y = 2;
  ASSERT_TRUE(p == p);
  ASSERT_EQ(p , gtuple.to<Point>());
  p.x = 3;
  gtuple[0].setDouble(gtuple[0].toDouble() + 2);
  ASSERT_EQ(p, gtuple.to<Point>());
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

  std::pair<qi::AnyReference, bool> res1 = gv1.convert(type);
  std::pair<qi::AnyReference, bool> res2 = gv2.convert(type);

  ASSERT_FALSE(res2.first.type());
  ASSERT_TRUE(res1.first.type() != 0);
  ASSERT_EQ(res1.first.type()->info(), type->info());
  ASSERT_EQ(gv1.size(), res1.first.size());
  ASSERT_STREQ("b", res1.first[3][1].asString().c_str());

  qi::TypeInterface *dest3 = qi::TypeInterface::fromSignature("(fffI)");
  qi::AnyValue gv3 = qi::decodeJSON("[1.1, 2.2, 3.3, \"42\"]");
  std::pair<qi::AnyReference, bool> res3 = gv3.convert(dest3);
  ASSERT_FALSE(res3.first.type());
}

TEST(Value, Convert_ListToMap)
{
  qi::TypeInterface *type1= qi::TypeInterface::fromSignature("{if}");
  qi::AnyValue gv1 = qi::decodeJSON("[[10.10, 42.42], [20, 43], [30, 44.44]]");
  std::pair<qi::AnyReference, bool> res1 = gv1.convert(type1);
  ASSERT_TRUE(res1.first.type() != 0);
  ASSERT_EQ(res1.first.type()->info(), type1->info());
  ASSERT_EQ(gv1.size(), res1.first.size());
  ASSERT_EQ(44.44f, res1.first[30].asFloat());

  qi::TypeInterface *type2 = qi::TypeInterface::fromSignature("{if}");
  qi::AnyValue gv2 = qi::decodeJSON("[[10.10, 42.42], [20, 43], [\"plop\", 44.44]]");
  std::pair<qi::AnyReference, bool> res2 = gv2.convert(type2);
  ASSERT_FALSE(res2.first.type());
}


struct Foo
{
  std::string str;
  double dbl;
};

struct Oof
{
  double dbl;
  std::string str;
};

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
};
QI_TYPE_STRUCT_REGISTER(FooBase, x, y);
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
  int z;
};

QI_TYPE_STRUCT_EXTENSION_DROP_FIELDS(FooEx, "z");
QI_TYPE_STRUCT_REGISTER(FooEx, x, y, z);


TEST(Struct, ExtendDrop)
{
  FooEx e; e.x = 1; e.y = 2; e.z = 3;
  FooBase f = qi::AnyReference::from(e).to<FooBase>();
  EXPECT_EQ(1, f.x);
  EXPECT_EQ(2, f.y);
  EXPECT_ANY_THROW(qi::AnyReference::from(e).to<OtherBase>());
  EXPECT_ANY_THROW(qi::AnyReference::from(f).to<FooEx>());
}

struct FooEx2
{
  int x;
  int y;
  int z;
  std::string s;
};


QI_TYPE_STRUCT_EXTENSION_DROP_FIELDS(FooEx2, "z", "s");
QI_TYPE_STRUCT_EXTENSION_FILL_FIELDS(FooEx2, "z", "s");
QI_TYPE_STRUCT_REGISTER(FooEx2, x, y, z, s);

TEST(Struct, ExtendFill)
{
  FooBase f; f.x = 1; f.y = 2;
  FooEx2 f2 = qi::AnyReference::from(f).to<FooEx2>();
  EXPECT_EQ(0, f2.z);
  EXPECT_TRUE(f2.s.empty());
  FooEx f1; f1.x = 1; f1.y = 2; f1.z = 3;
  f2 = qi::AnyReference::from(f1).to<FooEx2>();
  EXPECT_EQ(3, f2.z);
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
  FooBase f; f.x = 1; f.y = 2;
  FooExPub f2 = qi::AnyReference::from(f).to<FooExPub>();
  EXPECT_EQ(0, f2.fooex2.z);
  EXPECT_TRUE(f2.fooex2.s.empty());
  FooEx f1; f1.x = 1; f1.y = 2; f1.z = 3;
  f2 = qi::AnyReference::from(f1).to<FooExPub>();
  EXPECT_EQ(3, f2.fooex2.z);
  EXPECT_TRUE(f2.fooex2.s.empty());
}

// A good demo of why all mode is overkill
struct Velo
{
  int x;
  std::string y;
};
struct Poisson
{
  double z;
  std::vector<int> a;
};
QI_TYPE_STRUCT_EXTENSION_ALL(Velo);
QI_TYPE_STRUCT_EXTENSION_ALL(Poisson);
QI_TYPE_STRUCT_REGISTER(Velo, x, y);
QI_TYPE_STRUCT_REGISTER(Poisson, z, a);


TEST(Struct, ExtendAll)
{
  Velo v; Poisson p;
  p = qi::AnyReference::from(v).to<Poisson>();
  v = qi::AnyReference::from(p).to<Velo>();
  EXPECT_TRUE(true);
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
bool colorVersionHandler(ColorA* instance, const std::vector<std::string>& fields)
{
  if (fields.size() != 1 || fields.front() != "a")
    return false;
  return instance->a == 1;
}

QI_TYPE_STRUCT_REGISTER(Color, r, g, b);
QI_TYPE_STRUCT_EXTENSION_DROP_HANDLER(ColorA, colorVersionHandler);
QI_TYPE_STRUCT_REGISTER(ColorA, r, g, b, a);

TEST(Struct, DropHandler)
{
  ColorA a0, a1;
  Color c;
  a0.r = 0; a0.g = 1; a0.b = 2; a0.a = 0;
  a1.r = 0; a1.g = 1; a1.b = 2; a1.a = 1;
  c = qi::AnyReference::from(a1).to<Color>();
  EXPECT_EQ(c.r, a1.r);
  EXPECT_EQ(c.g, a1.g);
  EXPECT_EQ(c.b, a1.b);
  EXPECT_ANY_THROW(c = qi::AnyReference::from(a0).to<Color>());
}

/* Say we upgraded struct Face with a customizable normal
*/
struct V3
{
  int x,y,z; // use int for exact comparison
};
V3 normal(const V3& a, const V3& b)
{
  V3 res;
  res.x = a.y*b.z - a.z*b.y;
  res.y = a.x*b.z - a.z*b.x;
  res.z = a.x*b.y - a.y*b.x;
  return res;
}
V3 diff(const V3& a, const V3& b)
{
  V3 res;
  res.x = a.x-b.x;
  res.y = a.y-b.y;
  res.z = a.z-b.z;
  return res;
}

QI_TYPE_STRUCT_REGISTER(V3, x, y, z);
struct Face1
{
  V3 a, b, c;
};
QI_TYPE_STRUCT_REGISTER(Face1, a, b, c);

struct Face2
{
  V3 a,b,c,normal;
};

bool fillNormal(std::map<std::string, qi::AnyValue>& map, const std::vector<std::string>& fields)
{
  if (fields.size() != 1 || fields.front() != "normal")
    return false;
  V3 a = map["a"].to<V3>();
  V3 b = map["b"].to<V3>();
  V3 c = map["c"].to<V3>();
  map["normal"] = qi::AnyValue(normal(diff(a, b), diff(a, c)));
  return true;
}

QI_TYPE_STRUCT_EXTENSION_DROP_FIELDS(Face2, "normal");
QI_TYPE_STRUCT_EXTENSION_FILL_HANDLER(Face2, fillNormal);
QI_TYPE_STRUCT_REGISTER(Face2, a, b, c, normal);

TEST(Struct, FillHandler)
{
  Face2 f2;
  Face1 f1;
  EXPECT_NO_THROW(f1 = qi::AnyReference::from(f2).to<Face1>());
  memset(&f2, 0xcc, sizeof(Face2));
  memset(&f1, 0, sizeof(Face1));
  f1.b.x = 1;
  f1.c.y = 1;
  EXPECT_NO_THROW(f2 = qi::AnyReference::from(f1).to<Face2>());
  EXPECT_EQ(1, f2.normal.z);
}

TEST(Struct, ComplexType)
{
  std::pair<std::vector<V3>, std::list<V3> > p;
  AnyValue::from(p);
  std::pair<std::vector<int>, std::list<std::map<int, V3> > > p2;
  AnyValue::from(p2);
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
