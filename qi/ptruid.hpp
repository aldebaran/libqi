#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QI_PTRUID_HPP
#define QI_PTRUID_HPP

#include <iosfwd>
#include <boost/config.hpp> // for BOOST_CONSTEXPR, et al.
#include <boost/range/algorithm/lexicographical_compare.hpp>
#include <qi/uuid.hpp>
#include <qi/api.hpp>
#include <qi/assert.hpp>

namespace qi
{
  /// Allow to uniquely identify a pointer in the universe.
  /// It is in fact a sha1 hash on the concatenation of :
  /// - the machine id (an uuid)
  /// - the process uuid (also an uuid)
  /// - the pointer value (the address itself, not the pointed value)
  /// The process part is an uuid, and not the process id, to avoid that on a single machine
  /// two different processes with the same process id (after a loop in the process ids) cause
  /// different pointers to have the same PtrUid.
  ///
  /// A PtrUid models the concepts Sequence and Hashable (i.e. std::hash<PtrUid> is defined).
  ///
  /// If QI_PTRUID_DEBUG is defined, to ease debugging the first number of the digest is
  /// deterministic inside a process (static counter starting at 0).
  class QI_API PtrUid
  {
    static const size_t uint32Count = 5;
    uint32_t digest[uint32Count] = {};

    template<typename T>
    static uint8_t* toUInt8Ptr(T* t)
    {
      return reinterpret_cast<uint8_t*>(t);
    }

    template<typename T>
    static const uint8_t* toUInt8Ptr(const T* t)
    {
      return reinterpret_cast<const uint8_t*>(t);
    }

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
      return boost::range::lexicographical_compare(a.digest, b.digest);
    }

    // Streamable:
    friend QI_API std::ostream& operator<<(std::ostream& o, const PtrUid& uid);

    // Sequence:
    friend uint8_t* begin(PtrUid& a) BOOST_NOEXCEPT
    {
      return toUInt8Ptr(beginUInt32(a));
    }

    friend uint8_t* end(PtrUid& a) BOOST_NOEXCEPT
    {
      return toUInt8Ptr(endUInt32(a));
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

    /// Return a const ref to allow the address of the element to be taken.
    const uint8_t& operator[](size_t n) const BOOST_NOEXCEPT
    {
      QI_ASSERT(n < size(*this));
      return begin(*this)[n];
    }

    // Custom:
    friend uint32_t* beginUInt32(PtrUid& a)
    {
      return a.digest;
    }

    friend const uint32_t* beginUInt32(const PtrUid& a)
    {
      return beginUInt32(const_cast<PtrUid&>(a)); // const_cast is safe here.
    }

    friend uint32_t* endUInt32(PtrUid& a)
    {
      return a.digest + uint32Count;
    }

    friend const uint32_t* endUInt32(const PtrUid& a)
    {
      return endUInt32(const_cast<PtrUid&>(a)); // const_cast is safe here.
    }
  };

} // namespace qi

// Hashable:
namespace std
{
  template<>
  struct hash<qi::PtrUid>
  {
    std::size_t operator()(const qi::PtrUid& p) const
    {
      static_assert(sizeof(std::size_t) >= sizeof(*beginUInt32(p)),
        "A std::size_t bit length must be large enough to store a uint32_t.");
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
