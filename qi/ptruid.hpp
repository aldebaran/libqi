#pragma once
#ifndef QI_PTRUID_HPP
#define QI_PTRUID_HPP

#include <array>
#include <iosfwd>
#include <numeric>
#include <boost/config.hpp> // for BOOST_CONSTEXPR, et al.
#include <boost/functional/hash.hpp>
#include <qi/uuid.hpp>
#include <qi/api.hpp>
#include <qi/assert.hpp>
#include <ka/macroregular.hpp>

namespace qi
{
  /// Allows to uniquely identify a pointer in the universe.
  ///
  /// It is in fact a sha1 hash on the concatenation of:
  /// - the machine id (an uuid)
  /// - the process uuid (also an uuid)
  /// - the pointer value (the address itself, not the pointed value)
  ///
  /// The process part is an uuid instead of the process id, to avoid that on a
  /// single machine two different processes with the same process id (after a
  /// loop in the process ids) cause different pointers to have the same `PtrUid`.
  ///
  /// A `PtrUid` models the concepts Sequence and Hashable
  /// (i.e. std::hash<PtrUid> is defined).
  ///
  /// If QI_PTRUID_DEBUG is defined, to ease debugging the first number of the
  /// digest is deterministic inside a process (static counter starting at 0).
  class QI_API PtrUid
  {
    using Digest = std::array<uint32_t, 5>;
    using U32Iterator = Digest::iterator;
    using U32ConstIterator = Digest::const_iterator;

    Digest digest;

    template<typename T>
    static uint8_t* toUInt8Ptr(T* a)
    {
      return reinterpret_cast<uint8_t*>(a);
    }

    template<typename T>
    static const uint8_t* toUInt8Ptr(const T* a)
    {
      return reinterpret_cast<const uint8_t*>(a);
    }

  public:
    explicit PtrUid(const Uuid& machineUuid, const Uuid& processUuid, const void* ptr);

  // Regular: copy, assignment and destruction by default
    PtrUid() : digest()
    {
    }

    KA_GENERATE_FRIEND_REGULAR_OPS_1(PtrUid, digest)

  // OStreamable:
    friend QI_API std::ostream& operator<<(std::ostream& o, const PtrUid& uid);

  // Sequence:
    friend uint8_t* begin(PtrUid& a) BOOST_NOEXCEPT
    {
      return toUInt8Ptr(&*beginUInt32(a));
    }

    friend uint8_t* end(PtrUid& a) BOOST_NOEXCEPT
    {
      return begin(a) + size(a);
    }

    friend const uint8_t* begin(const PtrUid& a) BOOST_NOEXCEPT
    {
      return begin(const_cast<PtrUid&>(a)); // const_cast is safe here.
    }

    friend const uint8_t* end(const PtrUid& a) BOOST_NOEXCEPT
    {
      return end(const_cast<PtrUid&>(a)); // const_cast is safe here.
    }

    friend BOOST_CONSTEXPR size_t size(const PtrUid&) BOOST_NOEXCEPT
    {
      return sizeof(Digest) / sizeof(uint8_t);
    }

    friend BOOST_CONSTEXPR bool empty(const PtrUid&) BOOST_NOEXCEPT
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
    friend U32Iterator beginUInt32(PtrUid& a) BOOST_NOEXCEPT
    {
      return begin(a.digest);
    }

    friend U32Iterator endUInt32(PtrUid& a) BOOST_NOEXCEPT
    {
      return end(a.digest);
    }

    friend U32ConstIterator beginUInt32(const PtrUid& a) BOOST_NOEXCEPT
    {
      return begin(a.digest);
    }

    friend U32ConstIterator endUInt32(const PtrUid& a) BOOST_NOEXCEPT
    {
      return end(a.digest);
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
        "A std::size_t must be large enough to store a uint32_t.");
      return boost::hash_range(beginUInt32(p), endUInt32(p));
    }
  };

} // namespace std

#endif
