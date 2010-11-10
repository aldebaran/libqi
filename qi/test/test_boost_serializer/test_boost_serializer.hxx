
#include <gtest/gtest.h>  // gtest must be included first...!
#include <qi/serialization/boost/boost_serializers.hpp>
#include <qi/tools/dataperftimer.hpp>
#include <string>
#include <qi/messaging/call_definition.hpp>
#include <qi/messaging/result_definition.hpp>

using namespace qi::Serialization;
using qi::Test::DataPerfTimer;

unsigned int numPowers = 12;
unsigned int numMessages = 10000;

template<typename T>
void testSerializationDeserializationPerf(const T& arg)
{
  DataPerfTimer dt("", false);
  std::string buf;
  T res1;

  std::cout << "Serialize" << std::endl;
  dt.start(numMessages);
  // serialize
  for (unsigned int i = 0; i < numMessages; ++i) {
    buf = BoostBinarySerializer::serialize(arg);
  }
  dt.stop();

  buf = BoostBinarySerializer::serialize(arg);

  std::cout << "DeSerialize" << std::endl;
  dt.start(numMessages);
  for (unsigned int i = 0; i < numMessages; ++i) {
    res1 = BoostBinarySerializer::deserialize<T>(buf);
  }
  dt.stop();

  // deserialize to ref
  std::cout << "DeSerializeToRef" << std::endl;
  dt.start(numMessages);
  for (unsigned int i = 0; i < numMessages; ++i) {
    BoostBinarySerializer::deserialize(buf, res1);
  }
  dt.stop();
}


// test three serializations for type T (not a list)
template<typename T>
void testSerializationDeserialization(const T& arg)
{
  std::string buf;
  buf = BoostBinarySerializer::serialize(arg);
  T res1 = BoostBinarySerializer::deserialize<T>(buf);
  EXPECT_EQ(arg, res1);

  buf = BoostTextSerializer::serialize(arg);
  T res2 = BoostTextSerializer::deserialize<T>(buf);
  EXPECT_EQ(arg, res2);

  buf = BoostXmlSerializer::serialize(arg);
  T res3 = BoostXmlSerializer::deserialize<T>(buf);
  EXPECT_EQ(arg, res3);

  // to ref
  buf = BoostBinarySerializer::serialize(arg);
  BoostBinarySerializer::deserialize(buf, res3);
  EXPECT_EQ(arg, res3);
}


template<typename T>
void testSerializationDeserializationList(const T& arg)
{
  std::string buf;
  buf = BoostBinarySerializer::serialize(arg);
  T res1 = BoostBinarySerializer::deserialize<T>(buf);
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res1[i]);
  }

  buf = BoostTextSerializer::serialize(arg);
  T res2 = BoostTextSerializer::deserialize<T>(buf);
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res2[i]);
  }

  buf = BoostXmlSerializer::serialize(arg);
  T res3 = BoostXmlSerializer::deserialize<T>(buf);
  for (unsigned int i = 0; i < arg.size(); i++) {
    EXPECT_EQ(arg[i], res3[i]);
  }
}

void testSerialization_StringBufferSizes(int numMessages) {
  DataPerfTimer dt("Serialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);

    dt.start(numMessages, numBytes);
    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostBinarySerializer::serialize(request);
    }
    dt.stop();
  }
}

void testDeSerialization_StringBufferSizes(int numMessages) {
  DataPerfTimer dt("Deserialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::CallDefinition def;
    def.methodName() = "test2";
    def.args().push_back(request);

    std::string buffer = BoostBinarySerializer::serialize(def);


    dt.start(numMessages, numBytes);

    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      qi::Messaging::CallDefinition reply = BoostBinarySerializer::deserialize<qi::Messaging::CallDefinition>(buffer);
    }

    dt.stop();
  }
}

void testSerialization_CallDefBufferSizes(int numMessages) {
  DataPerfTimer dt("Serialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::CallDefinition def;
    def.methodName() = "test2";
    def.args().push_back(request);

    dt.start(numMessages, numBytes);
    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostBinarySerializer::serialize(def);
    }
    dt.stop();
  }
}

void testDeSerialization_CallDefBufferSizes(int numMessages) {
 DataPerfTimer dt("Deserialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::CallDefinition def;
    def.methodName() = "test2";
    def.args().push_back(request);
    std::string buffer = BoostBinarySerializer::serialize(def);

    dt.start(numMessages, numBytes);

    for (int loop = 0; loop < numMessages; loop++) {
      // DeSerialize
      qi::Messaging::CallDefinition reply = BoostBinarySerializer::deserialize<qi::Messaging::CallDefinition>(buffer);
    }

    dt.stop();
  }
}


void testSerialization_VariableValueBufferSizes(int numMessages) {
  DataPerfTimer dt("Serialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::VariableValue def;
    def = request;

    dt.start(numMessages, numBytes);
    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostBinarySerializer::serialize(def);
    }
    dt.stop();
  }
}

void testDeSerialization_VariableValueBufferSizes(int numMessages) {
  DataPerfTimer dt("Deserialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::VariableValue def;
    def = request;
    std::string buffer = BoostBinarySerializer::serialize(def);

    dt.start(numMessages, numBytes);

    for (int loop = 0; loop < numMessages; loop++) {
      // DeSerialize
      qi::Messaging::VariableValue reply = BoostBinarySerializer::deserialize<qi::Messaging::VariableValue>(buffer);
    }

    dt.stop();
  }
}

void testSerialization_ValueTypeBufferSizes(int numMessages) {
  DataPerfTimer dt("Serialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::ValueType def;
    def = request;

    dt.start(numMessages, numBytes);
    for (int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string reply = BoostBinarySerializer::serialize(def);
    }
    dt.stop();
  }
}

void testDeSerialization_ValueTypeBufferSizes(int numMessages) {
  DataPerfTimer dt("Deserialization");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    qi::Messaging::ValueType def;
    def = request;
    std::string buffer = BoostBinarySerializer::serialize(def);

    dt.start(numMessages, numBytes);

    for (int loop = 0; loop < numMessages; loop++) {
      // DeSerialize
      qi::Messaging::ValueType reply = BoostBinarySerializer::deserialize<qi::Messaging::ValueType>(buffer);
    }

    dt.stop();
  }
}

// not used
void testSerialization_vectorfloat(int numMessages) {
  std::string str;
  std::vector<float> floats;
  floats.assign(26, 1.1028284f);

  int numBytes = sizeof(float) * 26;

  std::cout << "Vector<float> binary: " << numMessages << std::endl;
  std::cout << "Bytes, msg/s, MB/s" << std::endl;
  boost::timer t;

  for (int i = 0; i < numMessages; i++) {
    str = BoostBinarySerializer::serialize(floats);
    std::vector<float>  rep = BoostBinarySerializer::deserialize<std::vector<float> >(str);
  }

  double elapsed = t.elapsed();
  float msgPs = 1.0f / ((float)elapsed / (1.0f * numMessages) );
  float mgbPs = (msgPs * numBytes) / (1024 * 1024.0f);
  std::cout << numBytes << ", " << msgPs << ", " << mgbPs << std::endl;
}

