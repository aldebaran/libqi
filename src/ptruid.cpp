/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#include <array>
#include <atomic>
#include <iterator>
#include <iostream>
#include <boost/algorithm/hex.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/core/ignore_unused.hpp>
#include <qi/ptruid.hpp>
#include <ka/sha1.hpp>
#include <qi/uuid.hpp>

namespace qi
{
  template<size_t N>
  static void staticAssertUuidIsByteArray(uint8_t (Uuid::*)[N])
  {
    static_assert(sizeof(Uuid) == N, "Uuid must be a byte array.");
  }

  PtrUid::PtrUid(const Uuid& machineUuid, const Uuid& processUuid, const void* ptr)
  {
    static_assert(sizeof(uint8_t) == 1, "uint8_t must have a size of 1 "
                                        "for the array size computation to be valid.");
    staticAssertUuidIsByteArray(&Uuid::data);
    std::array<uint8_t,
        sizeof(Uuid) // machine uuid
      + sizeof(Uuid) // process uuid
      + sizeof(ptr)> // pointer
        buffer;

    const auto uptr = reinterpret_cast<std::uintptr_t>(ptr);

    // Copy machine uuid, then process uuid, then pointer.
    const auto it = std::copy(toUInt8Ptr(&uptr), toUInt8Ptr(&uptr + 1),
      boost::copy(processUuid,
        boost::copy(machineUuid, begin(buffer))
      )
    );
    boost::ignore_unused(it);

    // The array size computation use the assumption that
    // for any Uuid u, u.end() - u.begin() == sizeof(UUid).
    // Assert that it's really the case.
    QI_ASSERT(static_cast<std::size_t>(it - begin(buffer)) == buffer.size());

    boost::copy(ka::sha1(buffer), begin(*this));
#ifdef QI_PTRUID_DEBUG
    // When debugging, the fact that PtrUids have different values at each execution is
    // not desirable (the values are different because the process uuid are different).
    // Therefore, for debugging purpose make the first number of the digest deterministic.
    static std::atomic<uint32_t> id{0};
    digest[0] = id++;
#endif
  }

  std::ostream& operator<<(std::ostream& o, const PtrUid& uid)
  {
    boost::algorithm::hex(begin(uid), end(uid), std::ostream_iterator<uint8_t>(o));
    return o;
  }

} // qi

