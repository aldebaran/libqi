
#include <gtest/gtest.h>
// gtest must be included before variables_list: errno_t error
#include <alcommon-ng/collections/variables_list.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>

using namespace AL::Messaging;

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

TEST(VariableValueTest, DISABLED_ToAndFrom_vector_of_float)
{
  std::vector<float> s;
  s.push_back((float)0.2f);
  VariableValue v = s;
  //std::vector<float> res = v;  // doesn't work
  //EXPECT_EQ(s, res); 
}

