
#include <gtest/gtest.h>
// gtest must be included before variables_list: errno_t error
#include <alcommon-ng/collections/variables_list.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>

using namespace AL::Messaging;

template<>
void GTestStreamToHelper<AL::ALValue>(std::basic_ostream<char, std::char_traits<char> >* os, AL::ALValue const& val)
{
  *os << val.toString();
}

TEST(VariableValueTest, ToAndFrom_int)
{
  int s = 1;
  VariableValue v = s;
  int res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueTest, ToAndFrom_float)
{
  float s = 0.2f;
  VariableValue v = s;
  float res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueTest, ToAndFrom_double)
{
  double s = 39709080.97987979;
  VariableValue v = s;
  double res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueTest, ToAndFrom_bool)
{
  bool s = true;
  VariableValue v = s;
  bool res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueTest, ToAndFrom_string)
{
  std::string s("hello world");
  VariableValue v = s;
  std::string res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueTest, ToAndFrom_vector_of_unsigned_char)
{
  std::vector<unsigned char> s;
  s.push_back((unsigned char)1);
  VariableValue v = s;
  std::vector<unsigned char> res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueTest, ToAndFrom_vector_of_VariableValue)
{
  std::vector<VariableValue> s;
  s.push_back((unsigned char)1);
  VariableValue v = s;
  std::vector<VariableValue> res = v;
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_int)
{
  AL::ALValue s = 1;
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_float)
{
  AL::ALValue s = 0.2f;
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_double)
{
  AL::ALValue s = 39709080.97987979;
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_bool)
{
  AL::ALValue s = true;
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_string)
{
  AL::ALValue s("hello world");
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_vector_of_unsigned_char)
{
  AL::ALValue s;
  std::vector<unsigned char> s1;
  s1.push_back((unsigned char)1);
  s.SetBinary(&s1,s1.size());
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_mixedArray)
{
  AL::ALValue s;
  s.arrayPush((int)1);
  s.arrayPush(std::string("Hello"));
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  EXPECT_EQ(s, res);
}

TEST(VariableValueConvertALValue, ToAndFrom_vector_float)
{
  std::vector<float> f;
  f.push_back(1.0f);
  f.push_back(2.1f);
  AL::ALValue s = f;
  VariableValue v = s;
  AL::ALValue res = v.convertToALValue();
  std::vector<float> ff = res;
  EXPECT_EQ(f, ff);
}


