#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QI_PTRUID_HPP
#define QI_PTRUID_HPP

#include <iosfwd>
#include <boost/config.hpp> // for BOOST_CONSTEXPR, et al.
#include <qi/uuid.hpp>
#include <qi/api.hpp>
#include <qi/assert.hpp>

namespace qi
{
  /// Allow to uniquely identify a pointer in the universe.
  /// It is in fact a sha1 hash on the concatenation of :
  /// - the machine id (an uuid)
  /// - the process uuid (also an uuid)
  /// - the pointer
  /// The process part is an uuid, and not the process id, to avoid that on a single machine
  /// two different processes with the same process id (after a loop in the process ids) cause
  /// different pointers to have the same PtrUid.
  class QI_API PtrUid
  {
    static const size_t nbUint32 = 5;
    uint32_t digest[nbUint32];
  public:
    explicit PtrUid(const Uuid& machineUuid, const Uuid& processUuid, const void* ptr);

    // Regular: copy, assignment and destruction by default
    PtrUid() = default;

    friend inline bool operator==(const PtrUid& a, const PtrUid& b) BOOST_NOEXCEPT
    {
      // std::equal could be used but the loop was unrolled for efficiency.
      return a.digest[0] == b.digest[0]
          && a.digest[1] == b.digest[1]
          && a.digest[2] == b.digest[2]
          && a.digest[3] == b.digest[3]
          && a.digest[4] == b.digest[4];
    }

    friend inline bool operator!=(const PtrUid& a, const PtrUid& b) BOOST_NOEXCEPT
    {
      return !(a == b);
    }

    friend inline bool operator<(const PtrUid& a, const PtrUid& b) BOOST_NOEXCEPT
    {
      return std::lexicographical_compare(a.digest, a.digest + nbUint32, b.digest, b.digest + nbUint32);
    }

    // Streamable:
    friend QI_API std::ostream& operator<<(std::ostream& o, const PtrUid& uid);

    // Sequence:
    friend uint8_t* begin(PtrUid& a) BOOST_NOEXCEPT
    {
      return reinterpret_cast<uint8_t*>(a.digest);
    }

    friend uint8_t* end(PtrUid& a) BOOST_NOEXCEPT
    {
      return reinterpret_cast<uint8_t*>(a.digest + a.nbUint32);
    }

    friend const uint8_t* begin(const PtrUid& a) BOOST_NOEXCEPT
    {
      return begin(const_cast<PtrUid&>(a)); // const_cast is safe here.
    }

    friend const uint8_t* end(const PtrUid& a) BOOST_NOEXCEPT
    {
      return end(const_cast<PtrUid&>(a)); // const_cast is safe here.
    }

    friend size_t size(const PtrUid& a) BOOST_NOEXCEPT
    {
      return end(a) - begin(a);
    }

    friend BOOST_CONSTEXPR bool empty(const PtrUid& a) BOOST_NOEXCEPT
    {
      return false;
    }

    /// The return is a const ref to allow to take the address of the element.
    const uint8_t& operator[](size_t n) const BOOST_NOEXCEPT
    {
      QI_ASSERT(n < size(*this));
      return begin(*this)[n];
    }

    // Custom:
    friend const uint32_t* beginUInt32(const PtrUid& p)
    {
      return p.digest;
    }
    friend const uint32_t* endUInt32(const PtrUid& p)
    {
      return p.digest + nbUint32;
    }
  };

} // namespace qi

namespace std
{
  template<>
  struct hash<qi::PtrUid>
  {
    std::size_t operator()(const qi::PtrUid& p) const
    {
      static_assert(sizeof(std::size_t) >= sizeof(*beginUInt32(p)), "A std::size_t bit length must be large enough to store a uint32_t.");
      std::size_t hash{0};
      auto b = beginUInt32(p), e = endUInt32(p);
      while (b != e) {
          hash += (std::size_t)*b;
          ++b;
      }
      return hash;
    }
  };

} // namespace std

#endif
