/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/ptruid.hpp>
#include <qi/uuid.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <array>
#include <atomic>
#include <iterator>
#include <iostream>
#include <boost/algorithm/hex.hpp>

namespace qi
{
  template<size_t N>
  static void staticAssertUuidSize(uint8_t (Uuid::*)[N])
  {
    static_assert(sizeof(Uuid) == N, "Uuid must only contain a byte array.");
  }

  PtrUid::PtrUid(const Uuid& machineUuid, const Uuid& processUuid, const void* ptr)
  {
    static_assert(sizeof(uint8_t) == 1, "uint8_t must have a size of 1 "
                                        "for the array size computation to be valid.");
    staticAssertUuidSize(&Uuid::data);
    std::array<uint8_t, sizeof(Uuid) + sizeof(Uuid) + sizeof(ptr)> buffer;

    auto it = boost::copy(machineUuid, begin(buffer));
    it = boost::copy(processUuid, it);

    const auto uptr = reinterpret_cast<uintptr_t>(ptr);
    it = std::copy(toUInt8Ptr(&uptr), toUInt8Ptr(&uptr + 1), it);

    // The array size computation use the assumption that
    // for any Uuid u, u.end() - u.begin() == sizeof(UUid).
    // Assert that it's really the case.
    QI_ASSERT(it - begin(buffer) == buffer.size());

    // This sha1 object is defined in the detail namespace of boost. This is
    // suboptimal because it could be removed in a further version. An alternative
    // would be to write a wrapper over the openssl implementation. For now, just
    // use the boost version, as it is an implementation detail and could be
    // easily changed.
    boost::uuids::detail::sha1 sha1;
    sha1.process_block(buffer.data(), buffer.data() + buffer.size());
    sha1.get_digest(digest);
#ifdef QI_PTRUID_DEBUG
    // When debugging, the fact that PtrUids have different values at each execution is
    // not desirable (the values are different because the process uuid are different).
    // Therefore, for debugging purpose make the first number of the digest deterministic.
    static std::atomic<int> id{0};
    digest[0] = id++;
#endif
  }

  std::ostream& operator<<(std::ostream& o, const PtrUid& uid)
  {
    boost::algorithm::hex(begin(uid), end(uid), std::ostream_iterator<uint8_t>(o));
    return o;
  }
} // qi
