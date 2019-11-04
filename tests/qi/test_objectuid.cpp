/*
** Copyright (C) 2010, 2019 Aldebaran Robotics
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <string>

#include <ka/opt.hpp>
#include <ka/empty.hpp>
#include <qi/objectuid.hpp>

namespace {
  const qi::ObjectUid& dummyValidObjectUid()
  {
    static auto dummy = []() {
      qi::ObjectUid u;
      std::fill(begin(u), end(u), static_cast<uint8_t>('z'));
      return u;
    }();
    return dummy;
  }

  const std::string& dummyValidObjectUidString()
  {
    static auto const uid = dummyValidObjectUid();
    static auto x = std::string(begin(uid), end(uid));
    return x;
  }
}

TEST(ObjectUid, SerializationEmptyStringGivesNoUid)
{
  const auto maybeUid = qi::deserializeObjectUid("");
  ASSERT_FALSE(maybeUid);
}

TEST(ObjectUid, SerializationInvalidStringGivesNoUid)
{
  const auto maybeUid = qi::deserializeObjectUid("abcd");
  ASSERT_FALSE(maybeUid);
}

TEST(ObjectUid, SerializationValidStringGivesUid)
{
  const auto maybeUid = qi::deserializeObjectUid(dummyValidObjectUidString());
  ASSERT_TRUE(maybeUid);
}

namespace {
  struct Value {
    template<typename T>
    typename T::value_type operator()(T const& t) const {
      return t.value();
    }
  };
} // namespace

TEST(ObjectUid, SerializationDeserializationIsSymmetric)
{
  using ka::functional_ops::operator*; // Mathematical function composition (right to left).
  auto const serialize = qi::serializeObjectUid<std::string>;
  auto const deserialize = Value{} * qi::deserializeObjectUid<std::string>;

  auto const id0 = serialize * deserialize; // string -> string
  auto const id1 = deserialize * serialize; // ObjectUid -> ObjectUid
  EXPECT_EQ(dummyValidObjectUidString(), id0(dummyValidObjectUidString()));
  EXPECT_EQ(dummyValidObjectUid(), id1(dummyValidObjectUid()));
}
