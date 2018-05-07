#include <gtest/gtest.h>
#include <ka/macroregular.hpp>
#include <ka/conceptpredicate.hpp>

struct s0_t {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(s0_t);
};

struct s1_t {
  int a;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(s1_t, a);
};

struct s2_t {
  int a; bool b;
  KA_GENERATE_FRIEND_REGULAR_OPS_2(s2_t, a, b);
};

struct s3_t {
  int a; bool b; char c;
  KA_GENERATE_FRIEND_REGULAR_OPS_3(s3_t, a, b, c);
};

struct s4_t {
  int a; bool b; char c; double d;
  KA_GENERATE_FRIEND_REGULAR_OPS_4(s4_t, a, b, c, d);
};

struct s5_t {
  int a; bool b; char c; double d; short e;
  KA_GENERATE_FRIEND_REGULAR_OPS_5(s5_t, a, b, c, d, e);
};

struct s6_t {
  int a; bool b; char c; double d; short e; float f;
  KA_GENERATE_FRIEND_REGULAR_OPS_6(s6_t, a, b, c, d, e, f);
};

struct s7_t {
  int a; bool b; char c; double d; short e; float f; long g;
  KA_GENERATE_FRIEND_REGULAR_OPS_7(s7_t, a, b, c, d, e, f, g);
};

struct s8_t {
  int a; bool b; char c; double d; short e; float f; long g; unsigned char h;
  KA_GENERATE_FRIEND_REGULAR_OPS_8(s8_t, a, b, c, d, e, f, g, h);
};

struct s9_t {
  int a; bool b; char c; double d; short e; float f; long g; unsigned char h; unsigned int i;
  KA_GENERATE_FRIEND_REGULAR_OPS_9(s9_t, a, b, c, d, e, f, g, h, i);
};

struct s10_t {
  int a; bool b; char c; double d; short e; float f; long g; unsigned char h; unsigned int i; unsigned short j;
  KA_GENERATE_FRIEND_REGULAR_OPS_10(s10_t, a, b, c, d, e, f, g, h, i, j);
};

TEST(Macro, KA_GENERATE_MEMBERWISE_EQUALITY) {
  // To be exhaustive, we should test all same/different argument permutations, but this should be ok.
  // 0
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_0(s0_t{}, s0_t{}));

  // 1
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_1(s1_t{0}, s1_t{0}, a));
  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_1(s1_t{0}, s1_t{1}, a));

  // 2
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_2( (s2_t{0, true}), (s2_t{0, true}), a, b));
  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_2((s2_t{1, true}), (s2_t{0, true}), a, b));
  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_2((s2_t{0, false}), (s2_t{0, true}), a, b));

  // 3
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_3( (s3_t{0, true, 'a'}),
                                                 (s3_t{0, true, 'a'}), a, b, c));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_3((s3_t{1, true, 'a'}),
                                                 (s3_t{0, true, 'a'}), a, b, c));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_3((s3_t{0, false, 'a'}),
                                                 (s3_t{0, true, 'a'}), a, b, c));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_3((s3_t{0, true, 'b'}),
                                                 (s3_t{0, true, 'a'}), a, b, c));

  // 4
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_4( (s4_t{0, true, 'a', 1.2}),
                                                 (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_4((s4_t{1, true, 'a', 1.2}),
                                                 (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_4((s4_t{0, false, 'a', 1.2}),
                                                 (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_4((s4_t{0, true, 'b', 1.2}),
                                                 (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_4((s4_t{0, true, 'a', 1.3}), (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  // 5
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_5( (s5_t{0, true, 'a', 1.2, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_5((s5_t{1, true, 'a', 1.2, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_5((s5_t{0, false, 'a', 1.2, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_5((s5_t{0, true, 'b', 1.2, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_5((s5_t{0, true, 'a', 1.0, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_5((s5_t{0, true, 'a', 1.2, 4}), (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  // 6
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_6( (s6_t{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_6((s6_t{1, true, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_6((s6_t{0, false, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_6((s6_t{0, true, 'b', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_6((s6_t{0, true, 'a', 1.0, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_6((s6_t{0, true, 'a', 1.2, 4, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_6((s6_t{0, true, 'a', 1.2, 4, 0.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 4, 4.5f}), a, b, c, d, e, f));

  // 7
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_7( (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{1, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{0, false, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{0, true, 'b', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{0, true, 'a', 1.0, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{0, true, 'a', 1.2, 4, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{0, true, 'a', 1.2, 3, 0.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_7((s7_t{0, true, 'a', 1.2, 3, 4.5f, 6L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  // 8
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_8( (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

 // 9
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_9( (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  // 10
  EXPECT_TRUE(KA_GENERATE_MEMBERWISE_EQUALITY_10( (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_FALSE(KA_GENERATE_MEMBERWISE_EQUALITY_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));
}

TEST(Macro, KA_GENERATE_REGULAR_OP_EQUAL) {
  // To be exhaustive, we should test all same/different argument permutations,
  // but this should be ok.
  EXPECT_TRUE((s0_t{} == s0_t{}));

  EXPECT_TRUE((s1_t{0} == s1_t{0}));
  EXPECT_FALSE((s1_t{0} == s1_t{1}));

  EXPECT_TRUE(((s2_t{0, true}) == (s2_t{0, true})));
  EXPECT_FALSE(((s2_t{1, true}) == (s2_t{0, true})));
  EXPECT_FALSE(((s2_t{0, false}) == (s2_t{0, true})));

  EXPECT_TRUE(((s3_t{0, true, 'a'}) == (s3_t{0, true, 'a'})));
  EXPECT_FALSE(((s3_t{1, true, 'a'}) == (s3_t{0, true, 'a'})));
  EXPECT_FALSE(((s3_t{0, false, 'a'}) == (s3_t{0, true, 'a'})));
  EXPECT_FALSE(((s3_t{0, true, 'b'}) == (s3_t{0, true, 'a'})));

  EXPECT_TRUE(((s4_t{0, true, 'a', 1.2}) == (s4_t{0, true, 'a', 1.2})));
  EXPECT_FALSE(((s4_t{1, true, 'a', 1.2}) == (s4_t{0, true, 'a', 1.2})));
  EXPECT_FALSE(((s4_t{0, false, 'a', 1.2}) == (s4_t{0, true, 'a', 1.2})));
  EXPECT_FALSE(((s4_t{0, true, 'b', 1.2}) == (s4_t{0, true, 'a', 1.2})));
  EXPECT_FALSE(((s4_t{0, true, 'a', 1.3}) == (s4_t{0, true, 'a', 1.2})));

  EXPECT_TRUE(((s5_t{0, true, 'a', 1.2, 3}) == (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((s5_t{1, true, 'a', 1.2, 3}) == (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((s5_t{0, false, 'a', 1.2, 3}) == (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((s5_t{0, true, 'b', 1.2, 3}) == (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((s5_t{0, true, 'a', 1.0, 3}) == (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_FALSE(((s5_t{0, true, 'a', 1.2, 4}) == (s5_t{0, true, 'a', 1.2, 3})));

  EXPECT_TRUE(((s6_t{0, true, 'a', 1.2, 3, 4.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((s6_t{1, true, 'a', 1.2, 3, 4.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((s6_t{0, false, 'a', 1.2, 3, 4.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((s6_t{0, true, 'b', 1.2, 3, 4.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((s6_t{0, true, 'a', 1.0, 3, 4.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((s6_t{0, true, 'a', 1.2, 4, 4.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_FALSE(((s6_t{0, true, 'a', 1.2, 4, 0.5f}) == (s6_t{0, true, 'a', 1.2, 3, 4.5f})));

  EXPECT_TRUE(((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{1, true, 'a', 1.2, 3, 4.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{0, false, 'a', 1.2, 3, 4.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{0, true, 'b', 1.2, 3, 4.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{0, true, 'a', 1.0, 3, 4.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{0, true, 'a', 1.2, 4, 4.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{0, true, 'a', 1.2, 3, 0.5f, 5L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_FALSE(((s7_t{0, true, 'a', 1.2, 3, 4.5f, 6L}) == (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));

  EXPECT_TRUE(((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_FALSE(((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}) == (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));

  EXPECT_TRUE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_FALSE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}) == (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}) == (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
}

TEST(Macro, KA_GENERATE_REGULAR_OP_EQUAL_Equivalence) {
  // TODO: Add more values, especially for types with a great number of members.
  using namespace ka;
  {
    std::equal_to<s0_t> equ;
    auto values = {s0_t{}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s1_t> equ;
    auto values = {s1_t{0}, s1_t{1}, s1_t{2}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s2_t> equ;
    auto values = {s2_t{0, true},
                   s2_t{0, false},
                   s2_t{1, true}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s3_t> equ;
    auto values = {s3_t{0, true, 'a'},
                   s3_t{0, true, 'b'},
                   s3_t{1, true, 'a'}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s4_t> equ;
    auto values = {s4_t{0, true, 'a', 1.2},
                   s4_t{0, true, 'b', 1.2},
                   s4_t{1, true, 'a', 1.0}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s5_t> equ;
    auto values = {s5_t{0, true, 'a', 1.2, 5},
                   s5_t{0, true, 'a', 1.2, 6},
                   s5_t{1, true, 'a', 1.0, 5}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s6_t> equ;
    auto values = {s6_t{0, true, 'a', 1.2, 5, 2.f},
                   s6_t{0, true, 'a', 1.2, 6, 1.f},
                   s6_t{1, true, 'a', 1.0, 5, 2.f}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s7_t> equ;
    auto values = {s7_t{0, true, 'a', 1.2, 5, 2.f, 1L},
                   s7_t{0, true, 'a', 1.2, 6, 1.f, 0L},
                   s7_t{1, true, 'a', 1.0, 5, 2.f, 1L}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s8_t> equ;
    auto values = {s8_t{0, true, 'a', 1.2, 5, 2.f, 1L, 3},
                   s8_t{0, true, 'a', 1.2, 6, 1.f, 0L, 2},
                   s8_t{1, true, 'a', 1.0, 5, 2.f, 1L, 3}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s9_t> equ;
    auto values = {s9_t{0, true, 'a', 1.2, 5, 2.f, 1L, 3, 4U},
                   s9_t{0, true, 'a', 1.2, 6, 1.f, 0L, 2, 3U},
                   s9_t{1, true, 'a', 1.0, 5, 2.f, 1L, 3, 4U}};
    EXPECT_TRUE(is_equivalence(equ, values));
  } {
    std::equal_to<s10_t> equ;
    auto values = {s10_t{0, true, 'a', 1.2, 5, 2.f, 1L, 3, 4U, 4},
                   s10_t{0, true, 'a', 1.2, 6, 1.f, 0L, 2, 3U, 5},
                   s10_t{1, true, 'a', 1.0, 5, 2.f, 1L, 3, 4U, 5}};
    EXPECT_TRUE(is_equivalence(equ, values));
  }
}

TEST(Macro, KA_GENERATE_LEXICOGRAPHICAL_LESS) {
  // To be exhaustive, we should test all same/different argument permutations,
  // but this should be ok.
  // 0
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_0(s0_t{}, s0_t{}));

  // 1
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_1(s1_t{0}, s1_t{0}, a));
  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_1( s1_t{0}, s1_t{1}, a));

  // 2
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_2((s2_t{0, true}), (s2_t{0, true}), a, b));
  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_2( (s2_t{0, true}), (s2_t{1, true}), a, b));
  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_2( (s2_t{0, false}), (s2_t{0, true}), a, b));

  // 3
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_3((s3_t{0, true, 'a'}),
                                                  (s3_t{0, true, 'a'}), a, b, c));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_3((s3_t{0, true, 'a'}),
                                                 (s3_t{1, true, 'a'}), a, b, c));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_3((s3_t{0, false, 'a'}),
                                                 (s3_t{0, true, 'a'}), a, b, c));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_3((s3_t{0, true, 'a'}),
                                                 (s3_t{0, true, 'b'}), a, b, c));

  // 4
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_4((s4_t{0, true, 'a', 1.2}),
                                                  (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_4((s4_t{0, true, 'a', 1.2}),
                                                 (s4_t{1, true, 'a', 1.2}), a, b, c, d));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_4((s4_t{0, false, 'a', 1.2}),
                                                 (s4_t{0, true, 'a', 1.2}), a, b, c, d));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_4((s4_t{0, true, 'a', 1.2}),
                                                 (s4_t{0, true, 'b', 1.2}), a, b, c, d));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_4((s4_t{0, true, 'a', 1.2}),
                                                 (s4_t{0, true, 'a', 1.3}), a, b, c, d));

  // 5
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_5((s5_t{0, true, 'a', 1.2, 3}),
                                                  (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_5((s5_t{0, true, 'a', 1.2, 3}),
                                                 (s5_t{1, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_5((s5_t{0, false, 'a', 1.2, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_5((s5_t{0, true, 'a', 1.2, 3}),
                                                 (s5_t{0, true, 'b', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_5((s5_t{0, true, 'a', 1.0, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 3}), a, b, c, d, e));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_5((s5_t{0, true, 'a', 1.2, 3}),
                                                 (s5_t{0, true, 'a', 1.2, 4}), a, b, c, d, e));

  // 6
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, true, 'a', 1.2, 3, 4.5f}),
                                                  (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{1, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, false, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'b', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, true, 'a', 1.0, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 3, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, true, 'a', 1.2, 3, 4.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 4, 4.5f}), a, b, c, d, e, f));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_6((s6_t{0, true, 'a', 1.2, 4, 0.5f}),
                                                 (s6_t{0, true, 'a', 1.2, 4, 4.5f}), a, b, c, d, e, f));

  // 7
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                  (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{1, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, false, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'b', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.0, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 4, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.2, 3, 0.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}), a, b, c, d, e, f, g));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_7((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}),
                                                 (s7_t{0, true, 'a', 1.2, 3, 4.5f, 6L}), a, b, c, d, e, f, g));

  // 8
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                  (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}), a, b, c, d, e, f, g, h));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_8((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}),
                                                 (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}), a, b, c, d, e, f, g, h));

 // 9
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                  (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}), a, b, c, d, e, f, g, h, i));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_9((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}),
                                                 (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}), a, b, c, d, e, f, g, h, i));

  // 10
  EXPECT_FALSE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                   (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}), a, b, c, d, e, f, g, h, i, j));

  EXPECT_TRUE(KA_GENERATE_LEXICOGRAPHICAL_LESS_10((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}),
                                                  (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}), a, b, c, d, e, f, g, h, i, j));
}

TEST(Macro, KA_GENERATE_REGULAR_OP_LESS) {
  // To be exhaustive, we should test all same/different argument permutations,
  // but this should be ok.
  // 0
  EXPECT_FALSE((s0_t{} < s0_t{}));

  // 1
  EXPECT_FALSE((s1_t{0} < s1_t{0}));
  EXPECT_TRUE(( s1_t{0} < s1_t{1}));

  // 2
  EXPECT_FALSE((s2_t{0, true}) < (s2_t{0, true}));
  EXPECT_TRUE( (s2_t{0, true}) < (s2_t{1, true}));
  EXPECT_TRUE( (s2_t{0, false}) < (s2_t{0, true}));

  // 3
  EXPECT_FALSE((s3_t{0, true, 'a'}) < (s3_t{0, true, 'a'}));
  EXPECT_TRUE((s3_t{0, true, 'a'}) < (s3_t{1, true, 'a'}));
  EXPECT_TRUE((s3_t{0, false, 'a'}) < (s3_t{0, true, 'a'}));
  EXPECT_TRUE((s3_t{0, true, 'a'}) < (s3_t{0, true, 'b'}));

  // 4
  EXPECT_FALSE((s4_t{0, true, 'a', 1.2}) < (s4_t{0, true, 'a', 1.2}));
  EXPECT_TRUE((s4_t{0, true, 'a', 1.2}) < (s4_t{1, true, 'a', 1.2}));
  EXPECT_TRUE((s4_t{0, false, 'a', 1.2}) < (s4_t{0, true, 'a', 1.2}));
  EXPECT_TRUE((s4_t{0, true, 'a', 1.2}) < (s4_t{0, true, 'b', 1.2}));
  EXPECT_TRUE((s4_t{0, true, 'a', 1.2}) < (s4_t{0, true, 'a', 1.3}));

  // 5
  EXPECT_FALSE((s5_t{0, true, 'a', 1.2, 3}) < (s5_t{0, true, 'a', 1.2, 3}));
  EXPECT_TRUE((s5_t{0, true, 'a', 1.2, 3}) < (s5_t{1, true, 'a', 1.2, 3}));
  EXPECT_TRUE((s5_t{0, false, 'a', 1.2, 3}) < (s5_t{0, true, 'a', 1.2, 3}));
  EXPECT_TRUE((s5_t{0, true, 'a', 1.2, 3}) < (s5_t{0, true, 'b', 1.2, 3}));
  EXPECT_TRUE((s5_t{0, true, 'a', 1.0, 3}) < (s5_t{0, true, 'a', 1.2, 3}));
  EXPECT_TRUE((s5_t{0, true, 'a', 1.2, 3}) < (s5_t{0, true, 'a', 1.2, 4}));

  // 6
  EXPECT_FALSE((s6_t{0, true, 'a', 1.2, 3, 4.5f}) < (s6_t{0, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((s6_t{0, true, 'a', 1.2, 3, 4.5f}) < (s6_t{1, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((s6_t{0, false, 'a', 1.2, 3, 4.5f}) < (s6_t{0, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((s6_t{0, true, 'a', 1.2, 3, 4.5f}) < (s6_t{0, true, 'b', 1.2, 3, 4.5f}));
  EXPECT_TRUE((s6_t{0, true, 'a', 1.0, 3, 4.5f}) < (s6_t{0, true, 'a', 1.2, 3, 4.5f}));
  EXPECT_TRUE((s6_t{0, true, 'a', 1.2, 3, 4.5f}) < (s6_t{0, true, 'a', 1.2, 4, 4.5f}));
  EXPECT_TRUE((s6_t{0, true, 'a', 1.2, 4, 0.5f}) < (s6_t{0, true, 'a', 1.2, 4, 4.5f}));

  // 7
  EXPECT_FALSE((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
               (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (s7_t{1, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, false, 'a', 1.2, 3, 4.5f, 5L}) <
              (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (s7_t{0, true, 'b', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, true, 'a', 1.0, 3, 4.5f, 5L}) <
              (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (s7_t{0, true, 'a', 1.2, 4, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, true, 'a', 1.2, 3, 0.5f, 5L}) <
              (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}));

  EXPECT_TRUE((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) <
              (s7_t{0, true, 'a', 1.2, 3, 4.5f, 6L}));

  // 8
  EXPECT_FALSE((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
               (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (s8_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (s8_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}) <
              (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (s8_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}) <
              (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (s8_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}));

  EXPECT_TRUE((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) <
              (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}));

 // 9
  EXPECT_FALSE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
               (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}));

  EXPECT_TRUE((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) <
              (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}));

  // 10
  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
               (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}));

  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) <
              (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}));
}

TEST(Macro, KA_GENERATE_REGULAR_OP_LESS_TotalOrdering) {
  // TODO: Add more values, especially for types with a great number of members.
  using namespace ka;
  {
    std::less<s0_t> lt;
    auto values = {s0_t{}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s1_t> lt;
    auto values = {s1_t{0}, s1_t{1}, s1_t{2}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s2_t> lt;
    auto values = {s2_t{0, true},
                   s2_t{0, false},
                   s2_t{1, true}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s3_t> lt;
    auto values = {s3_t{0, true, 'a'},
                   s3_t{0, true, 'b'},
                   s3_t{1, true, 'a'}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s4_t> lt;
    auto values = {s4_t{0, true, 'a', 1.2},
                   s4_t{0, true, 'b', 1.2},
                   s4_t{1, true, 'a', 1.0}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s5_t> lt;
    auto values = {s5_t{0, true, 'a', 1.2, 5},
                   s5_t{0, true, 'a', 1.2, 6},
                   s5_t{1, true, 'a', 1.0, 5}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s6_t> lt;
    auto values = {s6_t{0, true, 'a', 1.2, 5, 2.f},
                   s6_t{0, true, 'a', 1.2, 6, 1.f},
                   s6_t{1, true, 'a', 1.0, 5, 2.f}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s7_t> lt;
    auto values = {s7_t{0, true, 'a', 1.2, 5, 2.f, 1L},
                   s7_t{0, true, 'a', 1.2, 6, 1.f, 0L},
                   s7_t{1, true, 'a', 1.0, 5, 2.f, 1L}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s8_t> lt;
    auto values = {s8_t{0, true, 'a', 1.2, 5, 2.f, 1L, 3},
                   s8_t{0, true, 'a', 1.2, 6, 1.f, 0L, 2},
                   s8_t{1, true, 'a', 1.0, 5, 2.f, 1L, 3}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s9_t> lt;
    auto values = {s9_t{0, true, 'a', 1.2, 5, 2.f, 1L, 3, 4U},
                   s9_t{0, true, 'a', 1.2, 6, 1.f, 0L, 2, 3U},
                   s9_t{1, true, 'a', 1.0, 5, 2.f, 1L, 3, 4U}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  } {
    std::less<s10_t> lt;
    auto values = {s10_t{0, true, 'a', 1.2, 5, 2.f, 1L, 3, 4U, 4},
                   s10_t{0, true, 'a', 1.2, 6, 1.f, 0L, 2, 3U, 5},
                   s10_t{1, true, 'a', 1.0, 5, 2.f, 1L, 3, 4U, 5}};
    EXPECT_TRUE(is_total_ordering(lt, values));
  }
}

TEST(Macro, KA_GENERATE_REGULAR_OP_DIFFERENT) {
  // To be exhaustive, we should test all same/different argument permutations,
  // but this should be ok.
  EXPECT_FALSE((s0_t{} != s0_t{}));

  EXPECT_FALSE((s1_t{0} != s1_t{0}));
  EXPECT_TRUE((s1_t{0} != s1_t{1}));

  EXPECT_FALSE(((s2_t{0, true}) != (s2_t{0, true})));
  EXPECT_TRUE(((s2_t{1, true}) != (s2_t{0, true})));
  EXPECT_TRUE(((s2_t{0, false}) != (s2_t{0, true})));

  EXPECT_FALSE(((s3_t{0, true, 'a'}) != (s3_t{0, true, 'a'})));
  EXPECT_TRUE(((s3_t{1, true, 'a'}) != (s3_t{0, true, 'a'})));
  EXPECT_TRUE(((s3_t{0, false, 'a'}) != (s3_t{0, true, 'a'})));
  EXPECT_TRUE(((s3_t{0, true, 'b'}) != (s3_t{0, true, 'a'})));

  EXPECT_FALSE(((s4_t{0, true, 'a', 1.2}) != (s4_t{0, true, 'a', 1.2})));
  EXPECT_TRUE(((s4_t{1, true, 'a', 1.2}) != (s4_t{0, true, 'a', 1.2})));
  EXPECT_TRUE(((s4_t{0, false, 'a', 1.2}) != (s4_t{0, true, 'a', 1.2})));
  EXPECT_TRUE(((s4_t{0, true, 'b', 1.2}) != (s4_t{0, true, 'a', 1.2})));
  EXPECT_TRUE(((s4_t{0, true, 'a', 1.3}) != (s4_t{0, true, 'a', 1.2})));

  EXPECT_FALSE(((s5_t{0, true, 'a', 1.2, 3}) != (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((s5_t{1, true, 'a', 1.2, 3}) != (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((s5_t{0, false, 'a', 1.2, 3}) != (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((s5_t{0, true, 'b', 1.2, 3}) != (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((s5_t{0, true, 'a', 1.0, 3}) != (s5_t{0, true, 'a', 1.2, 3})));
  EXPECT_TRUE(((s5_t{0, true, 'a', 1.2, 4}) != (s5_t{0, true, 'a', 1.2, 3})));

  EXPECT_FALSE(((s6_t{0, true, 'a', 1.2, 3, 4.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((s6_t{1, true, 'a', 1.2, 3, 4.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((s6_t{0, false, 'a', 1.2, 3, 4.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((s6_t{0, true, 'b', 1.2, 3, 4.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((s6_t{0, true, 'a', 1.0, 3, 4.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((s6_t{0, true, 'a', 1.2, 4, 4.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));
  EXPECT_TRUE(((s6_t{0, true, 'a', 1.2, 4, 0.5f}) != (s6_t{0, true, 'a', 1.2, 3, 4.5f})));

  EXPECT_FALSE(((s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{1, true, 'a', 1.2, 3, 4.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{0, false, 'a', 1.2, 3, 4.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{0, true, 'b', 1.2, 3, 4.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{0, true, 'a', 1.0, 3, 4.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{0, true, 'a', 1.2, 4, 4.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{0, true, 'a', 1.2, 3, 0.5f, 5L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));
  EXPECT_TRUE(((s7_t{0, true, 'a', 1.2, 3, 4.5f, 6L}) != (s7_t{0, true, 'a', 1.2, 3, 4.5f, 5L})));

  EXPECT_FALSE(((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));
  EXPECT_TRUE(((s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7}) != (s8_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6})));

  EXPECT_FALSE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));
  EXPECT_TRUE(((s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U}) != (s9_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U})));

  EXPECT_FALSE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{1, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, false, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'b', 1.2, 3, 4.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.0, 3, 4.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 4, 4.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 0.5f, 5L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 6L, 6, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 7, 2U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 3U, 8}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
  EXPECT_TRUE((s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 9}) != (s10_t{0, true, 'a', 1.2, 3, 4.5f, 5L, 6, 2U, 8}));
}

TEST(Macro, KA_GENERATE_REGULAR_OP_LESS_EQUAL) {
  // 0
  EXPECT_TRUE((s0_t{} <= s0_t{}));

  // 1
  EXPECT_TRUE((s1_t{0} <= s1_t{0}));
  EXPECT_TRUE(( s1_t{0} <= s1_t{1}));
  EXPECT_FALSE(( s1_t{1} <= s1_t{0}));

  // 2
  EXPECT_TRUE((s2_t{0, true}) <= (s2_t{0, true}));
  EXPECT_TRUE( (s2_t{0, true}) <= (s2_t{1, true}));
  EXPECT_TRUE( (s2_t{0, false}) <= (s2_t{0, true}));
  EXPECT_FALSE( (s2_t{1, true}) <= (s2_t{0, true}));
  EXPECT_FALSE( (s2_t{0, true}) <= (s2_t{0, false}));

  // TODO: test others member numbers.
}

TEST(Macro, KA_GENERATE_REGULAR_OP_GREATER) {
  // 0
  EXPECT_FALSE((s0_t{} > s0_t{}));

  // 1
  EXPECT_FALSE((s1_t{0} > s1_t{0}));
  EXPECT_FALSE(( s1_t{0} > s1_t{1}));
  EXPECT_TRUE(( s1_t{1} > s1_t{0}));

  // 2
  EXPECT_FALSE((s2_t{0, true}) > (s2_t{0, true}));
  EXPECT_FALSE( (s2_t{0, true}) > (s2_t{1, true}));
  EXPECT_FALSE( (s2_t{0, false}) > (s2_t{0, true}));
  EXPECT_TRUE( (s2_t{1, true}) > (s2_t{0, true}));
  EXPECT_TRUE( (s2_t{0, true}) > (s2_t{0, false}));

  // TODO: test others member numbers.
}

TEST(Macro, KA_GENERATE_REGULAR_OP_GREATER_OR_EQUAL) {
  // 0
  EXPECT_TRUE((s0_t{} >= s0_t{}));

  // 1
  EXPECT_TRUE((s1_t{0} >= s1_t{0}));
  EXPECT_FALSE(( s1_t{0} >= s1_t{1}));
  EXPECT_TRUE(( s1_t{1} >= s1_t{0}));

  // 2
  EXPECT_TRUE((s2_t{0, true}) >= (s2_t{0, true}));
  EXPECT_FALSE( (s2_t{0, true}) >= (s2_t{1, true}));
  EXPECT_FALSE( (s2_t{0, false}) >= (s2_t{0, true}));
  EXPECT_TRUE( (s2_t{1, true}) >= (s2_t{0, true}));
  EXPECT_TRUE( (s2_t{0, true}) >= (s2_t{0, false}));

  // TODO: test others member numbers.
}
