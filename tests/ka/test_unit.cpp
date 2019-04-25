#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/unit.hpp>

TEST(Unit, Regular) {
  using namespace ka;
  ASSERT_TRUE(is_regular({unit, unit, unit}));
}
