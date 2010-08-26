
#include <alcommon-ng/serialization/boost_serializer.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace AL::Serialization;
using AL::Test::DataPerfTimer;

unsigned int numPowers = 12;
unsigned int numMessages = 10000;

template<typename T>
void testSerializationDeserializationPerf(const T& arg, SERIALIZATION_TYPE type)
{
  DataPerfTimer dt("", false);
  std::string buf;
  T res1;

  std::cout << "Serialize" << std::endl;
  dt.start(numMessages);
  // serialize
  for (unsigned int i = 0; i < numMessages; ++i) {
    buf = BoostSerializer::serialize(arg, type);
  }
  dt.stop();

  buf = BoostSerializer::serialize(arg, type);

  std::cout << "DeSerialize" << std::endl;
  dt.start(numMessages);
  for (unsigned int i = 0; i < numMessages; ++i) {
    res1 = BoostSerializer::deserialize<T>(buf, type);
  }
  dt.stop();

  // deserialize to ref
  std::cout << "DeSerializeToRef" << std::endl;
  dt.start(numMessages);
  for (unsigned int i = 0; i < numMessages; ++i) {
    BoostSerializer::deserialize(buf, res1, type);
  }
  dt.stop();
}


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

  // to ptr
  buf = BoostSerializer::serialize(arg);
  boost::shared_ptr<T> res4 = BoostSerializer::deserializeToPtr<T>(buf);
  EXPECT_EQ(arg, *res4.get());

  // to ref
  buf = BoostSerializer::serialize(arg);
  BoostSerializer::deserialize(buf, res3);
  EXPECT_EQ(arg, *res4.get());
}


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
void testSerialization_StringBufferSizes(SERIALIZATION_TYPE type, int numMessages) {
  DataPerfTimer dt("Serialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);

    dt.start(numMessages, numBytes);
    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostSerializer::serialize(request, type);
    }
    dt.stop();
  }
}

void testDeSerialization_StringBufferSizes(SERIALIZATION_TYPE type, int numMessages) {
  DataPerfTimer dt("Deserialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    std::string buffer = BoostSerializer::serialize(request, type);

    dt.start(numMessages, numBytes);

    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostSerializer::deserialize<std::string>(buffer, type);
    }

    dt.stop();
  }
}

// not used
void testSerialization_vectorfloat(SERIALIZATION_TYPE type, int numMessages) {
  std::string str;
  std::vector<float> floats;
  floats.assign(26, 1.1028284f);

  int numBytes = sizeof(float) * 26;

  std::cout << "Vector<float> binary: " << numMessages << std::endl;
  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  boost::timer t;

  for (int i = 0; i < numMessages; i++) {
    str = BoostSerializer::serialize(floats, type);
    std::vector<float>  rep = BoostSerializer::deserialize<std::vector<float> >(str, type);
  }

  double elapsed = t.elapsed();
  float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
  float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
  std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
}

