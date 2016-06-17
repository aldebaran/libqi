#include <gtest/gtest.h>
#include <qi/macroregular.hpp>

struct S0
{
  QI_GENERATE_FRIEND_REGULAR_OPS_0(S0);
};

struct S1
{
  int a;
  QI_GENERATE_FRIEND_REGULAR_OPS_1(S1, a);
};

struct S2
{
  int a; bool b;
  QI_GENERATE_FRIEND_REGULAR_OPS_2(S2, a, b);
};

struct S3
{
  int a; bool b; char c;
  QI_GENERATE_FRIEND_REGULAR_OPS_3(S3, a, b, c);
};

struct S4
{
  int a; bool b; char c; double d;
  QI_GENERATE_FRIEND_REGULAR_OPS_4(S4, a, b, c, d);
};

struct S5
{
  int a; bool b; char c; double d; short e;
  QI_GENERATE_FRIEND_REGULAR_OPS_5(S5, a, b, c, d, e);
};

struct S6
{
  int a; bool b; char c; double d; short e; float f;
  QI_GENERATE_FRIEND_REGULAR_OPS_6(S6, a, b, c, d, e, f);
};

struct S7
{
  int a; bool b; char c; double d; short e; float f; long g;
  QI_GENERATE_FRIEND_REGULAR_OPS_7(S7, a, b, c, d, e, f, g);
};

struct S8
{
  int a; bool b; char c; double d; short e; float f; long g; unsigned char h;
  QI_GENERATE_FRIEND_REGULAR_OPS_8(S8, a, b, c, d, e, f, g, h);
};

struct S9
{
  int a; bool b; char c; double d; short e; float f; long g; unsigned char h; unsigned int i;
  QI_GENERATE_FRIEND_REGULAR_OPS_9(S9, a, b, c, d, e, f, g, h, i);
};

struct S10
{
  int a; bool b; char c; double d; short e; float f; long g; unsigned char h; unsigned int i; unsigned short j;
  QI_GENERATE_FRIEND_REGULAR_OPS_10(S10, a, b, c, d, e, f, g, h, i, j);
};

TEST(Macro, QI_GENERATE_MEMBERWISE_EQUALITY)
{
  // To be exhaustive, we should test all same/different argument permutations, but this should be ok.
  // 0
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_0(S0{}, S0{}));

  // 1
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_1(S1{0}, S1{0}, a));
  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_1(S1{0}, S1{1}, a));

  // 2
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_2( (S2{0, true}), (S2{0, true}), a, b));
  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_2((S2{1, true}), (S2{0, true}), a, b));
  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_2((S2{0, false}), (S2{0, true}), a, b));

  // 3
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_3( (S3{0, true, 'a'}),
                                                 (S3{0, true, 'a'}), a, b, c));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_3((S3{1, true, 'a'}),
                                                 (S3{0, true, 'a'}), a, b, c));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_3((S3{0, false, 'a'}),
                                                 (S3{0, true, 'a'}), a, b, c));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_3((S3{0, true, 'b'}),
                                                 (S3{0, true, 'a'}), a, b, c));

  // 4
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_4( (S4{0, true, 'a', 1.2}),
                                                 (S4{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_4((S4{1, true, 'a', 1.2}),
                                                 (S4{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_4((S4{0, false, 'a', 1.2}),
                                                 (S4{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_4((S4{0, true, 'b', 1.2}),
                                                 (S4{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_4((S4{0, true, 'a', 1.3}), (S4{0, true, 'a', 1.2}), a, b, c, d));

  // 5
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_5( (S5{0, true, 'a', 1.2, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_5((S5{1, true, 'a', 1.2, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_5((S5{0, false, 'a', 1.2, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_5((S5{0, true, 'b', 1.2, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_5((S5{0, true, 'a', 1.0, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_5((S5{0, true, 'a', 1.2, 4}), (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  // 6
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_6( (S6{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_6((S6{1, true, 'a', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_6((S6{0, false, 'a', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_6((S6{0, true, 'b', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_6((S6{0, true, 'a', 1.0, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_6((S6{0, true, 'a', 1.2, 4, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_6((S6{0, true, 'a', 1.2, 4, 0.5f}),
                                                 (S6{0, true, 'a', 1.2, 4, 4.5f}), a, b, c, d, e, f));

  // 7
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_7( (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{1, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{0, false, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{0, true, 'b', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{0, true, 'a', 1.0, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{0, true, 'a', 1.2, 4, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{0, true, 'a', 1.2, 3, 0.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_7((S7{0, true, 'a', 1.2, 3, 4.5f, 6L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  // 8
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_8( (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

 // 9
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_9( (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  // 10
  EXPECT_TRUE(QI_GENERATE_MEMBERWISE_EQUALITY_10( (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(QI_GENERATE_MEMBERWISE_EQUALITY_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));
}

TEST(Macro, QI_GENERATE_REGULAR_OP_EQUAL)
{
  // To be exhaustive, we should test all same/different argument permutations, but this should be ok.
  EXPECT_TRUE((S0{} == S0{}));

  EXPECT_TRUE((S1{0} == S1{0}));
  EXPECT_FALSE((S1{0} == S1{1}));

  EXPECT_TRUE(((S2{0, true}) == (S2{0, true})));
  EXPECT_FALSE(((S2{1, true}) == (S2{0, true})));
  EXPECT_FALSE(((S2{0, false}) == (S2{0, true})));

  EXPECT_TRUE(((S3{0, true, 'a'}) == (S3{0, true, 'a'})));
  EXPECT_FALSE(((S3{1, true, 'a'}) == (S3{0, true, 'a'})));
  EXPECT_FALSE(((S3{0, false, 'a'}) == (S3{0, true, 'a'})));
  EXPECT_FALSE(((S3{0, true, 'b'}) == (S3{0, true, 'a'})));

  EXPECT_TRUE(((S4{0, true, 'a', 1.2}) == (S4{0, true, 'a', 1.2})));
  EXPECT_FALSE(((S4{1, true, 'a', 1.2}) == (S4{0, true, 'a', 1.2})));
  EXPECT_FALSE(((S4{0, false, 'a', 1.2}) == (S4{0, true, 'a', 1.2})));
  EXPECT_FALSE(((S4{0, true, 'b', 1.2}) == (S4{0, true, 'a', 1.2})));
  EXPECT_FALSE(((S4{0, true, 'a', 1.3}) == (S4{0, true, 'a', 1.2})));

  EXPECT_TRUE(((S5{0, true, 'a', 1.2, 3}) == (S5{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((S5{1, true, 'a', 1.2, 3}) == (S5{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((S5{0, false, 'a', 1.2, 3}) == (S5{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((S5{0, true, 'b', 1.2, 3}) == (S5{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((S5{0, true, 'a', 1.0, 3}) == (S5{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((S5{0, true, 'a', 1.2, 4}) == (S5{0, true, 'a', 1.2, 3})));

  EXPECT_TRUE(((S6{0, true, 'a', 1.2, 3, 4.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((S6{1, true, 'a', 1.2, 3, 4.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((S6{0, false, 'a', 1.2, 3, 4.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((S6{0, true, 'b', 1.2, 3, 4.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((S6{0, true, 'a', 1.0, 3, 4.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((S6{0, true, 'a', 1.2, 4, 4.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((S6{0, true, 'a', 1.2, 4, 0.5f}) == (S6{0, true, 'a', 1.2, 3, 4.5f})));

  EXPECT_TRUE(((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{1, true, 'a', 1.2, 3, 4.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{0, false, 'a', 1.2, 3, 4.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{0, true, 'b', 1.2, 3, 4.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{0, true, 'a', 1.0, 3, 4.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{0, true, 'a', 1.2, 4, 4.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{0, true, 'a', 1.2, 3, 0.5f, 5L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((S7{0, true, 'a', 1.2, 3, 4.5f, 6L}) == (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));

  EXPECT_TRUE(((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}) == (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));

  EXPECT_TRUE(((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}) == (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}) == (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
}

TEST(Macro, QI_GENERATE_LEXICOGRAPHICAL_LESS)
{
  // To be exhaustive, we should test all same/different argument permutations, but this should be ok.
  // 0
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_0(S0{}, S0{}));

  // 1
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_1(S1{0}, S1{0}, a));
  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_1( S1{0}, S1{1}, a));

  // 2
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_2((S2{0, true}), (S2{0, true}), a, b));
  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_2( (S2{0, true}), (S2{1, true}), a, b));
  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_2( (S2{0, false}), (S2{0, true}), a, b));

  // 3
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_3((S3{0, true, 'a'}),
                                                  (S3{0, true, 'a'}), a, b, c));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_3((S3{0, true, 'a'}),
                                                 (S3{1, true, 'a'}), a, b, c));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_3((S3{0, false, 'a'}),
                                                 (S3{0, true, 'a'}), a, b, c));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_3((S3{0, true, 'a'}),
                                                 (S3{0, true, 'b'}), a, b, c));

  // 4
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_4((S4{0, true, 'a', 1.2}),
                                                  (S4{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_4((S4{0, true, 'a', 1.2}),
                                                 (S4{1, true, 'a', 1.2}), a, b, c, d));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_4((S4{0, false, 'a', 1.2}),
                                                 (S4{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_4((S4{0, true, 'a', 1.2}),
                                                 (S4{0, true, 'b', 1.2}), a, b, c, d));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_4((S4{0, true, 'a', 1.2}),
                                                 (S4{0, true, 'a', 1.3}), a, b, c, d));

  // 5
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_5((S5{0, true, 'a', 1.2, 3}),
                                                  (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_5((S5{0, true, 'a', 1.2, 3}),
                                                 (S5{1, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_5((S5{0, false, 'a', 1.2, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_5((S5{0, true, 'a', 1.2, 3}),
                                                 (S5{0, true, 'b', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_5((S5{0, true, 'a', 1.0, 3}),
                                                 (S5{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_5((S5{0, true, 'a', 1.2, 3}),
                                                 (S5{0, true, 'a', 1.2, 4}), a, b, c, d, e));

  // 6
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, true, 'a', 1.2, 3, 4.5f}),
                                                  (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (S6{1, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, false, 'a', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'b', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, true, 'a', 1.0, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (S6{0, true, 'a', 1.2, 4, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_6((S6{0, true, 'a', 1.2, 4, 0.5f}),
                                                 (S6{0, true, 'a', 1.2, 4, 4.5f}), a, b, c, d, e, f));

  // 7
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                  (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{1, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, false, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'b', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.0, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 4, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.2, 3, 0.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_7((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (S7{0, true, 'a', 1.2, 3, 4.5f, 6L}), a, b, c, d, e, f, g));

  // 8
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                  (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_8((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}), a, b, c, d, e, f, g, h));

 // 9
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                  (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_9((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}), a, b, c, d, e, f, g, h, i));

  // 10
  EXPECT_FALSE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                   (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(QI_GENERATE_LEXICOGRAPHICAL_LESS_10((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}), a, b, c, d, e, f, g, h, i, j));
}

TEST(Macro, QI_GENERATE_REGULAR_OP_LESS)
{
  // To be exhaustive, we should test all same/different argument permutations, but this should be ok.
  // 0
  EXPECT_FALSE((S0{} < S0{}));

  // 1
  EXPECT_FALSE((S1{0} < S1{0}));
  EXPECT_TRUE(( S1{0} < S1{1}));

  // 2
  EXPECT_FALSE((S2{0, true}) < (S2{0, true}));
  EXPECT_TRUE( (S2{0, true}) < (S2{1, true}));
  EXPECT_TRUE( (S2{0, false}) < (S2{0, true}));

  // 3
  EXPECT_FALSE((S3{0, true, 'a'}) < (S3{0, true, 'a'}));
  EXPECT_TRUE((S3{0, true, 'a'}) < (S3{1, true, 'a'}));
  EXPECT_TRUE((S3{0, false, 'a'}) < (S3{0, true, 'a'}));
  EXPECT_TRUE((S3{0, true, 'a'}) < (S3{0, true, 'b'}));

  // 4
  EXPECT_FALSE((S4{0, true, 'a', 1.2}) < (S4{0, true, 'a', 1.2}));
  EXPECT_TRUE((S4{0, true, 'a', 1.2}) < (S4{1, true, 'a', 1.2}));
  EXPECT_TRUE((S4{0, false, 'a', 1.2}) < (S4{0, true, 'a', 1.2}));
  EXPECT_TRUE((S4{0, true, 'a', 1.2}) < (S4{0, true, 'b', 1.2}));
  EXPECT_TRUE((S4{0, true, 'a', 1.2}) < (S4{0, true, 'a', 1.3}));

  // 5
  EXPECT_FALSE((S5{0, true, 'a', 1.2, 3}) < (S5{0, true, 'a', 1.2, 3}));
  EXPECT_TRUE((S5{0, true, 'a', 1.2, 3}) < (S5{1, true, 'a', 1.2, 3}));
  EXPECT_TRUE((S5{0, false, 'a', 1.2, 3}) < (S5{0, true, 'a', 1.2, 3}));
  EXPECT_TRUE((S5{0, true, 'a', 1.2, 3}) < (S5{0, true, 'b', 1.2, 3}));
  EXPECT_TRUE((S5{0, true, 'a', 1.0, 3}) < (S5{0, true, 'a', 1.2, 3}));
  EXPECT_TRUE((S5{0, true, 'a', 1.2, 3}) < (S5{0, true, 'a', 1.2, 4}));

  // 6
  EXPECT_FALSE((S6{0, true, 'a', 1.2, 3, 4.5f}) < (S6{0, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((S6{0, true, 'a', 1.2, 3, 4.5f}) < (S6{1, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((S6{0, false, 'a', 1.2, 3, 4.5f}) < (S6{0, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((S6{0, true, 'a', 1.2, 3, 4.5f}) < (S6{0, true, 'b', 1.2, 3, 4.5f}));
  EXPECT_TRUE((S6{0, true, 'a', 1.0, 3, 4.5f}) < (S6{0, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((S6{0, true, 'a', 1.2, 3, 4.5f}) < (S6{0, true, 'a', 1.2, 4, 4.5f}));
  EXPECT_TRUE((S6{0, true, 'a', 1.2, 4, 0.5f}) < (S6{0, true, 'a', 1.2, 4, 4.5f}));

  // 7
  EXPECT_FALSE((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
               (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (S7{1, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, false, 'a', 1.2, 3, 4.5f, 5L}) <
              (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (S7{0, true, 'b', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, true, 'a', 1.0, 3, 4.5f, 5L}) <
              (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (S7{0, true, 'a', 1.2, 4, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, true, 'a', 1.2, 3, 0.5f, 5L}) <
              (S7{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (S7{0, true, 'a', 1.2, 3, 4.5f, 6L}));

  // 8
  EXPECT_FALSE((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
               (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (S8{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (S8{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}) <
              (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (S8{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}) <
              (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (S8{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}));

  EXPECT_TRUE((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}));

 // 9
  EXPECT_FALSE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
               (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}));

  EXPECT_TRUE((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}));

  // 10
  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
               (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}));

  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}));
}

TEST(Macro, QI_GENERATE_REGULAR_OP_DIFFERENT)
{
  // To be exhaustive, we should test all same/different argument permutations, but this should be ok.
  EXPECT_FALSE((S0{} != S0{}));

  EXPECT_FALSE((S1{0} != S1{0}));
  EXPECT_TRUE((S1{0} != S1{1}));

  EXPECT_FALSE(((S2{0, true}) != (S2{0, true})));
  EXPECT_TRUE(((S2{1, true}) != (S2{0, true})));
  EXPECT_TRUE(((S2{0, false}) != (S2{0, true})));

  EXPECT_FALSE(((S3{0, true, 'a'}) != (S3{0, true, 'a'})));
  EXPECT_TRUE(((S3{1, true, 'a'}) != (S3{0, true, 'a'})));
  EXPECT_TRUE(((S3{0, false, 'a'}) != (S3{0, true, 'a'})));
  EXPECT_TRUE(((S3{0, true, 'b'}) != (S3{0, true, 'a'})));

  EXPECT_FALSE(((S4{0, true, 'a', 1.2}) != (S4{0, true, 'a', 1.2})));
  EXPECT_TRUE(((S4{1, true, 'a', 1.2}) != (S4{0, true, 'a', 1.2})));
  EXPECT_TRUE(((S4{0, false, 'a', 1.2}) != (S4{0, true, 'a', 1.2})));
  EXPECT_TRUE(((S4{0, true, 'b', 1.2}) != (S4{0, true, 'a', 1.2})));
  EXPECT_TRUE(((S4{0, true, 'a', 1.3}) != (S4{0, true, 'a', 1.2})));

  EXPECT_FALSE(((S5{0, true, 'a', 1.2, 3}) != (S5{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((S5{1, true, 'a', 1.2, 3}) != (S5{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((S5{0, false, 'a', 1.2, 3}) != (S5{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((S5{0, true, 'b', 1.2, 3}) != (S5{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((S5{0, true, 'a', 1.0, 3}) != (S5{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((S5{0, true, 'a', 1.2, 4}) != (S5{0, true, 'a', 1.2, 3})));

  EXPECT_FALSE(((S6{0, true, 'a', 1.2, 3, 4.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((S6{1, true, 'a', 1.2, 3, 4.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((S6{0, false, 'a', 1.2, 3, 4.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((S6{0, true, 'b', 1.2, 3, 4.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((S6{0, true, 'a', 1.0, 3, 4.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((S6{0, true, 'a', 1.2, 4, 4.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((S6{0, true, 'a', 1.2, 4, 0.5f}) != (S6{0, true, 'a', 1.2, 3, 4.5f})));

  EXPECT_FALSE(((S7{0, true, 'a', 1.2, 3, 4.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{1, true, 'a', 1.2, 3, 4.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{0, false, 'a', 1.2, 3, 4.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{0, true, 'b', 1.2, 3, 4.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{0, true, 'a', 1.0, 3, 4.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{0, true, 'a', 1.2, 4, 4.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{0, true, 'a', 1.2, 3, 0.5f, 5L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((S7{0, true, 'a', 1.2, 3, 4.5f, 6L}) != (S7{0, true, 'a', 1.2, 3, 4.5f, 5L})));

  EXPECT_FALSE(((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}) != (S8{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));

  EXPECT_FALSE(((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}) != (S9{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));

  EXPECT_FALSE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}) != (S10{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
}

TEST(Macro, QI_GENERATE_REGULAR_OP_LESS_EQUAL)
{
  // 0
  EXPECT_TRUE((S0{} <= S0{}));

  // 1
  EXPECT_TRUE((S1{0} <= S1{0}));
  EXPECT_TRUE(( S1{0} <= S1{1}));
  EXPECT_FALSE(( S1{1} <= S1{0}));

  // 2
  EXPECT_TRUE((S2{0, true}) <= (S2{0, true}));
  EXPECT_TRUE( (S2{0, true}) <= (S2{1, true}));
  EXPECT_TRUE( (S2{0, false}) <= (S2{0, true}));
  EXPECT_FALSE( (S2{1, true}) <= (S2{0, true}));
  EXPECT_FALSE( (S2{0, true}) <= (S2{0, false}));

  // TODO: test others member numbers.
}

TEST(Macro, QI_GENERATE_REGULAR_OP_GREATER)
{
  // 0
  EXPECT_FALSE((S0{} > S0{}));

  // 1
  EXPECT_FALSE((S1{0} > S1{0}));
  EXPECT_FALSE(( S1{0} > S1{1}));
  EXPECT_TRUE(( S1{1} > S1{0}));

  // 2
  EXPECT_FALSE((S2{0, true}) > (S2{0, true}));
  EXPECT_FALSE( (S2{0, true}) > (S2{1, true}));
  EXPECT_FALSE( (S2{0, false}) > (S2{0, true}));
  EXPECT_TRUE( (S2{1, true}) > (S2{0, true}));
  EXPECT_TRUE( (S2{0, true}) > (S2{0, false}));

  // TODO: test others member numbers.
}

TEST(Macro, QI_GENERATE_REGULAR_OP_GREATER_OR_EQUAL)
{
  // 0
  EXPECT_TRUE((S0{} >= S0{}));

  // 1
  EXPECT_TRUE((S1{0} >= S1{0}));
  EXPECT_FALSE(( S1{0} >= S1{1}));
  EXPECT_TRUE(( S1{1} >= S1{0}));

  // 2
  EXPECT_TRUE((S2{0, true}) >= (S2{0, true}));
  EXPECT_FALSE( (S2{0, true}) >= (S2{1, true}));
  EXPECT_FALSE( (S2{0, false}) >= (S2{0, true}));
  EXPECT_TRUE( (S2{1, true}) >= (S2{0, true}));
  EXPECT_TRUE( (S2{0, true}) >= (S2{0, false}));

  // TODO: test others member numbers.
}
