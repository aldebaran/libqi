
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

// ---- increasing string buffer sizes

TEST(SerializationPerformance, CallDefinitionBuffers) {
  std::cout << "BINARY " << numMessages << std::endl;
  testSerialization_CallDefBufferSizes(numMessages);
  testDeSerialization_CallDefBufferSizes(numMessages);
}

TEST(SerializationPerformance, VariableValueBuffers) {
  std::cout << "BINARY " << numMessages << std::endl;
  testSerialization_VariableValueBufferSizes(numMessages);
  testDeSerialization_VariableValueBufferSizes(numMessages);
}

TEST(SerializationPerformance, ValueTypeBuffers) {
  std::cout << "BINARY " << numMessages << std::endl;
  testSerialization_ValueTypeBufferSizes(numMessages);
  testDeSerialization_ValueTypeBufferSizes(numMessages);
}

TEST(SerializationPerformance, StringBuffers) {
  std::cout << "BINARY " << numMessages << std::endl;
  testSerialization_StringBufferSizes(numMessages);
  testDeSerialization_StringBufferSizes(numMessages);
}


TEST(SerializationPerformance, CallDefinition) {
  AL::Messaging::CallDefinition calldef;
  calldef.push(1.0f);
  calldef.push(std::string("hello1"));
  testSerializationDeserializationPerf(calldef);
}

TEST(SerializationPerformance, ResultDefinition) {
  AL::Messaging::VariableValue v("result");
  AL::Messaging::ResultDefinition resultdef(1,v);
  testSerializationDeserializationPerf(resultdef);
}


// Simple types test one by one

TEST(SerializationPerformance, float_) {
  float f = 1.06788f;
  testSerializationDeserializationPerf(f);
}

TEST(SerializationPerformance, double_) {
  double d = 34.4598495;
  testSerializationDeserializationPerf(d);
}

TEST(SerializationPerformance, short_) {
  short s = 12;
  testSerializationDeserializationPerf(s);
}

TEST(SerializationPerformance, int_) {
  int i = 198032;
  testSerializationDeserializationPerf(i);
}

TEST(SerializationPerformance, string_) {
  std::string str = "Hello World.";
  testSerializationDeserializationPerf(str);
}

TEST(SerializationPerformance, pair_) {
  std::pair<int, std::string> pair = std::make_pair<int, std::string>(1, "hello");
  testSerializationDeserializationPerf(pair);
}

TEST(SerializationPerformance, map_) {
  std::map<int, std::string> map;
  map.insert(std::make_pair<int, std::string>(1, "hello"));
  map.insert(std::make_pair<int, std::string>(2, "hello2"));
  testSerializationDeserializationPerf(map);
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
