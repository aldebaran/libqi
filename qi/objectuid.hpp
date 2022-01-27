#pragma once
#ifndef QI_OBJECTUID_HPP
#define QI_OBJECTUID_HPP

#include <boost/optional.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>
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

  /// Deserializes an ObjectUid from a range of bytes.
  /// @returns An ObjectUid or none if the range has the wrong size.
  ///
  /// Post-condition (where result == deserializeObjectUid(r)):
  ///   (std::distance(begin(r), end(r)) == size(uid)) == result.has_value()
  ///
  /// Invariant: boost::range::equal(serializeObjectUid(*deserializeObjectUid(r)), r)
  ///   provided deserializeObjectUid(r).has_value()
  ///
  /// Linearizable<T> R, where T is implicitly convertible to uint8_t
  template<typename R>
  boost::optional<ObjectUid> deserializeObjectUid(const R& r)
  {
    using std::begin;
    using std::end;
    ObjectUid uid;
    using It = ka::Decay<decltype(begin(r))>;
    using DiffType = typename std::iterator_traits<It>::difference_type;
    if (std::distance(begin(r), end(r)) == static_cast<DiffType>(size(uid)))
    {
      std::copy(begin(r), end(r), begin(uid));
      return uid;
    }
    return {};
  }

  /// Serializes an ObjectUid into container (in a non human-readable way).
  /// @remark This is useful for storing an ObjectUid in an arbitrary container.
  ///         Do not use this function with string for printing for human readers.
  ///         To print an ObjectUid's content, use `operator<<` instead.
  ///
  /// Requires SequenceContainer<T>
  /// @returns A T object initialized with the ObjectUid's data.
  ///
  /// Invariant: *deserializeObjectUid(serializeObjectUid(x)) == x
  template<typename T>
  T serializeObjectUid(const ObjectUid& uid)
  {
    return T(begin(uid), end(uid));
  }

} // namespace qi

#endif
