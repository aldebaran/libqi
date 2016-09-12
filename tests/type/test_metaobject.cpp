#include <gtest/gtest.h>
#include <qi/anyobject.hpp>
#include <qi/jsoncodec.hpp>

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
  QI_UNUSED(mm);
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
  EXPECT_TRUE(std::equal(mm.parameters().begin(), mm.parameters().end(),
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
