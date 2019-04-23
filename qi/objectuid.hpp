#pragma once
#ifndef QI_OBJECTUID_HPP
#define QI_OBJECTUID_HPP

#include <qi/ptruid.hpp>

namespace qi
{
  /// Unique identifier of an object being referred to by a qi::Object instance.
  ///
  /// @warning Users: your code SHALL NOT assume that ObjectUid will always
  /// be implemented as an alias to PtrUid.
  /// We only guarantee that it is Regular, i.e. it has value semantics.
  /// See ka/concept.hpp for a complete definition of Regular.
  /// The definition of ObjectUid may be changed in the future.
  using ObjectUid = PtrUid;

} // namespace qi

#endif
