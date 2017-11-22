/*
**  Copyright (C) 2017 Softbank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_DETAIL_SHA1_HPP
#define QI_DETAIL_SHA1_HPP
#pragma once

#include <algorithm>
#include <boost/utility/string_ref.hpp>
#include <boost/uuid/sha1.hpp>

#include <qi/assert.hpp>

namespace qi {
namespace detail {

  /** Digest (hash) value given by SHA1 algorithm.

      This type is Regular and Linearizable.
      It can be constructed using any data in which case it's value
      will be the processed value after sha1 applied.

      Use example:

          std::string someDataA = acquireData();
          std::string someDataB = acquireData();
          SHA1Digest digestA{ someDataA };
          SHA1Digest digestB{ someDataB };

          if(digestA == digestB)
          {
            throw std::runtime_error("Same Data");
            return;
          }

          // assuming std::map<SHA1Digest, std::string> valuesIndex;
          valuesIndex.emplace(digestA, someDataA);
          valuesIndex.emplace(digestB, someDataB);

  */
  struct SHA1Digest
  {
    static const int size = 5;
    unsigned int value[size];

    /// Construct with value zero.
    SHA1Digest()
      : value{}  // force default-initialization <=> zero-initialization for int
    {}

    SHA1Digest(const SHA1Digest&) = default;
    SHA1Digest& operator=(const SHA1Digest&) = default;

    /** Construct by generating the value using SHA1 based on the provided data.

        @param data     Non-null non-empty pointer to valid readable data.
        @param dataSize Size in byte of the data to read.
    */
    SHA1Digest(const void* data, size_t dataSize)
    {
      QI_ASSERT_TRUE(data);
      boost::uuids::detail::sha1 s;
      s.process_bytes(data, dataSize);
      s.get_digest(value);
    }

    /** Construct by generating the value using SHA1 based on the provided string.

        @param data     Non-empty reference to string data.
    */
    explicit SHA1Digest(boost::string_ref data)
      : SHA1Digest(data.data(), data.size())
    {}

    /** Construct by copying the value provided in the provided array.

        @param newValues Values to copy. Must be valid as a SHA1 value.
    */
    // TODO: replace the array type by an std::array_view/span or equivalent once available.
    SHA1Digest(const std::array<unsigned int, size>& newValues)
    {
      std::copy(newValues.begin(), newValues.end(), std::begin(value));
    }

  // Regular:
    friend bool operator==(const SHA1Digest& left, const SHA1Digest& right)
    {
      return left.value[0] == right.value[0]
          && left.value[1] == right.value[1]
          && left.value[2] == right.value[2]
          && left.value[3] == right.value[3]
          && left.value[4] == right.value[4];
    }

    friend bool operator<(const SHA1Digest& left, const SHA1Digest& right)
    {
      using std::begin;
      using std::end;
      return std::lexicographical_compare(begin(left.value), end(left.value),
                                          begin(right.value), end(right.value));
    }

    QI_GENERATE_FRIEND_REGULAR_DERIVED_OPS(SHA1Digest)
  // Linearizable<unsigned int>:
    auto begin() const -> decltype(std::begin(value)) // TODO: C++14 replace return type by decltype(auto)
    {
      return std::begin(value);
    }

    auto end() const -> decltype(std::end(value)) // TODO: C++14 replace return type by decltype(auto)
    {
      return std::end(value);
    }

    auto begin() -> decltype(std::begin(value)) // TODO: C++14 replace return type by decltype(auto)
    {
      return std::begin(value);
    }

    auto end() -> decltype(std::end(value)) // TODO: C++14 replace return type by decltype(auto)
    {
      return std::end(value);
    }

  };

}}

#endif // QI_DETAIL_SHA1_HPP
