#ifndef QI_TEST_OBJECTIO_HPP
#define QI_TEST_OBJECTIO_HPP
#include <iostream>
#include <iomanip>

namespace qi
{
  // This is a dummy extraction operator for unit tests only.
  template<typename T>
  std::ostream& operator<<(std::ostream& o, const Object<T>& x)
  {
    return x
      ? o << "Object at " << std::hex << &x
      : o << "null Object";
  }
} // namespace qi

#endif // QI_TEST_OBJECTIO_HPP
