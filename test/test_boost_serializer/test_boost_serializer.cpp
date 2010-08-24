
#include <gtest/gtest.h>
#include <alcommon-ng/serialization/boost_serializer.hpp>
#include <boost/timer.hpp>
#include <string>

using namespace AL::Serialization;

int numMessages = 100000;

// test three serializations for type T (not a list)
template<typename T>
void testSerializationDeserialization(const T& arg)
{
  std::string buf;
  buf = BoostSerializer::serialize(arg, BOOST_BINARY);
  T res1 = BoostSerializer::deserialize<T>(buf, BOOST_BINARY);
  EXPECT_EQ(arg, res1);

  buf = BoostSerializer::serialize(arg, BOOST_TEXT);
  T res2 = BoostSerializer::deserialize<T>(buf, BOOST_TEXT);
  EXPECT_EQ(arg, res2);

  buf = BoostSerializer::serialize(arg, BOOST_XML);
  T res3 = BoostSerializer::deserialize<T>(buf, BOOST_XML);
  EXPECT_EQ(arg, res3);

  buf = BoostSerializer::serialize(arg, BOOST_BINARY);
  boost::shared_ptr<T> res4 = BoostSerializer::deserializeToPtr<T>(buf);
  EXPECT_EQ(arg, *res4.get());
}


// test three serializations for type T (a list)
template<typename T>
void testSerializationDeserializationList(const T& arg)
{
  std::string buf;
  buf = BoostSerializer::serialize(arg, BOOST_BINARY);
  T res1 = BoostSerializer::deserialize<T>(buf, BOOST_BINARY);
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res1[i]);
  }

  buf = BoostSerializer::serialize(arg, BOOST_TEXT);
  T res2 = BoostSerializer::deserialize<T>(buf, BOOST_TEXT);
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res2[i]);
  }

  buf = BoostSerializer::serialize(arg, BOOST_XML);
  T res3 = BoostSerializer::deserialize<T>(buf, BOOST_XML);
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res3[i]);
  }

  buf = BoostSerializer::serialize(arg, BOOST_BINARY);
  boost::shared_ptr<T> res4 = BoostSerializer::deserializeToPtr<T>(buf);
  T res5 = *res4.get();
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res5[i]);
  }
}

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

void testSerialization_StringBufferSizes(SERIALIZATION_TYPE type, int numMessages) {

  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  // loop message sizes 2^i bytes
  for (unsigned int i=1; i < 12; i++) {

    char character = 'A';
    unsigned int numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string request = std::string(numBytes, character);

    boost::timer t;
    double elapsed;
    t.restart();

    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostSerializer::serialize(request, type);
    }

    elapsed = t.elapsed();
    float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
    float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
    std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
  }
}

void testDeSerialization_StringBufferSizes(SERIALIZATION_TYPE type, int numMessages) {

  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  // loop message sizes 2^i bytes
  for (unsigned int i=1; i < 12; i++) {

    char character = 'A';
    unsigned int numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string request = std::string(numBytes, character);

    boost::timer t;
    double elapsed;
    std::string buffer = BoostSerializer::serialize(request, type);

    t.restart();

    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostSerializer::deserialize<std::string>(buffer, type);
    }

    elapsed = t.elapsed();
    float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
    float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
    std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
  }
}

TEST(SerializationPerformance, binary) {
  std::cout << " BINARY Serialization " << numMessages << std::endl;
  testSerialization_StringBufferSizes(BOOST_BINARY, numMessages);
  std::cout << " BINARY DeSerialization " << numMessages << std::endl;
  testDeSerialization_StringBufferSizes(BOOST_BINARY, numMessages);
}

TEST(SerializationPerformance, DISABLED_text) {
  std::cout << " TEXT Serialization " << numMessages << std::endl;
  testSerialization_StringBufferSizes(BOOST_TEXT, numMessages);
  std::cout << " TEXT DeSerialization " << numMessages << std::endl;
  testDeSerialization_StringBufferSizes(BOOST_TEXT, numMessages);
}

TEST(SerializationPerformance, DISABLED_xml) {
  std::cout << " XML Serialization " << numMessages << std::endl;
  testSerialization_StringBufferSizes(BOOST_XML, numMessages);
  std::cout << " XML DeSerialization " << numMessages << std::endl;
  testDeSerialization_StringBufferSizes(BOOST_XML, numMessages);

}
void testSerialization_vectorfloat(SERIALIZATION_TYPE type, int numMessages) {
  std::string str;
  std::vector<float> floats;
  floats.assign(26,1.1028284f);

  int numBytes = sizeof(float) * 26;

  std::cout << "Vector<float> binary: " << numMessages << std::endl;
  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  boost::timer t;

  for( int i=0; i< numMessages; i++) {

    str = BoostSerializer::serialize(floats, type);
    std::vector<float>  rep = BoostSerializer::deserialize<std::vector<float> >(str, type);
  }

  double elapsed = t.elapsed();
  float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
  float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
  std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
}


// to keep gtest happy
template<typename T, typename U>
inline std::ostream & operator << (std::ostream & ostr, const std::pair<T,U> & value) {
  ostr << "pair";
        return ostr;
}

// to keep gtest happy
template<typename T, typename U>
inline std::ostream & operator << (std::ostream & ostr, const std::map<T,U> & value) {
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
