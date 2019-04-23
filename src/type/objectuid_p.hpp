#pragma once

#include <boost/optional.hpp>
#include <ka/utility.hpp>
#include <qi/objectuid.hpp>

namespace qi {

  /// Deserializes an ObjectUid from a range of bytes.
  /// @returns An ObjectUid or none if the range has the wrong size.
  ///
  /// Post-conditions (where empty(r) means begin(r) == end(r)):
  ///  - (empty(r) && result.empty()) or !result.empty()
  ///  - boost::range::equal(serialize(deserializeObjectUid(r)), r)
  ///
  /// ForwardRange<T> R, where T is implicitly convertible to uint8_t
  template<typename R>
  boost::optional<ObjectUid> deserializeObjectUid(const R& r)
  {
    using std::begin;
    using std::end;
    ObjectUid uid;
    if (std::distance(begin(r), end(r)) == size(uid))
    {
      std::copy(begin(r), end(r), begin(uid));
      return uid;
    }
    return {};
  }

  /// Serializes an ObjectUid into a string.
  /// @returns A string with the ObjectUid's data.
  ///
  /// Invariant: *deserializeObjectUid(serialize(x)) == x
  inline
  std::string serialize(const ObjectUid& uid)
  {
    return std::string(begin(uid), end(uid));
  }

}