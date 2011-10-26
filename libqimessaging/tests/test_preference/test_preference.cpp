#include <gtest/gtest.h>
#include <string>
#include <map>
#include <climits>
#include <qipreference/preference.hpp>


TEST(qipreference, basicValue)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/basicValue.xml");

  qi::ValueMap v = pm.values();
  qi::ValueMap &vm = (v.begin()->second).value<qi::ValueMap>();
  qi::ValueMap::iterator it;

  it = vm.find("bool value");
  if (it != vm.end())
    EXPECT_EQ(true, (it->second).toBool());
  else
    std::cerr << "Error cannot find 'bool value' value!" << std::endl;

  it = vm.find("char value");
  if (it != vm.end())
    EXPECT_EQ('d', (it->second).toChar());
  else
    std::cerr << "Error cannot find 'char value' value!" << std::endl;

  it = vm.find("int value");
  if (it != vm.end())
    EXPECT_EQ(-2147483647, (it->second).toInt32());
  else
    std::cerr << "Error cannot find 'int value' value!" << std::endl;

  it = vm.find("uint value");
  if (it != vm.end())
    EXPECT_EQ(2147483647, (it->second).toUInt32());
  else
    std::cerr << "Error cannot find 'uint value' value!" << std::endl;

  it = vm.find("long long value");
  if (it != vm.end())
    EXPECT_EQ(-2147483647, (it->second).toInt64());
  else
    std::cerr << "Error cannot find 'long long value' value!" << std::endl;

  it = vm.find("ulong long value");
  if (it != vm.end())
    EXPECT_EQ(2147483647, (it->second).toUInt64());
  else
    std::cerr << "Error cannot find 'ulong long value' value!" << std::endl;

  // it = vm.find("float value");
  // if (it != vm.end())
  //   EXPECT_EQ(-1.42, (it->second).toFloat());
  // else
  //   std::cerr << "Error cannot find value!" << std::endl;

  // it = vm.find("double value");
  // if (it != vm.end())
  //   EXPECT_EQ(-170141183460469231731687303715884105728, (it->second).toDouble());
  // else
  //   std::cerr << "Error cannot find value!" << std::endl;

  it = vm.find("string value");
  if (it != vm.end())
    EXPECT_EQ("test string", (it->second).toString());
  else
    std::cerr << "Error cannot find 'string value' value!" << std::endl;
}

TEST(qipreference, simpleArray)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/simpleArray.xml");

  qi::ValueMap v = pm.values();
  qi::ValueMap &vm = (v.begin()->second).value<qi::ValueMap>();
  qi::ValueMap::iterator it;

  it = vm.find("simple array");
  if (it != vm.end())
  {
    qi::ValueMap &vm1 = (it->second).value<qi::ValueMap>();
    qi::ValueMap::iterator it1;

    it1 = vm1.find("bool value");
    if (it1 != vm1.end())
      EXPECT_EQ(true, (it1->second).toBool());
    else
      std::cerr << "Error cannot find 'bool value' value!" << std::endl;

    it1 = vm1.find("char value");
    if (it1 != vm1.end())
      EXPECT_EQ('d', (it1->second).toChar());
    else
      std::cerr << "Error cannot find 'char value' value!" << std::endl;

    it1 = vm1.find("int value");
    if (it1 != vm1.end())
      EXPECT_EQ(-2147483647, (it1->second).toInt32());
    else
      std::cerr << "Error cannot find 'int value' value!" << std::endl;

    it1 = vm1.find("uint value");
    if (it1 != vm1.end())
      EXPECT_EQ(2147483647, (it1->second).toUInt32());
    else
      std::cerr << "Error cannot find 'uint value' value!" << std::endl;

    it1 = vm1.find("long long value");
    if (it1 != vm1.end())
      EXPECT_EQ(-2147483647, (it1->second).toInt64());
    else
      std::cerr << "Error cannot find 'long long value' value!" << std::endl;

    it1 = vm1.find("ulong long value");
    if (it1 != vm1.end())
      EXPECT_EQ(2147483647, (it1->second).toUInt64());
    else
      std::cerr << "Error cannot find 'ulong long value' value!" << std::endl;

    // it1 = vm1.find("float value");
    // if (it1 != vm1.end())
    //   EXPECT_EQ(-1.42, (it1->second).toFloat());
    // else
    //   std::cerr << "Error cannot find value!" << std::endl;

    // it1 = vm1.find("double value");
    // if (it1 != vm1.end())
    //   EXPECT_EQ(-170141183460469231731687303715884105728, (it1->second).toDouble());
    // else
    //   std::cerr << "Error cannot find value!" << std::endl;

    it1 = vm1.find("string value");
    if (it1 != vm1.end())
      EXPECT_EQ("test string", (it1->second).toString());
    else
      std::cerr << "Error cannot find 'string value' value!" << std::endl;
  }
  else
  {
    std::cerr << "Error cannot find 'simple array' value!" << std::endl;
  }
}

TEST(qipreference, advanceValue)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/advanceValue.xml");

  qi::ValueMap v = pm.values();
  qi::ValueMap &vm = (v.begin()->second).value<qi::ValueMap>();
  qi::ValueMap::iterator it;

  it = vm.find("simple array 2");
  if (it != vm.end())
  {
    qi::ValueMap &vm1 = (it->second).value<qi::ValueMap>();
    qi::ValueMap::iterator it1;

    it1 = vm1.find("bool value");
    if (it1 != vm1.end())
      EXPECT_EQ(true, (it1->second).toBool());
    else
      std::cerr << "Error cannot find 'bool value' value!" << std::endl;

    it1 = vm1.find("char value");
    if (it1 != vm1.end())
      EXPECT_EQ('d', (it1->second).toChar());
    else
      std::cerr << "Error cannot find 'char value' value!" << std::endl;

    it1 = vm1.find("int value");
    if (it1 != vm1.end())
      EXPECT_EQ(-2147483647, (it1->second).toInt32());
    else
      std::cerr << "Error cannot find 'int value' value!" << std::endl;

    it1 = vm1.find("uint value");
    if (it1 != vm1.end())
      EXPECT_EQ(2147483647, (it1->second).toUInt32());
    else
      std::cerr << "Error cannot find 'uint value' value!" << std::endl;

    it1 = vm1.find("long long value");
    if (it1 != vm1.end())
      EXPECT_EQ(-2147483647, (it1->second).toInt64());
    else
      std::cerr << "Error cannot find 'long long value' value!" << std::endl;

    it1 = vm1.find("ulong long value");
    if (it1 != vm1.end())
      EXPECT_EQ(2147483647, (it1->second).toUInt64());
    else
      std::cerr << "Error cannot find 'ulong long value' value!" << std::endl;

    // it1 = vm1.find("float value");
    // if (it1 != vm1.end())
    //   EXPECT_EQ(-1.42, (it1->second).toFloat());
    // else
    //   std::cerr << "Error cannot find value!" << std::endl;

    // it1 = vm1.find("double value");
    // if (it1 != vm1.end())
    //   EXPECT_EQ(-170141183460469231731687303715884105728, (it1->second).toDouble());
    // else
    //   std::cerr << "Error cannot find value!" << std::endl;

    it1 = vm1.find("string value");
    if (it1 != vm1.end())
      EXPECT_EQ("test string", (it1->second).toString());
    else
      std::cerr << "Error cannot find 'string value' value!" << std::endl;
  }
  else
  {
    std::cerr << "Error cannot find 'simple array 2' value!" << std::endl;
  }

  it = vm.find("simple array 1");
  if (it != vm.end())
  {
    qi::ValueMap &vm1 = (it->second).value<qi::ValueMap>();
    qi::ValueMap::iterator it1;

    it1 = vm1.find("bool value");
    if (it1 != vm1.end())
      EXPECT_EQ(true, (it1->second).toBool());
    else
      std::cerr << "Error cannot find 'bool value' value!" << std::endl;

    it1 = vm1.find("char value");
    if (it1 != vm1.end())
      EXPECT_EQ('d', (it1->second).toChar());
    else
      std::cerr << "Error cannot find 'char value' value!" << std::endl;

    it1 = vm1.find("int value");
    if (it1 != vm1.end())
      EXPECT_EQ(-2147483647, (it1->second).toInt32());
    else
      std::cerr << "Error cannot find 'int value' value!" << std::endl;

    it1 = vm1.find("uint value");
    if (it1 != vm1.end())
      EXPECT_EQ(2147483647, (it1->second).toUInt32());
    else
      std::cerr << "Error cannot find 'uint value' value!" << std::endl;

    it1 = vm1.find("long long value");
    if (it1 != vm1.end())
      EXPECT_EQ(-2147483647, (it1->second).toInt64());
    else
      std::cerr << "Error cannot find 'long long value' value!" << std::endl;

    it1 = vm1.find("ulong long value");
    if (it1 != vm1.end())
      EXPECT_EQ(2147483647, (it1->second).toUInt64());
    else
      std::cerr << "Error cannot find 'ulong long value' value!" << std::endl;

    // it1 = vm1.find("float value");
    // if (it1 != vm1.end())
    //   EXPECT_EQ(-1.42, (it1->second).toFloat());
    // else
    //   std::cerr << "Error cannot find value!" << std::endl;

    // it1 = vm1.find("double value");
    // if (it1 != vm1.end())
    //   EXPECT_EQ(-170141183460469231731687303715884105728, (it1->second).toDouble());
    // else
    //   std::cerr << "Error cannot find value!" << std::endl;

    it1 = vm1.find("string value");
    if (it1 != vm1.end())
      EXPECT_EQ("test string", (it1->second).toString());
    else
      std::cerr << "Error cannot find 'string value' value!" << std::endl;
  }
  else
  {
    std::cerr << "Error cannot find 'simple array 1' value!" << std::endl;
  }

  it = vm.find("after array");
  if (it != vm.end())
    EXPECT_EQ("This is a string", (it->second).toString());
  else
    std::cerr << "Error cannot find 'after array' value!" << std::endl;


  it = vm.find("before array");
  if (it != vm.end())
    EXPECT_EQ(-21, (it->second).toInt32());
  else
    std::cerr << "Error cannot find 'before array' value!" << std::endl;
}


TEST(qipreference, getAdvanceValue)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/advanceValue.xml");

  qi::Value vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/simple array 2/");
  EXPECT_EQ(9, vi.toMap().size());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1");
  EXPECT_EQ(9, vi.toMap().size());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/skdjkjkjsd");
  EXPECT_EQ(qi::Value::Invalid, vi._private.type);


  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/bool value/");
  EXPECT_EQ(true, vi.toBool());

  vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/simple array 2/bool value/");
  EXPECT_EQ(true, vi.toBool());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/bool value");
  EXPECT_EQ(true, vi.toBool());

  vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/simple array 2/bool value");
  EXPECT_EQ(true, vi.toBool());


  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/char value/");
  EXPECT_EQ('d', vi.toChar());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/int value/");
  EXPECT_EQ(-2147483647, vi.toInt32());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/uint value/");
  EXPECT_EQ(2147483647, vi.toUInt32());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/long long value/");
  EXPECT_EQ(-2147483647, vi.toInt64());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/ulong long value/");
  EXPECT_EQ(2147483647, vi.toUInt64());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 2/string value/");
  EXPECT_EQ("test string", vi.toString());


  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/bool value/");
  EXPECT_EQ(true, vi.toBool());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/char value/");
  EXPECT_EQ('d', vi.toChar());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/int value/");
  EXPECT_EQ(-2147483647, vi.toInt32());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/uint value/");
  EXPECT_EQ(2147483647, vi.toUInt32());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/long long value/");
  EXPECT_EQ(-2147483647, vi.toInt64());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/ulong long value/");
  EXPECT_EQ(2147483647, vi.toUInt64());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/simple array 1/string value/");
  EXPECT_EQ("test string", vi.toString());



  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/after array/");
  EXPECT_EQ("This is a string", vi.toString());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/before array/");
  EXPECT_EQ(-21, vi.toInt32());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/after array");
  EXPECT_EQ("This is a string", vi.toString());

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/before array");
  EXPECT_EQ(-21, vi.toInt32());

  vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/after array/");
  EXPECT_EQ("This is a string", vi.toString());

  vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/before array/");
  EXPECT_EQ(-21, vi.toInt32());

  vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/after array");
  EXPECT_EQ("This is a string", vi.toString());

  vi = pm.get("aldebaran-robotics.com@ALTextToSpeech/before array/");
  EXPECT_EQ(-21, vi.toInt32());
}

TEST(qipreference, setValue)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/basicValue.xml");

  qi::Value vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/int value/");
  EXPECT_EQ(-2147483647, vi.toInt32());

  int i = 42;
  qi::Value v(i);
  pm.set("/aldebaran-robotics.com@ALTextToSpeech/int value/", v);

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/int value/");
  EXPECT_EQ(42, vi.toInt32());


  std::string s = "42";
  qi::Value vs(s);
  pm.set("/aldebaran-robotics.com@ALTextToSpeech/int value/", vs);

  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/int value/");
  EXPECT_EQ("42", vi.toString());
}



TEST(qipreference, removeValue)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/basicValue.xml");

  qi::Value vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/int value/");
  EXPECT_EQ(-2147483647, vi.toInt32());

  pm.remove("/aldebaran-robotics.com@ALTextToSpeech/int value/");
  vi = pm.get("/aldebaran-robotics.com@ALTextToSpeech/int value/");
  EXPECT_EQ(vi._private.type, qi::Value::Invalid);
}


TEST(qipreference, getKeys)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/advanceValue.xml");

  std::vector<std::string> k = pm.keys("/aldebaran-robotics.com@ALTextToSpeech/sim");
  for (int i = 0; i < k.size(); ++i)
    std::cout << k[i] << std::endl;

  k = pm.keys("/");
  k = pm.keys("erw");
}



TEST(qipreference, saveKeys)
{
  qi::pref::PreferenceMap pm;
  pm.load("/home/hcuche/src/qi/qimessaging/libqimessaging/tests/test_preference/xml_examples/advanceValue.xml");

  pm.save("/home/hcuche/test.xml");
}
