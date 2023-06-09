#ifndef KA_MACROREGULAR_HPP
#define KA_MACROREGULAR_HPP
#pragma once

/// @file
/// Contains various macros to help user generate
/// regular relational operators (==, <, !=, >, <=, >=).
///
/// Meaning of regularity
/// =============================================
/// Regular means that:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   == is an equivalence
///   < is a total ordering (see below)
///   != is always the opposite of ==
///   a > b is equivalent to b < a
///   a <= b is equivalent to !(b < a)
///   a >= b is equivalent to !(a < b)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// See conceptpredicate.hpp for a formal definition.
///
/// Meaning of total ordering
/// =============================================
/// A total ordering is a relation that imposes an order on values.
/// Given any values a, b, only one of these hold:
/// - a is before b
/// - b is before a
/// - a is equal to b
/// (this is called the trichotomy law)
///
/// Furthermore, a total ordering is transitive:
/// if a is before b and b is before c, a is before c.
///
/// See relationpredicate.hpp for a formal definition.
///
/// Macros' purpose
/// =============================================
/// Macros are provided to generate only one operator or to define
/// bundles of operators, so that the user has a fine-grained control
/// over the generation.
/// For a motivation for why using macros, see the comment of
/// KA_GENERATE_REGULAR_OP_EQUAL_0.

/// The KA_GENERATE_MEMBERWISE_EQUALITY_n macros family generate
/// memberwise equality with the form:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// (instanceA.member0 == instanceB.member0 && instanceA.member1 == instanceB.member1 && ...)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// The macro parameters are the 2 instances' names followed by the members' names
/// (the number at the end of the macro name indicates the number of members).
/// These macros are helpers to define KA_GENERATE_FRIEND_OP_EQUALITY_n.
#define KA_GENERATE_MEMBERWISE_EQUALITY_0(a, b) \
  true

#define KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m0) \
  (a.m0 == b.m0)

#define KA_GENERATE_MEMBERWISE_EQUALITY_2(a, b, m0, m1) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m0)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m1))

#define KA_GENERATE_MEMBERWISE_EQUALITY_3(a, b, m0, m1, m2) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_2(a, b, m0, m1)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m2))

#define KA_GENERATE_MEMBERWISE_EQUALITY_4(a, b, m0, m1, m2, m3) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_3(a, b, m0, m1, m2)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m3))

#define KA_GENERATE_MEMBERWISE_EQUALITY_5(a, b, m0, m1, m2, m3, m4) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_4(a, b, m0, m1, m2, m3)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m4))

#define KA_GENERATE_MEMBERWISE_EQUALITY_6(a, b, m0, m1, m2, m3, m4, m5) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_5(a, b, m0, m1, m2, m3, m4)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m5))

#define KA_GENERATE_MEMBERWISE_EQUALITY_7(a, b, m0, m1, m2, m3, m4, m5, m6) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_6(a, b, m0, m1, m2, m3, m4, m5)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m6))

#define KA_GENERATE_MEMBERWISE_EQUALITY_8(a, b, m0, m1, m2, m3, m4, m5, m6, m7) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_7(a, b, m0, m1, m2, m3, m4, m5, m6)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m7))

#define KA_GENERATE_MEMBERWISE_EQUALITY_9(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_8(a, b, m0, m1, m2, m3, m4, m5, m6, m7)        \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m8))

#define KA_GENERATE_MEMBERWISE_EQUALITY_10(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9) \
    (KA_GENERATE_MEMBERWISE_EQUALITY_9(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8)         \
  && KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m9))


/// Equality is central to generic value-semantic programming, so we need an
/// easy way to generate it. It would be nice if the compiler could generate
/// operator== with memberwise equality, but it is not currently possible.
/// The KA_GENERATE_REGULAR_OP_EQUAL_n family of macros implement such a
/// memberwise equality.
///
/// The typical use is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// struct MyStruct {
///   X x;
///   Y y;
///   // ...
/// public:
///   friend KA_GENERATE_REGULAR_OP_EQUAL_2(MyStruct, x, y)
///   // ...
/// };
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// The macro expands to:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   friend bool operator==(MyStruct const& a, MyStruct const& b) {
///     return a.x == b.x && a.y == b.y;
///   }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// The general form of the generated body is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// (a.m0 == b.m0 && a.m1 == b.m1 && ...)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// These macros can be used inside a class by prefixing them with 'friend'.
///
/// Why using macros to define equality? There exist alternatives but they have
/// their own drawbacks (e.g [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern),
/// by relying on public inheritance effectively implements a "is-a" relation, which is totally not
/// what we're trying to achieve here).
///
/// In the current state of the language (C++11), macros seem the least bad solution.
#define KA_GENERATE_REGULAR_OP_EQUAL_0(type)        \
  bool operator==(type const&, type const&) {       \
    return KA_GENERATE_MEMBERWISE_EQUALITY_0(a, b); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_1(type, m0)        \
  bool operator==(type const& a, type const& b) {       \
    return KA_GENERATE_MEMBERWISE_EQUALITY_1(a, b, m0); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_2(type, m0, m1)        \
  bool operator==(type const& a, type const& b) {           \
    return KA_GENERATE_MEMBERWISE_EQUALITY_2(a, b, m0, m1); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_3(type, m0, m1, m2)        \
  bool operator==(type const& a, type const& b) {               \
    return KA_GENERATE_MEMBERWISE_EQUALITY_3(a, b, m0, m1, m2); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_4(type, m0, m1, m2, m3)        \
  bool operator==(type const& a, type const& b) {                   \
    return KA_GENERATE_MEMBERWISE_EQUALITY_4(a, b, m0, m1, m2, m3); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_5(type, m0, m1, m2, m3, m4)        \
  bool operator==(type const& a, type const& b)  {                      \
    return KA_GENERATE_MEMBERWISE_EQUALITY_5(a, b, m0, m1, m2, m3, m4); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_6(type, m0, m1, m2, m3, m4, m5)        \
  bool operator==(type const& a, type const& b) {                           \
    return KA_GENERATE_MEMBERWISE_EQUALITY_6(a, b, m0, m1, m2, m3, m4, m5); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_7(type, m0, m1, m2, m3, m4, m5, m6)        \
  bool operator==(type const& a, type const& b) {                               \
    return KA_GENERATE_MEMBERWISE_EQUALITY_7(a, b, m0, m1, m2, m3, m4, m5, m6); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_8(type, m0, m1, m2, m3, m4, m5, m6, m7)        \
  bool operator==(type const& a, type const& b) {                                   \
    return KA_GENERATE_MEMBERWISE_EQUALITY_8(a, b, m0, m1, m2, m3, m4, m5, m6, m7); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8)        \
  bool operator==(type const& a, type const& b) {                                       \
    return KA_GENERATE_MEMBERWISE_EQUALITY_9(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8); \
  }

#define KA_GENERATE_REGULAR_OP_EQUAL_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)       \
  bool operator==(type const& a, type const& b) {                                           \
    return KA_GENERATE_MEMBERWISE_EQUALITY_10(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9);\
  }

/// Helper for the KA_GENERATE_LEXICOGRAPHICAL_LESS_n macros family.
#define KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m0, m1) \
  (!(b.m0 < a.m0) && a.m1 < b.m1)

/// Helper for the KA_GENERATE_LEXICOGRAPHICAL_LESS_n macros family.
#define KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1, or_condition) \
  (!(b.m0 < a.m0) && (a.m1 < b.m1 || (or_condition)))

/// The KA_GENERATE_LEXICOGRAPHICAL_LESS_n macros family generate
/// total ordering expressions.
/// See KA_GENERATE_FRIEND_OP_LESS_0
#define KA_GENERATE_LEXICOGRAPHICAL_LESS_0(a, b) \
  false

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_1(a, b, m0) \
  (a.m0 < b.m0)

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_2(a, b, m0, m1)  \
  (a.m0 < b.m0                                            \
    || KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m0, m1))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_3(a, b, m0, m1, m2)  \
  (a.m0 < b.m0                                                \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,        \
        KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m1, m2)))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_4(a, b, m0, m1, m2, m3)  \
  (a.m0 < b.m0                                                    \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,            \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,           \
          KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m2, m3))))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_5(a, b, m0, m1, m2, m3, m4)  \
  (a.m0 < b.m0                                                        \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,                \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,               \
          KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m2, m3,             \
            KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m3, m4)))))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_6(a, b, m0, m1, m2, m3, m4, m5)  \
  (a.m0 < b.m0                                                            \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,                    \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,                   \
          KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m2, m3,                 \
            KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m3, m4,               \
              KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m4, m5))))))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_7(a, b, m0, m1, m2, m3, m4, m5, m6)  \
  (a.m0 < b.m0                                                                \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,                        \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,                       \
          KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m2, m3,                     \
            KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m3, m4,                   \
              KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m4, m5,                 \
                KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m5, m6)))))))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_8(a, b, m0, m1, m2, m3, m4, m5, m6, m7)  \
  (a.m0 < b.m0                                                                    \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,                            \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,                           \
          KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m2, m3,                         \
            KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m3, m4,                       \
              KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m4, m5,                     \
                KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m5, m6,                   \
                  KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m6, m7))))))))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_9(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8)  \
  (a.m0 < b.m0                                                                        \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,                                \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,                               \
          KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m2, m3,                             \
            KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m3, m4,                           \
              KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m4, m5,                         \
                KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m5, m6,                       \
                  KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m6, m7,                     \
                    KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m7, m8)))))))))

#define KA_GENERATE_LEXICOGRAPHICAL_LESS_10(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9) \
  (a.m0 < b.m0                                                                            \
    || KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m0, m1,                                    \
        KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m1, m2,                                   \
          KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m2, m3,                                 \
            KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m3, m4,                               \
              KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m4, m5,                             \
                KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m5, m6,                           \
                  KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m6, m7,                         \
                    KA_FIRST_EQUAL_AND_SECOND_LESS_OR(a, b, m7, m8,                       \
                      KA_FIRST_EQUAL_AND_SECOND_LESS(a, b, m8, m9))))))))))


/// Ordering is important in generic programming by allowing elements to be sorted.
/// The KA_GENERATE_REGULAR_OP_LESS_n macros family generate operator< as a
/// total ordering (see relationpredicates.hpp for a formal definition).
///
/// The typical use is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// struct MyStruct {
///   X x;
///   Y y;
///   // ...
/// public:
///   friend KA_GENERATE_REGULAR_OP_LESS_2(MyStruct, x, y)
///   // ...
/// };
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// The macro expands to:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   friend bool operator<(MyStruct const& a, MyStruct const& b) {
///     return a.x < b.x || (!(b.x < a.x) && a.y < b.y);
///   }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// The general form of the generated body is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// a.m0 < b.m0 ||
///   (!(b.m0 < a.m0) && (a.m1 < b.m1 ||
///     (!(b.m1 < a.m1) && (b.m2 < a.m2 ||
///       ...))))
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// These macros can be used inside a class by prefixing them with 'friend'.
/// See KA_GENERATE_MEMBERWISE_EQUAL_0 comment for a motivation for using macros.
#define KA_GENERATE_REGULAR_OP_LESS_0(type)          \
  bool operator<(type const&, type const&) {         \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_0(a, b); \
  }

#define KA_GENERATE_REGULAR_OP_LESS_1(type, m0)          \
  bool operator<(type const& a, type const& b) {         \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_1(a, b, m0); \
  }

#define KA_GENERATE_REGULAR_OP_LESS_2(type, m0, m1)           \
  bool operator<(type const& a, type const& b) {              \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_2(a, b, m0, m1);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_3(type, m0, m1, m2)           \
  bool operator<(type const& a, type const& b) {                  \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_3(a, b, m0, m1, m2);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_4(type, m0, m1, m2, m3)           \
  bool operator<(type const& a, type const& b) {                      \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_4(a, b, m0, m1, m2, m3);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_5(type, m0, m1, m2, m3, m4)           \
  bool operator<(type const& a, type const& b) {                          \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_5(a, b, m0, m1, m2, m3, m4);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_6(type, m0, m1, m2, m3, m4, m5)           \
  bool operator<(type const& a, type const& b) {                              \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_6(a, b, m0, m1, m2, m3, m4, m5);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_7(type, m0, m1, m2, m3, m4, m5, m6)           \
  bool operator<(type const& a, type const& b) {                                  \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_7(a, b, m0, m1, m2, m3, m4, m5, m6);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_8(type, m0, m1, m2, m3, m4, m5, m6, m7)           \
  bool operator<(type const& a, type const& b) {                                      \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_8(a, b, m0, m1, m2, m3, m4, m5, m6, m7);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8)           \
  bool operator<(type const& a, type const& b) {                                          \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_9(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8);  \
  }

#define KA_GENERATE_REGULAR_OP_LESS_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)          \
  bool operator<(type const& a, type const& b) {                                              \
    return KA_GENERATE_LEXICOGRAPHICAL_LESS_10(a, b, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9); \
  }


/// The KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_n family of macros
/// generate operator== and operator< in a regular way.
/// Must be used in a class because of friend declarations.
/// Equal and less are bundled in isolation from other relational ops
/// (!=, >, <=, >=) because the user might want to implement these other ones
/// via namespace-level template functions.
#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_0(type) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_0(type)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_0(type)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_1(type, m0) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_1(type, m0)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_1(type, m0)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_2(type, m0, m1) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_2(type, m0, m1)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_2(type, m0, m1)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_3(type, m0, m1, m2) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_3(type, m0, m1, m2)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_3(type, m0, m1, m2)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_4(type, m0, m1, m2, m3) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_4(type, m0, m1, m2, m3)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_4(type, m0, m1, m2, m3)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_5(type, m0, m1, m2, m3, m4) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_5(type, m0, m1, m2, m3, m4)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_5(type, m0, m1, m2, m3, m4)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_6(type, m0, m1, m2, m3, m4, m5) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_6(type, m0, m1, m2, m3, m4, m5)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_6(type, m0, m1, m2, m3, m4, m5)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_7(type, m0, m1, m2, m3, m4, m5, m6) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_7(type, m0, m1, m2, m3, m4, m5, m6)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_7(type, m0, m1, m2, m3, m4, m5, m6)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_8(type, m0, m1, m2, m3, m4, m5, m6, m7) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_8(type, m0, m1, m2, m3, m4, m5, m6, m7)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_8(type, m0, m1, m2, m3, m4, m5, m6, m7)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8) \
  friend KA_GENERATE_REGULAR_OP_EQUAL_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8)                   \
  friend KA_GENERATE_REGULAR_OP_LESS_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8)

#define KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)  \
  friend KA_GENERATE_REGULAR_OP_EQUAL_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)                    \
  friend KA_GENERATE_REGULAR_OP_LESS_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)


/// Generate != in terms of ==.
#define KA_GENERATE_REGULAR_OP_DIFFERENT(type)    \
  bool operator!=(type const& a, type const& b) { \
    return !(a == b);                             \
  }

/// Generate > in terms of <.
#define KA_GENERATE_REGULAR_OP_GREATER(type)     \
  bool operator>(type const& a, type const& b) { \
    return b < a;                                \
  }

/// Generate <= in terms of <.
#define KA_GENERATE_REGULAR_OP_LESS_OR_EQUAL(type) \
  bool operator<=(type const& a, type const& b) {  \
    return !(b < a);                               \
  }

/// Generate >= in terms of <.
#define KA_GENERATE_REGULAR_OP_GREATER_OR_EQUAL(type) \
  bool operator>=(type const& a, type const& b) {     \
    return !(a < b);                                  \
  }

/// Generate operators != in terms of ==, and >, <=, >= in terms of <.
/// Set 'prefix' parameter to 'friend' if you want to generate in a class,
/// otherwise leave it empty.
#define KA_GENERATE_REGULAR_DERIVED_OPS_PREFIX(type, prefix)  \
  prefix KA_GENERATE_REGULAR_OP_DIFFERENT(type)               \
  prefix KA_GENERATE_REGULAR_OP_GREATER(type)                 \
  prefix KA_GENERATE_REGULAR_OP_LESS_OR_EQUAL(type)           \
  prefix KA_GENERATE_REGULAR_OP_GREATER_OR_EQUAL(type)

/// Generate all relational 'derived' operators (!=, >, <=, >=) in a class.
/// See KA_GENERATE_REGULAR_DERIVED_OPS_PREFIX
#define KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)   \
  KA_GENERATE_REGULAR_DERIVED_OPS_PREFIX(type, friend)


/// The KA_GENERATE_FRIEND_REGULAR_OPS_n macros family define all
/// relational operators (==, !=, <, >, <=, >=) as friend free functions.
/// Must be used in a class.
#define KA_GENERATE_FRIEND_REGULAR_OPS_0(type)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_0(type) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_1(type, m0)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_1(type, m0) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_2(type, m0, m1)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_2(type, m0, m1) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_3(type, m0, m1, m2)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_3(type, m0, m1, m2) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_4(type, m0, m1, m2, m3)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_4(type, m0, m1, m2, m3) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_5(type, m0, m1, m2, m3, m4)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_5(type, m0, m1, m2, m3, m4) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_6(type, m0, m1, m2, m3, m4, m5)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_6(type, m0, m1, m2, m3, m4, m5) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_7(type, m0, m1, m2, m3, m4, m5, m6)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_7(type, m0, m1, m2, m3, m4, m5, m6) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_8(type, m0, m1, m2, m3, m4, m5, m6, m7)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_8(type, m0, m1, m2, m3, m4, m5, m6, m7) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8)            \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_9(type, m0, m1, m2, m3, m4, m5, m6, m7, m8) \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

#define KA_GENERATE_FRIEND_REGULAR_OPS_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)             \
  KA_GENERATE_FRIEND_REGULAR_OP_EQUAL_AND_OP_LESS_10(type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)  \
  KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(type)

/// Defines a Regular type without any implicit conversion, and whose state is
/// an integer.
///
/// Such a type is useful in testing generic code to ensure there is no typing
/// mistake. On the opposite, builtin types (`int`, `bool`, `char`, etc.) are
/// poor candidates for such a task, due to their implicit conversions.
///
/// Warning: This type is not an archetype for Regular: it exposes an integer
///   and is output streamable.
#define KA_DERIVE_REGULAR_TEST_TYPE(type)                              \
  struct type {                                                        \
    int value;                                                         \
    inline explicit type(int i = 0) : value(i) {}                      \
  /* Regular: */                                                       \
    KA_GENERATE_FRIEND_REGULAR_OPS_1(type, value)                      \
  /* OStreamable: */                                                   \
    friend std::ostream& operator<<(std::ostream& os, const type& v) { \
      return os << #type "(" << v.value << ")";                        \
    }                                                                  \
  }

#endif  // KA_MACROREGULAR_HPP
