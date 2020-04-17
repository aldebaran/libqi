#include <gtest/gtest.h>
#include <ka/zero.hpp>

// `zero_t` is uninstantiable. Therefore, these tests are for documentation
// purpose only, and cannot compile.
TEST(Zero, Basic) {
  using namespace ka;
  // zero_t z;
  // zero_t{}; // Would compile with `zero_t() = delete`.

  // Still easy to form references/pointers...
  // zero_t const& z0 = *(zero_t*)nullptr;
  // zero_t const& z1 = *(zero_t*)nullptr;
  // auto b = z0 == z1; // ... but methods must not compile.
}
