/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#include <iostream>
#include <qi/ptruid.hpp>
#include <qi/uuid.hpp>

namespace qi
{
  PtrUid::PtrUid(const Uuid& machineUuid, const Uuid& processUuid, const void* ptr)
  {
    static_assert(sizeof(uint8_t) == 1, "uint8_t must have a size of 1 "
                                        "for the array size computation to be valid.");
    uint8_t buffer[sizeof(Uuid) + sizeof(Uuid) + sizeof(ptr)];

    auto it = std::copy(machineUuid.begin(), machineUuid.end(), buffer);
    it = std::copy(processUuid.begin(), processUuid.end(), it);

    const auto uptr = reinterpret_cast<uintptr_t>(ptr);
    auto b = reinterpret_cast<const uint8_t*>(&uptr);
    auto e = reinterpret_cast<const uint8_t*>(&uptr + 1);
    it = std::copy(b, e, it);

    // This sha1 object is defined in the detail namespace of boost, which is not
    // great because it could be removed in a further version. An alternative would
    // be to write a wrapper over the openssl implementation. For now, just use
    // the boost version, as it is an implementation detail and could be
    // easily changed.
    boost::uuids::detail::sha1 sha1;
    sha1.process_block(buffer, buffer + sizeof(buffer));
    sha1.get_digest(digest);
  }

  std::ostream& operator<<(std::ostream& o, const PtrUid& uid)
  {
    static const char* digits = "0123456789ABCDEF";
    for (auto byte: uid)
    {
      o << digits[(byte >> 4) & 0x0F] << digits[byte & 0x0F];
    }
    return o;
  }
} // qi
