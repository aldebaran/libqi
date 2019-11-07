#ifndef KA_BASE64_HPP
#define KA_BASE64_HPP
#pragma once
#include <climits>
#include <algorithm>
#include <iterator>
#include <string>
#include <boost/iterator/iterator_facade.hpp>
#include "macro.hpp"
#include "macroregular.hpp"
#include "utility.hpp"

namespace ka {

namespace detail {
  // Precondition: 0 <= n < 64
  inline constexpr
  char base64_char(int n) noexcept {
    return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[n];
  }

  // Integral N
  template<typename N> constexpr
  N base64_bits(N non_encoded_byte_count) noexcept {
    return N(8) * non_encoded_byte_count;
  }

  // Distance in bits, sextets, and whether byte-distance and sextet-distance
  // are commensurable.
  // Integral N
  template<typename N>
  struct base64_distance_t {
    N bits;
    N sextets;
    bool commensurable;
  };

  // Measures the given distance with several units.
  //
  // Precondition: (0 <= offset0 < 8) && (0 <= offset1 < 8)
  // TODO: Make constexpr when above C++11.
  // Integral N, Integral M
  template<typename N, typename M = N>
  base64_distance_t<N> measure(N distance, M offset0 = M(0), M offset1 = M(0)) {
    auto const bits = base64_bits(distance) + offset1 - offset0;
    auto const sextets = bits / N(6);
    return {bits, sextets, bits == N(6)*sextets};
  }

  // Sextet-distance corresponding to the given byte-distance, starting sextet
  // offset and final sextet offset.
  //
  // Precondition: (0 <= offset0 < 8) && (0 <= offset1 < 8)
  // TODO: Make constexpr when above C++11.
  // Integral N, Integral M
  template<typename N, typename M>
  N distance_to(N distance, M offset0, M offset1) noexcept {
    auto const m = measure(distance, offset0, offset1);
    return m.sextets + (m.commensurable ? 0 : (m.sextets >= 0 ? 1 : -1));
  }

  // Data needed to advance the iterator by the given sextets and from the given
  // sextet offset. This data is composed of the byte-distance and the sextet
  // offset.
  //
  // Precondition: 0 <= offset < 8
  // TODO: Make constexpr when above C++11.
  // Integral N, Integral M
  template<typename N, typename M>
  std::pair<N, M> advance_data(N sextets, M offset) noexcept {
    // octets:  0-------1-------2-------
    //          012345670123456701234567
    // sextets: a-----b-----c-----d-----
    // offsets: 0     6     4     2
    auto const bits = offset + 6*sextets;
    if (bits >= 0) {
      return {static_cast<N>(bits / 8), static_cast<M>(bits % 8)};
    } else {
      // Return value is of the form
      //    {f(bits), g(sextets, offset)}
      // where:
      //   f(x) = -1, for x in [-8, -1]
      //          -2, for x in [-16, -9]
      //          ...
      //   g(-1, 6) = 0
      //   g(-2, 6) = 2
      //   g(-3, 6) = 4
      //   g(-4, 6) = 6
      //   ...
      //   g's implementation relies on the fact that successive offsets from
      //   higher to lower addresses are repeatedly: 0, 2, 4, 6...
      return {static_cast<N>(((bits + 1) / 8) - 1), static_cast<M>((offset - 2*sextets) % 8)};
    }
  }
} // namespace detail

/// Iterator that transforms binary data to base64-encoded data.
///
/// This iterator has the same iterator category (forward, bidirectional, random
/// access) than its underlying byte iterator.
///
/// Note: This iterator is read-only.
///
/// Note: Constructing the end iterator requires computing the offset of the
///   final sextet through the underlying byte iterators. This computation is
///   O(1) for random access iterators and O(n) otherwise. If this final offset
///   is already known, it can be specified through the `base64_iter`
///   constructor function, instead of `base64_end` to avoid the computation.
///
/// Note: This iterator is not suitable for situations requiring (super) high
///   performances, as it handles one byte at a time, does not leverage
///   vectorized operations, etc. It is nonetheless suitable for most situations
///   (no allocation, etc.).
///
/// Note: No padding is added to allow incremental encoding (e.g. chunk by chunk
///   encoding).
///
/// Example: Appending successive base64-encoded data *without* padding.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // `b64` is a `char` container, `b` and `e` are forward iterators on `char`.
/// ...
/// b64.insert(b64.end(), base64_begin(b, e), base64_end(b, e));
/// ...
/// b64.insert(b64.end(), base64_begin(b, e), base64_end(b, e));
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Finishing base64-encoding by padding.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // `b64` is a `char` container, `b` and `e` are forward iterators on `char`.
/// ...
/// b64.insert(b64.end(), base64_begin(b, e), base64_end(b, e));
/// ...
/// std::fill_n(
///   std::back_inserter(b64), base64_padding_byte_count(b64.size()), '=');
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// The following diagram represents how binary data is cut into sextets (6-bit
/// chunks). Each sextet then corresponds to one output character. Three bytes
/// (i.e. 24 bits) yield four output characters ((3 * 8) / 6 == 4).
///
/// octets:  0-------1-------2-------
///          012345670123456701234567
/// sextets: a-----b-----c-----d-----
/// offsets: 0     6     4     2
///
/// ForwardIterator<char> I
template<typename I>
struct base64_iter_t
  : boost::iterator_facade<
      base64_iter_t<I>,
      char,
      typename std::iterator_traits<I>::iterator_category,
      char,
      typename std::iterator_traits<I>::difference_type
    >
{
  I b, e;
  int offset; // Beginning of the sextet inside the current byte.

  explicit
  base64_iter_t(I b = {}, I e = {}, int offset = {}) : b(b), e(e), offset(offset) {
    static_assert(CHAR_BIT == 8, "base64_iter_t: A char must be 8-bit long.");
  }

  char dereference() const {
    return detail::base64_char([&]() -> unsigned int {
      // Sextets a and d (see diagram above) are fully inside one byte:
      if (offset <= 2) {
        return *b >> (2 - offset);
      }
      // Sextets b and c are overlapping two bytes:
      auto const l = 8 - offset; // Bit count of the left part.
      auto const r = 6 - l;      // Bit count of the right part.
      auto const i = std::next(b);
      auto const left = *b << r; // Shift to make space for right part.
      auto const right = (i == e
        ? 0                      // At end -> right part is 0.
        : *i >> (8 - r));        // Not at end -> shift out bits not in right part.
      return left | right;       // Assemble the two parts of the sextet.
    }() & 0x3F);                 // 0x3F forces 6-bit length.
  }
  bool equal(base64_iter_t const& x) const KA_NOEXCEPT_EXPR(b == x.b) {
    return b == x.b && e == x.e && offset == x.offset;
  }
  void increment() {
    advance(1);
  }
  void decrement() {
    advance(-1);
  }
  /// Integral N
  template<typename N>
  void advance(N sextets) {
    auto d = detail::advance_data(sextets, offset);
    std::advance(b, d.first);
    offset = d.second;
  }
  auto distance_to(base64_iter_t const& x) const -> decltype(std::distance(b, x.b)) {
    auto const d = std::distance(b, x.b);
    if (d >= 0) {
      return detail::distance_to(d, offset, x.offset);
    } else {
      return -detail::distance_to(-d, x.offset, offset);
    }
  }
};

/// Contructs a base64 iterator, performing argument type deduction.
///
/// Complexity: O(1)
/// Precondition: readable_bounded_range(b, e) && 0 <= offset < 8
///
/// ForwardIterator<char> I
template<typename I>
base64_iter_t<I> base64_iter(I b, I e, int offset = 0) {
  return base64_iter_t<I>{b, e, offset};
}

/// Contructs a base64 iterator pointing to the beginning of the range.
///
/// Complexity: O(1)
/// Precondition: readable_bounded_range(b, e)
///
/// ForwardIterator<char> I
template<typename I>
base64_iter_t<I> base64_begin(I b, I e) {
  return base64_iter(b, e, 0);
}

/// Contructs a base64 iterator pointing to the end of the range.
///
/// Complexity: O(1) if RandomAccessIterator(I), O(n) otherwise
/// Precondition: readable_bounded_range(b, e)
///
/// ForwardIterator<char> I
template<typename I>
base64_iter_t<I> base64_end(I b, I e) {
  auto m = detail::measure(std::distance(b, e));
  if (!m.commensurable) ++m.sextets;
  return base64_iter(e, e, detail::advance_data(m.sextets, 0).second);
}

/// Number of padding bytes to add to a base64-encoded string that is not
/// yet padded.
///
/// Integral N
template<typename N> constexpr
N base64_padding_byte_count(N encoded_bytes) noexcept {
  // When encoding, each sextet of binary data becomes a character. When
  // decoding, characters are processed by groups of four, because each such
  // group will correspond to three decoded bytes (the least common multiple of
  // 6 and 8 is 24). Therefore, padding's goal is to make the encoded string's
  // length a multiple of four.
  return (N(4) - (encoded_bytes % N(4))) % N(4);
}

/// Number of bytes of a base64-encoded string *without* padding, computed from
/// the number of bytes of a non-encoded string.
///
/// See `base64_encoded_with_padding_byte_count` for a version with padding.
/// TODO: Make constexpr when above C++11.
///
/// Integral N
template<typename N>
N base64_encoded_byte_count(N n) noexcept {
  auto const m = detail::measure(n);
  return m.sextets + N(m.commensurable ? 0 : 1);
}

/// Number of bytes of a base64-encoded string *with* padding, computed from
/// the number of bytes of a non-encoded string.
///
/// See `base64_encoded_byte_count` for a version without padding.
/// TODO: Make constexpr when above C++11.
///
/// Integral N
template<typename N>
N base64_encoded_with_padding_byte_count(N non_encoded_byte_count) noexcept {
  return base64_encoded_byte_count(non_encoded_byte_count)
    + base64_padding_byte_count(
        base64_encoded_byte_count(non_encoded_byte_count));
}

/// Base64-encodes a range of bytes, *without* padding.
///
/// This algorithm is suitable for incremental encoding, letting the caller add
/// padding when it is finished.
///
/// See `base64_encode_with_padding` for a version with padding.
///
/// Precondition: readable_bounded_range(b, e)
///   && writable_counted_range(o, base64_encoded_byte_count(std::distance(b, e)))
///
/// ForwardIterator<char> I, OutputIterator<char> O
template<typename I, typename O>
O base64_encode(I b, I e, O o) {
  return std::copy(base64_begin(b, e), base64_end(b, e), o);
}

namespace detail {

// Output iterator that counts how many times it is incremented.
//
// OutputIterator O
template<typename O>
struct counting_output_iter_t {
  O base;
  std::size_t count; // Output iterators have incomplete difference type.
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_2(counting_output_iter_t, base, count)
// OutputIterator:
  using value_type = typename std::iterator_traits<O>::value_type;
  using pointer = typename std::iterator_traits<O>::pointer;
  using reference = typename std::iterator_traits<O>::reference;
  using difference_type = typename std::iterator_traits<O>::difference_type;
  using iterator_category = typename std::iterator_traits<O>::iterator_category;
  auto operator*() -> decltype(*base) {
    return *base;
  }
  auto operator++() -> counting_output_iter_t& {
    ++base;
    ++count;
    return *this;
  }
  auto operator++(int) -> counting_output_iter_t {
    return {base++, count++};
  }
  // TODO: Remove operator+ after upgrading from VS2015.
  auto operator+(std::size_t n) const -> counting_output_iter_t {
    return {std::next(base, n), count + n};
  }
};

// OutputIterator O
template<typename O>
counting_output_iter_t<O> counting_output_iter(O o) {
  return {o, 0};
}

} // namespace detail

/// Base64-encodes a range of bytes, *with* padding.
///
/// See `base64_encode` for a version without padding.
///
/// Precondition: readable_bounded_range(b, e)
///   && writable_counted_range(o,
///       base64_encoded_with_padding_byte_count(std::distance(b, e)))
///
/// ForwardIterator<char> I, OutputIterator<char> O
template<typename I, typename O>
O base64_encode_with_padding(I b, I e, O o) {
  auto co = std::copy(
    base64_begin(b, e), base64_end(b, e), detail::counting_output_iter(o));
  return std::fill_n(co.base, base64_padding_byte_count(co.count), '=');
}

namespace detail {
  /// ForwardIterator<char> I
  template<typename I>
  void reserve_base64_encode(
      std::string& s, I const& b, I const& e, std::random_access_iterator_tag) {
    s.reserve(base64_encoded_byte_count(e - b));
  }

  /// ForwardIterator<char> I
  template<typename I>
  void reserve_base64_encode(
      std::string&, I const&, I const&, std::forward_iterator_tag) {
  }

  /// ForwardIterator<char> I
  template<typename I>
  void reserve_base64_encode_with_padding(
      std::string& s, I const& b, I const& e, std::random_access_iterator_tag) {
    s.reserve(base64_encoded_with_padding_byte_count(e - b));
  }

  /// ForwardIterator<char> I
  template<typename I>
  void reserve_base64_encode_with_padding(
      std::string&, I const&, I const&, std::forward_iterator_tag) {
  }
} // namespace detail

/// Base64-encodes a range of bytes, *without* padding, returning a string.
///
/// Precondition: readable_bounded_range(b, e)
///
/// ForwardIterator<char> I
template<typename I>
std::string base64_encode(I b, I e) {
  std::string s;
  detail::reserve_base64_encode(s, b, e,
    typename std::iterator_traits<I>::iterator_category{});
  base64_encode(b, e, std::back_inserter(s));
  return s;
}

/// Base64-encodes a range of bytes, *with* padding, returning a string.
///
/// Precondition: readable_bounded_range(b, e)
///
/// ForwardIterator<char> I
template<typename I>
std::string base64_encode_with_padding(I b, I e) {
  std::string s;
  detail::reserve_base64_encode_with_padding(s, b, e,
    typename std::iterator_traits<I>::iterator_category{});
  base64_encode_with_padding(b, e, std::back_inserter(s));
  return s;
}

} // namespace ka

#endif // KA_BASE64_HPP
