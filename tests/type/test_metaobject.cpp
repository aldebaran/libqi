#include <gtest/gtest.h>

#include "test_object.hpp"

#include <qi/anyobject.hpp>
#include <qi/jsoncodec.hpp>
#include <qi/type/metaobject.hpp>
#include <ka/conceptpredicate.hpp>
#include <src/type/metaobject_p.hpp>

bool compareMetaMethodParameter(
    const qi::MetaMethodParameter& lhs,
    const qi::MetaMethodParameter& rhs)
{
  return lhs.name() == rhs.name() && lhs.description() == rhs.description();
}

const qi::Signature returnSignature{"_"};
const std::string name{"plop"};
const qi::Signature parametersSignature{"()"};
const std::string description{"let us plop!"};
const qi::MetaMethodParameterVector parameters;
const std::string returnDescription;

qi::MetaMethod makeMetaMethod()
{
  return qi::MetaMethod{42, returnSignature, name, parametersSignature, description, parameters, returnDescription};
}

qi::MetaMethodBuilder makeMetaMethodBuilder()
{
  qi::MetaMethodBuilder mmb;
  mmb.setName(name);
  mmb.setDescription(description);
  mmb.setParametersSignature(parametersSignature);
  mmb.setReturnSignature(returnSignature);
  mmb.setReturnDescription(returnDescription);
  return mmb;
}

TEST(MetaMethod, isDefaultConstructible)
{
  ASSERT_TRUE(std::is_default_constructible<qi::MetaMethod>{}.value);
  qi::MetaMethod mm;
  QI_IGNORE_UNUSED(mm);
}

TEST(MetaMethod, constructVoidMethod)
{ // Do not use makeMetaMethod, since we are testing the construction itself
  qi::MetaMethod mm{42, returnSignature, name, parametersSignature, description, parameters, returnDescription};
  EXPECT_EQ(name, mm.name());
  EXPECT_EQ(parametersSignature, mm.parametersSignature());
  EXPECT_EQ(description, mm.description());
  EXPECT_TRUE(std::equal(parameters.begin(), parameters.end(),
                         mm.parameters().begin(), compareMetaMethodParameter));
  EXPECT_EQ(returnDescription, mm.returnDescription());
}

TEST(MetaMethod, toFromJSON)
{
  auto mm = makeMetaMethod();
  auto mmJSON = qi::encodeJSON(mm);
  auto reconstructedMm = qi::decodeJSON(mmJSON).to<qi::MetaMethod>();
  EXPECT_EQ(mm.name(), reconstructedMm.name());
  EXPECT_EQ(mm.parametersSignature(), reconstructedMm.parametersSignature());
  EXPECT_EQ(mm.description(), reconstructedMm.description());

  const auto parameters = mm.parameters();
  EXPECT_TRUE(std::equal(parameters.begin(), parameters.end(),
    reconstructedMm.parameters().begin(), compareMetaMethodParameter));

  EXPECT_EQ(mm.returnDescription(), reconstructedMm.returnDescription());
}

TEST(MetaMethod, fromJSON)
{
  auto mmJSON =
      "{"
      "  \"uid\": 42,"
      "  \"returnSignature\":\"_\","
      "  \"name\": \"plop\","
      "  \"parametersSignature\": \"()\","
      "  \"description\": \"let us plop!\","
      "  \"parameters\": [],"
      "  \"returnDescription\":\"\""
      "}";
  auto mm = qi::decodeJSON(mmJSON).to<qi::MetaMethod>();
  EXPECT_EQ(name, mm.name());
  EXPECT_EQ(parametersSignature, mm.parametersSignature());
  EXPECT_EQ(description, mm.description());
  EXPECT_TRUE(std::equal(parameters.begin(), parameters.end(),
                         mm.parameters().begin(), compareMetaMethodParameter));
  EXPECT_EQ(returnDescription, mm.returnDescription());
}

TEST(MetaObject, constructSimpleMetaObject)
{
  qi::MetaObjectBuilder mob;
  auto mmb = makeMetaMethodBuilder();
  mob.addMethod(mmb, 42);

  auto mo = mob.metaObject();
  auto& mm = *mo.method(42);

  EXPECT_EQ(name, mm.name());
  EXPECT_EQ(parametersSignature, mm.parametersSignature());
  EXPECT_EQ(description, mm.description());
  EXPECT_TRUE(std::equal(parameters.begin(), parameters.end(),
                         mm.parameters().begin(), compareMetaMethodParameter));
  EXPECT_EQ(returnDescription, mm.returnDescription());
}

// MetaObject carry a map<int, MetaMethod>, therefore it cannot be serialized
// to JSON, because JSON does not support non-string keys in maps!


TEST(MetaObject, findMethod)
{
  qi::MetaObjectBuilder b;
  const unsigned int f   = b.addMethod("i", "f", "(i)").id;
  const unsigned int g1  = b.addMethod("i", "g", "(i)").id;
  const unsigned int g2  = b.addMethod("i", "g", "(ii)").id;
  const unsigned int h1i = b.addMethod("i", "h", "(i)").id;
  const unsigned int h1s = b.addMethod("i", "h", "(s)").id;
  const unsigned int h2  = b.addMethod("i", "h", "(ii)").id;

  qi::MetaObject mo = b.metaObject();
  bool canCache = false;
  int mid = mo.findMethod("f", args(1), &canCache);
  EXPECT_EQ(mid, (int)f); EXPECT_TRUE(canCache);
  mid = mo.findMethod("g", args(1), &canCache);
  EXPECT_EQ(mid, (int)g1); EXPECT_TRUE(canCache);
  mid = mo.findMethod("g", args(1, 1), &canCache);
  EXPECT_EQ(mid, (int)g2); EXPECT_TRUE(canCache);
  // no garantee is made on result of findmethod(g, "foo"), so not tested
  mid = mo.findMethod("h", args(1), &canCache);
  EXPECT_EQ(mid, (int)h1i); EXPECT_FALSE(canCache);
  mid = mo.findMethod("h", args("foo"), &canCache);
  EXPECT_EQ(mid, (int)h1s); EXPECT_FALSE(canCache);
  mid = mo.findMethod("h", args(1, 1), &canCache);
  EXPECT_EQ(mid, (int)h2); EXPECT_TRUE(canCache);

  mid = mo.findMethod("h::(i)", args(1), &canCache);
  EXPECT_EQ(mid, (int)h1i); EXPECT_TRUE(canCache);

  // check null canCache
  mo.findMethod("h::(i)", args(1), 0);
  mid = mo.findMethod("h", args("foo"), 0);
  EXPECT_TRUE(true);
}

TEST(MetaObject, defaultConstructedMosAreEqual)
{
  qi::MetaObject mo1;
  qi::MetaObject mo2;
  EXPECT_FALSE(mo1 < mo2);
  EXPECT_FALSE(mo2 < mo1);
}

TEST(MetaObject, copiedMosAreEqual)
{
  qi::MetaObjectBuilder b;
  b.addMethod("i", "f", "(i)");

  qi::MetaObject mo1 = b.metaObject();
  qi::MetaObject mo2 = mo1;

  EXPECT_FALSE(mo1 < mo2);
  EXPECT_FALSE(mo2 < mo1);
}

TEST(MetaObject, independentMosAreDifferent)
{
  qi::MetaObjectBuilder b1;
  b1.addMethod("i", "f", "(i)");
  b1.setDescription("first mo");

  qi::MetaObject mo1 = b1.metaObject();

  qi::MetaObjectBuilder b2;
  b2.addMethod("i", "f", "(i)");
  b2.setDescription("second mo");

  qi::MetaObject mo2 = b2.metaObject();

  EXPECT_TRUE(mo1 < mo2 || mo2 < mo1);
}

TEST(MetaObject, independentMosWithTheSameContentAreEqual)
{
  qi::MetaObjectBuilder b1;
  b1.addMethod("i", "f", "(i)");
  b1.setDescription("my_mo");

  qi::MetaObject mo1 = b1.metaObject();

  qi::MetaObjectBuilder b2;
  b2.addMethod("i", "f", "(i)");
  b2.setDescription("my_mo");

  qi::MetaObject mo2 = b2.metaObject();

  EXPECT_FALSE(mo1 < mo2);
  EXPECT_FALSE(mo2 < mo1);
}
