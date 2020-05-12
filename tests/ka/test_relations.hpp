#ifndef KA_TEST_RELATIONS
#define KA_TEST_RELATIONS

/// @file
/// Relations used in unit tests.
/// A relation is a homogeneous binary predicate, i.e. a function
/// with the signature bool (T, T).
///
/// A complete explanation can be found in relationpredicate.hpp.
/// The properties we're interested in are, amongst others, reflexivity,
/// symmetry and transitivity.

namespace test_relation {
  enum person_t : int {
    person_begin = 0,
    abe = 0,
    james = 1,
    phil = 2,
    bob = 3,
    jim = 4,
    joe = 5,
    jack = 6,
    jay = 7,
    ben = 8,
    person_end = 9
  };

  inline person_t& operator++(person_t& p) {
    p = static_cast<person_t>(p + 1);
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
  struct is_parent_t {
    inline bool operator()(person_t a, person_t b) const {
      // To know if 'a' is the parent of 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[person_end][person_end] = {
        //             abe   james phil  bob   jim   joe   jack  jay  ben
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
  struct is_brother_t {
    inline bool operator()(person_t a, person_t b) const {
      // bob, joe and jack are brothers.
      // jim and jay are brothers.
      // ben has no brother.
      // To know if 'a' is the brother of 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[person_end][person_end] = {
        //             abe   james phil  bob   jim   joe   jack  jay   ben
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
  struct share_a_parent_t {
    inline bool operator()(person_t a, person_t b) const {
      // To know if 'a' shares a parent with 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[person_end][person_end] = {
        //             abe   james phil  bob   jim   joe   jack  jay   ben
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
  struct know_name_of_t {
    inline bool operator()(person_t a, person_t b) const {
      // To know if 'a' knows the name of 'b':
      // go to the line of 'a' and look at the value in the column of 'b'.
      static const bool data[person_end][person_end] = {
        //             abe   james phil  bob   jim   joe   jack  jay  ben
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

  enum entity_t : int {
    entity_begin = 0,
    a = 0,
    b = 1,
    c = 2,
    d = 3,
    entity_end = 4
  };

  inline entity_t& operator++(entity_t& e) {
    e = static_cast<entity_t>(e + 1);
    return e;
  }

  struct not_reflexive_not_symmetric_not_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "parent of" 'b', 'b' "parent of" 'c', 'b' "parent of" 'd'
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 0, 1, 0, 0},
        /* b */ { 0, 0, 1, 1},
        /* c */ { 0, 0, 0, 0},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct not_reflexive_symmetric_not_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "brother of" 'b', 'b' "brother of" 'c'
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 0, 1, 1, 0},
        /* b */ { 1, 0, 1, 0},
        /* c */ { 1, 1, 0, 0},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct reflexive_symmetric_not_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 1, 0, 1, 0},
        /* b */ { 0, 1, 0, 0},
        /* c */ { 1, 0, 1, 1},
        /* d */ { 0, 0, 1, 1}
      };
      return data[a][b];
    }
  };

  struct reflexive_symmetric_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "shares a parent with" 'b', 'b' "shares a parent with" 'c'
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 1, 1, 1, 0},
        /* b */ { 1, 1, 1, 0},
        /* c */ { 1, 1, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct reflexive_not_symmetric_not_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      // Example of interpretation: 'a' "knows name of" 'b', 'b' "knows name of" 'c', 'b' "knows name of" 'a'
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 1, 1, 0, 0},
        /* b */ { 1, 1, 1, 0},
        /* c */ { 0, 0, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct reflexive_not_symmetric_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 1, 1, 1, 0},
        /* b */ { 0, 1, 0, 0},
        /* c */ { 0, 1, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct not_reflexive_symmetric_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 0, 0, 0, 0},
        /* b */ { 0, 0, 0, 0},
        /* c */ { 0, 0, 0, 0},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct not_reflexive_not_symmetric_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 0, 1, 1, 1},
        /* b */ { 0, 0, 1, 1},
        /* c */ { 0, 0, 0, 1},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct not_trichotomic_not_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 1, 0, 1, 0},
        /* b */ { 0, 0, 0, 0},
        /* c */ { 0, 1, 0, 0},
        /* d */ { 1, 0, 0, 0}
      };
      return data[a][b];
    }
  };

  struct trichotomic_not_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 0, 1, 1, 0},
        /* b */ { 0, 0, 0, 1},
        /* c */ { 0, 1, 0, 0},
        /* d */ { 1, 0, 1, 0}
      };
      return data[a][b];
    }
  };

  struct not_trichotomic_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 1, 1, 1, 0},
        /* b */ { 1, 1, 1, 0},
        /* c */ { 1, 1, 1, 0},
        /* d */ { 0, 0, 0, 1}
      };
      return data[a][b];
    }
  };

  struct trichotomic_transitive_t {
    inline bool operator()(entity_t a, entity_t b) const {
      // See other relations to know how to read the grid.
      static const bool data[entity_end][entity_end] = {
        //        a  b  c  d
        /* a */ { 0, 1, 1, 1},
        /* b */ { 0, 0, 1, 1},
        /* c */ { 0, 0, 0, 1},
        /* d */ { 0, 0, 0, 0}
      };
      return data[a][b];
    }
  };

} // namespace test_relation

#endif // KA_TEST_RELATIONS
