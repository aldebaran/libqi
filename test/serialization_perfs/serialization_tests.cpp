
#include "gtest/gtest.h"
#include <alcommon-ng/serialization/serialization.h>
#include <boost/timer.hpp>
#include <string>

using namespace AL::Serialization;
int numMessages = 1000;

TEST(SerializationTest, TextSerializationSimpleTypes)
{
  //    float three = 3.0f;
  std::string text = "three";
  std::string str;

  // text
  str = serialize(3, TEXT);
  int str_int = deserialize<int>(str, TEXT);
  EXPECT_EQ(3, str_int);

  str = serialize(3.0f, TEXT);
  float str_float = deserialize<float>(str, TEXT);
  EXPECT_EQ(3.0f, str_float);

  str = serialize(text, TEXT);
  std::string str_str = deserialize<std::string>(str, TEXT);
  EXPECT_EQ(text, str_str);

  std::vector<float> floats;
  floats.assign(26,1.1028284f);
  str = serialize(floats, TEXT);
  std::vector<float>  rep = deserialize<std::vector<float> >(str, TEXT);
  for (unsigned int i = 0; i < floats.size(); ++i) {
    EXPECT_EQ(floats[i], rep[i]);
  }
}

TEST(SerializationTest, XmlSerializationSimpleTypes)
{
  std::string text = "three";
  std::string str;

  // xml
  str = serialize(3, XML);
  int xml_int = deserialize<int>(str, XML);
  EXPECT_EQ(3, xml_int);

  str = serialize(3.0f, XML);
  float xml_float = deserialize<float>(str, XML);
  EXPECT_EQ(3.0f, xml_float);

  str = serialize(text, XML);
  std::string xml_str = deserialize<std::string>(str, XML);
  EXPECT_EQ(text, xml_str);

  std::vector<float> floats;
  floats.assign(26,1.1028284f);
  str = serialize(floats, XML);
  std::vector<float>  rep = deserialize<std::vector<float> >(str, XML);
  for (unsigned int i = 0; i < floats.size(); ++i) {
    EXPECT_EQ(floats[i], rep[i]);
  }
}


TEST(SerializationTest, BinarySerializationSimpleTypes)
{
  std::string text = "three";
  std::string str;

  // bin
  str = serialize(3, BINARY);
  int bin_int = deserialize<int>(str, BINARY);
  EXPECT_EQ(3, bin_int);

  str = serialize(3.0f, BINARY);
  float bin_float = deserialize<float>(str, BINARY);
  EXPECT_EQ(3.0f, bin_float);

  str = serialize(text, BINARY);
  std::string bin_str = deserialize<std::string>(str, BINARY);
  EXPECT_EQ(text, bin_str);

  std::vector<float> floats;
  floats.assign(26,1.1028284f);
  str = serialize(floats, BINARY);
  std::vector<float>  rep = deserialize<std::vector<float> >(str, BINARY);
  for (unsigned int i = 0; i < floats.size(); ++i) {
    EXPECT_EQ(floats[i], rep[i]);
  }

}


TEST(SerializationTest, DefaultSerializationSimpleTypes)
{
  std::string text = "three";
  std::string str;

  // auto binary
  str = serialize(3);
  int aut_int = deserialize<int>(str);
  EXPECT_EQ(3, aut_int);

  str = serialize(3.0f);
  float aut_float = deserialize<float>(str);
  EXPECT_EQ(3.0f, aut_float);

  str = serialize(text);
  std::string aut_str = deserialize<std::string>(str);
  EXPECT_EQ(text, aut_str);

}


TEST(SerializationTest, BinaryToPtrSerializationSimpleTypes)
{
  std::string text = "three";
  std::string str;

  // to ptr
  str = serialize(3);
  boost::shared_ptr<int> ptr_int = deserializeToPtr<int>(str);
  EXPECT_EQ(3, *ptr_int.get());

  str = serialize(3.0f);
  boost::shared_ptr<float> ptr_float = deserializeToPtr<float>(str);
  EXPECT_EQ(3.0f, *ptr_float.get());

  str = serialize(text);
  boost::shared_ptr<std::string> ptr_str = deserializeToPtr<std::string>(str);
  EXPECT_EQ(text, *ptr_str.get());

  // from ptr
}


void testSerialization_StringBufferSizes(AL::Serialization::SERIALIZATION_TYPE type, int numMessages) {

  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  // loop message sizes 2^i bytes
  for (unsigned int i=1; i < 21; i++) {

    char character = 'A';
    unsigned int numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string request = std::string(numBytes, character);

    boost::timer t;
    double elapsed;
    t.restart();

    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = AL::Serialization::serialize(request, type);
    }

    elapsed = t.elapsed();
    float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
    float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
    std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
  }
}

void testDeSerialization_StringBufferSizes(AL::Serialization::SERIALIZATION_TYPE type, int numMessages) {

  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  // loop message sizes 2^i bytes
  for (unsigned int i=1; i < 21; i++) {

    char character = 'A';
    unsigned int numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string request = std::string(numBytes, character);

    boost::timer t;
    double elapsed;
    std::string buffer = AL::Serialization::serialize(request, type);

    t.restart();

    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = AL::Serialization::deserialize<std::string>(buffer, type);
    }

    elapsed = t.elapsed();
    float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
    float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
    std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
  }
}

TEST(SerializationPerformance, DISABLED_binary) {
  //int numMessages = 10000;
  std::cout << " BINARY Serialization " << numMessages << std::endl; 
  testSerialization_StringBufferSizes(BINARY, numMessages);
  std::cout << " BINARY DeSerialization " << numMessages << std::endl; 
  testDeSerialization_StringBufferSizes(BINARY, numMessages);
}

TEST(SerializationPerformance, DISABLED_text) {
  //int numMessages = 10000;

  std::cout << " TEXT Serialization " << numMessages << std::endl;
  testSerialization_StringBufferSizes(TEXT, numMessages);
  std::cout << " TEXT DeSerialization " << numMessages << std::endl;
  testDeSerialization_StringBufferSizes(TEXT, numMessages);
}

TEST(SerializationPerformance, DISABLED_xml) {
  //int numMessages = 10000;

  std::cout << " XML Serialization " << numMessages << std::endl; 
  testSerialization_StringBufferSizes(XML, numMessages);
  std::cout << " XML DeSerialization " << numMessages << std::endl; 
  testDeSerialization_StringBufferSizes(XML, numMessages);

}
void testSerialization_vectorfloat(AL::Serialization::SERIALIZATION_TYPE type, int numMessages) {
  std::string str;
  std::vector<float> floats;
  floats.assign(26,1.1028284f);

  int numBytes = sizeof(float) * 26;
  
  //std::cout << "Vector<float> binary: " << numMessages << std::endl;
  //std::cout << "Bytes, msg/s, MB/s" << std::endl;
  boost::timer t;

  for( int i=0; i< numMessages; i++) {
    
    str = serialize(floats, type);
    std::vector<float>  rep = deserialize<std::vector<float> >(str, type);
  }

  double elapsed = t.elapsed();
  float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
  float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
  //std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
}

TEST(SerializationPerformance, vectorfloatBinary) {
  int numMessages = 1000;
  testSerialization_vectorfloat(BINARY,numMessages);
}

TEST(SerializationPerformance, vectorfloatText) {
  int numMessages = 1000;
  testSerialization_vectorfloat(TEXT,numMessages);
}

TEST(SerializationPerformance, vectorfloatXML) {
  int numMessages = 1000;
  testSerialization_vectorfloat(XML,numMessages);
}
