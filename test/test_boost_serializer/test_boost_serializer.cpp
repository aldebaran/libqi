
#include "test_boost_serializer.hxx"

using namespace AL::Serialization;

// -- basic types

TEST(SerializationTest, Bool)
{
  bool arg = true;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, Int)
{
  int arg = 1;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, NegativeInt)
{
  int arg = -1;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, Float)
{
  float arg = 34.4598495f;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, NegativeFloat)
{
  float arg = -34.4598495f;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, Double)
{
  double arg = 34.4598495;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, NegativeDouble)
{
  double arg = -34.4598495;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, Short)
{
  short arg = 12;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, Long)
{
  long arg = (long)12234234234234;
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, String)
{
  std::string arg = "Hello World.";
  testSerializationDeserialization(arg);
}

// ---- exotic types
//
TEST(SerializationTest, StdPair)
{
  std::pair<int, std::string> arg = std::make_pair<int, std::string>(1, "hello");
  testSerializationDeserialization(arg);
}

TEST(SerializationTest, StdMap)
{
  std::map<int, std::string> arg;
  arg.insert(std::make_pair<int, std::string>(1, "hello"));
  arg.insert(std::make_pair<int, std::string>(2, "hello2"));
  testSerializationDeserialization(arg);
}


// ---- vectors

TEST(SerializationTest, VectorBool)
{
  std::vector<bool> arg;
  arg.assign(26, true);
  testSerializationDeserializationList(arg);
}

TEST(SerializationTest, VectorFloat)
{
  std::vector<float> arg;
  arg.assign(26, 0.653f);
  testSerializationDeserializationList(arg);
}

TEST(SerializationTest, VectorInt)
{
  std::vector<int> arg;
  arg.assign(26, 42);
  testSerializationDeserializationList(arg);
}

TEST(SerializationTest, VectorString)
{
  std::string val = "Hello World.";
  std::vector<std::string> arg;
  arg.assign(26, val);
  testSerializationDeserializationList(arg);
}

// -- lists
//
//TEST(SerializationTest, ListBool)
//{
//  std::list<bool> arg;
//  arg.assign(26, true);
//
//  testSerializationDeserialization(arg);
//}
//
//TEST(SerializationTest, ListFloat)
//{
//  std::list<float> arg;
//  arg.assign(26, 0.653f);
//  testSerializationDeserialization(arg);
//}
//
//TEST(SerializationTest, ListInt)
//{
//  std::list<int> arg;
//  arg.assign(26, 42);
//  testSerializationDeserialization(arg);
//}
//
//TEST(SerializationTest, ListString)
//{
//  std::string val = "Hello World.";
//  std::list<std::string> arg;
//  arg.assign(26, val);
//  testSerializationDeserialization(arg);
//}

#include <alcommon-ng/serialization/call_definition.hpp>
TEST(SerializationTest, CallDefinition)
{
  AL::Messaging::CallDefinition arg;
  testSerializationDeserialization(arg);
}

#include <alcommon-ng/serialization/result_definition.hpp>
TEST(SerializationTest, ResultDefinition)
{
  AL::Messaging::ResultDefinition arg;
  testSerializationDeserialization(arg);
}


TEST(SerializationPerformance, stringBuffers_BINARY) {
  std::cout << "BINARY " << numMessages << std::endl;
  testSerialization_StringBufferSizes(BOOST_BINARY, numMessages);
  testDeSerialization_StringBufferSizes(BOOST_BINARY, numMessages);
}

TEST(SerializationPerformance, DISABLED_stringBuffers_TEXT) {
  std::cout << "TEXT " << numMessages << std::endl;
  testSerialization_StringBufferSizes(BOOST_TEXT, numMessages);
  testDeSerialization_StringBufferSizes(BOOST_TEXT, numMessages);
}

TEST(SerializationPerformance, DISABLED_stringBuffers_XML) {
  std::cout << "XML " << numMessages << std::endl;
  testSerialization_StringBufferSizes(BOOST_XML, numMessages);
  testDeSerialization_StringBufferSizes(BOOST_XML, numMessages);
}

TEST(SerializationPerformance, float_BINARY) {
  float f = 1.06788f;
  testSerializationDeserializationPerf(f, BOOST_BINARY);
}

TEST(SerializationPerformance, double_BINARY) {
  double d = 34.4598495;
  testSerializationDeserializationPerf(d, BOOST_BINARY);
}

TEST(SerializationPerformance, short_BINARY) {
  short s = 12;
  testSerializationDeserializationPerf(s, BOOST_BINARY);
}

TEST(SerializationPerformance, int_BINARY) {
  int i = 198032;
  testSerializationDeserializationPerf(i, BOOST_BINARY);
}

TEST(SerializationPerformance, string_BINARY) {
  std::string str = "Hello World.";
  testSerializationDeserializationPerf(str, BOOST_BINARY);
}

TEST(SerializationPerformance, pair_BINARY) {
  std::pair<int, std::string> pair = std::make_pair<int, std::string>(1, "hello");
  testSerializationDeserializationPerf(pair, BOOST_BINARY);
}

TEST(SerializationPerformance, map_BINARY) {
  std::map<int, std::string> map;
  map.insert(std::make_pair<int, std::string>(1, "hello"));
  map.insert(std::make_pair<int, std::string>(2, "hello2"));
  testSerializationDeserializationPerf(map, BOOST_BINARY);
}

TEST(SerializationPerformance, CallDefinition_BINARY) {
  AL::Messaging::CallDefinition calldef;
  calldef.push(1.0f);
  calldef.push(std::string("hello1"));
  testSerializationDeserializationPerf(calldef, BOOST_BINARY);
}

TEST(SerializationPerformance, ResultDefinition_BINARY) {
  AL::Messaging::VariableValue v("result");
  AL::Messaging::ResultDefinition resultdef(1,v);
  testSerializationDeserializationPerf(resultdef, BOOST_BINARY);
}




// to keep gtest happy
template<typename T, typename U>
inline std::ostream & operator << (std::ostream & ostr, const std::pair<T, U> & value) {
  ostr << "pair";
        return ostr;
}

// to keep gtest happy
template<typename T, typename U>
inline std::ostream & operator << (std::ostream & ostr, const std::map<T, U> & value) {
  ostr << "map";
        return ostr;
}

// to keep gtest happy
inline std::ostream & operator << (std::ostream & ostr, const AL::Messaging::CallDefinition & value) {
  ostr << "CallDefinition";
        return ostr;
}

// to keep gtest happy
inline std::ostream & operator << (std::ostream & ostr, const AL::Messaging::ResultDefinition & value) {
  ostr << "ResultDefinition";
        return ostr;
}
