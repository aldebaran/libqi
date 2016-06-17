#ifndef QI_TEST_RELATIONS
#define QI_TEST_RELATIONS

/// @file
/// Relations used in unit tests.
/// A relation is a homogeneous binary predicate, i.e. a function
/// with the signature bool (T, T).
///
/// A complete explanation can be found in relationpredicate.hpp.
/// The properties we're interested in are, amongst others, reflexivity, symmetry and transitivity.

namespace test
{
  enum Person : int
  {
    personBegin = 0,
    abe = 0,
    james = 1,
    phil = 2,
    bob = 3,
    jim = 4,
    joe = 5,
    jack = 6,
    jay = 7,
    ben = 8,
    personEnd = 9
  };

  inline Person& operator++(Person& p)
  {
    p = static_cast<Person>(p + 1);
    return p;
  }

  // abe is the father of james, james is the father of bob, joe and jack.
  // phil is the father of ben.
  //       abe
  //        |
  //      james       phil       ?
  //     /  |  \       |        / \_
  //  bob  joe jack   ben     jim jay

  // Not reflexive, not symmetric, not transitive.
  struct IsParent
  {
    inline bool operator()(Person a, Person b) const
    {
      // To know if 'a' is the parent of 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[personEnd][personEnd] =
      { //             abe   james phil  bob   jim   joe   jack  jay  ben
        /* abe   */ {  0,    1,    0,    0,    0,    0,    0,    0,   0},
        /* james */ {  0,    0,    0,    1,    0,    1,    1,    0,   0},
        /* phil  */ {  0,    0,    0,    0,    0,    0,    0,    0,   1},
        /* bob   */ {  0,    0,    0,    0,    0,    0,    0,    0,   0},
        /* jim   */ {  0,    0,    0,    0,    0,    0,    0,    0,   0},
        /* joe   */ {  0,    0,    0,    0,    0,    0,    0,    0,   0},
        /* jack  */ {  0,    0,    0,    0,    0,    0,    0,    0,   0},
        /* jay   */ {  0,    0,    0,    0,    0,    0,    0,    0,   0},
        /* ben   */ {  0,    0,    0,    0,    0,    0,    0,    0,   0}
      };
      return data[a][b];
    }
  };

  // Not reflexive, symmetric, not transitive.
  struct IsBrother
  {
    inline bool operator()(Person a, Person b) const
    {
      // bob, joe and jack are brothers.
      // jim and jay are brothers.
      // ben has no brother.
      // To know if 'a' is the brother of 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[personEnd][personEnd] =
      { //             abe   james phil  bob   jim   joe   jack  jay   ben
        /* abe   */ {  0,    0,    0,    0,    0,    0,    0,    0,    0},
        /* james */ {  0,    0,    0,    0,    0,    0,    0,    0,    0},
        /* phil  */ {  0,    0,    0,    0,    0,    0,    0,    0,    0},
        /* bob   */ {  0,    0,    0,    0,    0,    1,    1,    0,    0},
        /* jim   */ {  0,    0,    0,    0,    0,    0,    0,    1,    0},
        /* joe   */ {  0,    0,    0,    1,    0,    0,    1,    0,    0},
        /* jack  */ {  0,    0,    0,    1,    0,    1,    0,    0,    0},
        /* jay   */ {  0,    0,    0,    0,    1,    0,    0,    0,    0},
        /* ben   */ {  0,    0,    0,    0,    0,    0,    0,    0,    0}
      };
      return data[a][b];
    }
  };

  // Reflexive, symmetric, transitive.
  struct ShareAParent
  {
    inline bool operator()(Person a, Person b) const
    {
      // To know if 'a' shares a parent with 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[personEnd][personEnd] =
      { //             abe   james phil  bob   jim   joe   jack  jay   ben
        /* abe   */ {  1,    0,    0,    0,    0,    0,    0,    0,    0},
        /* james */ {  0,    1,    0,    0,    0,    0,    0,    0,    0},
        /* phil  */ {  0,    0,    1,    0,    0,    0,    0,    0,    0},
        /* bob   */ {  0,    0,    0,    1,    0,    1,    1,    0,    0},
        /* jim   */ {  0,    0,    0,    0,    1,    0,    0,    1,    0},
        /* joe   */ {  0,    0,    0,    1,    0,    1,    1,    0,    0},
        /* jack  */ {  0,    0,    0,    1,    0,    1,    1,    0,    0},
        /* jay   */ {  0,    0,    0,    0,    1,    0,    0,    1,    0},
        /* ben   */ {  0,    0,    0,    0,    0,    0,    0,    0,    1}
      };
      return data[a][b];
    }
  };

  // Reflexive, not symmetric, not transitive.
  struct KnowNameOf
  {
    inline bool operator()(Person a, Person b) const
    {
      // To know if 'a' knows the name of 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[personEnd][personEnd] =
      { //             abe   james phil  bob   jim   joe   jack  jay  ben
        /* abe   */ {  1,    1,    0,    1,    0,    1,    0,    0,   0}, // (abe forgot the name of his grandson jack)
        /* james */ {  1,    1,    0,    1,    0,    1,    1,    0,   0},
        /* phil  */ {  0,    0,    1,    0,    0,    0,    0,    0,   1},
        /* bob   */ {  1,    1,    0,    1,    0,    1,    1,    0,   0},
        /* jim   */ {  0,    0,    0,    0,    1,    0,    0,    1,   0}, // (does not know the name of ben)
        /* joe   */ {  1,    1,    0,    1,    0,    1,    1,    0,   0},
        /* jack  */ {  1,    1,    0,    1,    0,    1,    1,    0,   0},
        /* jay   */ {  0,    0,    0,    0,    1,    0,    0,    1,   0},
        /* ben   */ {  0,    0,    1,    0,    1,    0,    0,    0,   1}  // (knows the name of jim)
      };
      return data[a][b];
    }
  };

  enum Entity : int
  {
    entityBegin = 0,
    a = 0,
    b = 1,
    c = 2,
    d = 3,
    entityEnd = 4
  };

  inline Entity& operator++(Entity& e)
  {
    e = static_cast<Entity>(e + 1);
    return e;
  }

  struct NotReflexiveNotSymmetricNotTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "parent of" 'b', 'b' "parent of" 'c', 'b' "parent of" 'd'
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 0, 1, 0, 0},
        /* b */ { 0, 0, 1, 1},
        /* c */ { 0, 0, 0, 0},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct NotReflexiveSymmetricNotTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "brother of" 'b', 'b' "brother of" 'c'
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 0, 1, 1, 0},
        /* b */ { 1, 0, 1, 0},
        /* c */ { 1, 1, 0, 0},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct ReflexiveSymmetricNotTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 1, 0, 1, 0},
        /* b */ { 0, 1, 0, 0},
        /* c */ { 1, 0, 1, 1},
        /* d */ { 0, 0, 1, 1}
      };
      return data[a][b];
    }
  };

  struct ReflexiveSymmetricTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "shares a parent with" 'b', 'b' "shares a parent with" 'c'
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 1, 1, 1, 0},
        /* b */ { 1, 1, 1, 0},
        /* c */ { 1, 1, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct ReflexiveNotSymmetricNotTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "knows name of" 'b', 'b' "knows name of" 'c', 'b' "knows name of" 'a'
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 1, 1, 0, 0},
        /* b */ { 1, 1, 1, 0},
        /* c */ { 0, 0, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct ReflexiveNotSymmetricTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 1, 1, 1, 0},
        /* b */ { 0, 1, 0, 0},
        /* c */ { 0, 1, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct NotReflexiveSymmetricTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 0, 0, 0, 0},
        /* b */ { 0, 0, 0, 0},
        /* c */ { 0, 0, 0, 0},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct NotReflexiveNotSymmetricTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 0, 1, 1, 1},
        /* b */ { 0, 0, 1, 1},
        /* c */ { 0, 0, 0, 1},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct NotTrichotomicNotTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 1, 0, 1, 0},
        /* b */ { 0, 0, 0, 0},
        /* c */ { 0, 1, 0, 0},
        /* d */ { 1, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct TrichotomicNotTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 0, 1, 1, 0},
        /* b */ { 0, 0, 0, 1},
        /* c */ { 0, 1, 0, 0},
        /* d */ { 1, 0, 1, 0}
      };
      return data[a][b];
    }
  };

  struct NotTrichotomicTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 1, 1, 1, 0},
        /* b */ { 1, 1, 1, 0},
        /* c */ { 1, 1, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct TrichotomicTransitive
  {
    inline bool operator()(Entity a, Entity b) const
    {
      // See other relations to know how to read the grid.
      static const bool data[entityEnd][entityEnd] =
      { //        a  b  c  d
        /* a */ { 0, 1, 1, 1},
        /* b */ { 0, 0, 1, 1},
        /* c */ { 0, 0, 0, 1},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

} // namespace qi

#endif // QI_TEST_RELATIONS
