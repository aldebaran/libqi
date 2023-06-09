#ifndef KA_SHA1_HPP
#define KA_SHA1_HPP
#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <openssl/evp.h>
#include "scoped.hpp"
#include "typetraits.hpp"

/// @file Contains sha1 digest functions.
///
/// The implementation relies on openssl.
///
/// An alternative is `boost::uuids::detail::sha1` but:
/// 1) it is in a `detail` namespace, so not officially supported
/// 2) the produced digests are different from the one of tools such as
///    `openssl sha1` and `sha1sum` (!).
///    This is apparently because of a wrong handling of endianness.

namespace ka {
  /// The result of a sha1 computation is an array of 20 bytes.
  using sha1_digest_t = std::array<uint8_t, 20>;

  namespace detail {
    // With pointers, we use the single-call update.
    //
    // Precondition: boundedRange(b, e) && s has been initialized
    //
    // I == T*, with T having the size of one byte (e.g. int8_t, uint8_t,
    // unsigned char on some platforms, etc.)
    template<typename I>
    int digest_update(EVP_MD_CTX& ctx, I b, I e, std::true_type /* is_pointer */) {
      return EVP_DigestUpdate(&ctx, b, e - b);
    }

    // With non-pointer iterators, we update byte by byte.
    //
    // Precondition: boundedRange(b, e) && s has been initialized
    //
    // InputIterator<T> I, with T having the size of one byte
    template<typename I>
    int digest_update(EVP_MD_CTX& ctx, I b, I e, std::false_type /* is_pointer */) {
      int res = 1;
      while (b != e) {
        auto const c = *b;
        res = EVP_DigestUpdate(&ctx, &c, 1u);
        if (res == 0) {
          return res;
        }
        ++b;
      }
      return res;
    }
  } // namespace detail

  /// Computes the sha1 digest of the given bytes.
  ///
  /// Note: Passing pointers instead of non-pointer iterators (even random access ones)
  ///   will typically be faster, due to the underlying C api.
  ///
  /// Precondition: boundedRange(b, e)
  ///
  /// InputIterator<T> I, with T having the size of one byte
  template<typename I>
  sha1_digest_t sha1(I b, I e) {
    static_assert(sizeof(Decay<decltype(*b)>) == 1,
      "sha1: element size is different than 1.");

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(
        EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    if (!EVP_DigestInit_ex2(ctx.get(), EVP_sha1(), nullptr)) {
      throw std::runtime_error("Can't initialize the sha1 digest context. "
                               "data=\"" + std::string(b, e) + "\"");
    }

    if (!detail::digest_update(*ctx, b, e, std::is_pointer<I>{})) {
      throw std::runtime_error("Can't update sha1 on \"" + std::string(b, e) + "\"");
    }
    sha1_digest_t d;
    if (!EVP_DigestFinal_ex(ctx.get(), d.data(), nullptr)) {
      throw std::runtime_error("Can't compute sha1 on \"" + std::string(b, e) + "\"");
    }
    return d;
  }

  /// Computes the sha1 digest of the given bytes.
  ///
  /// Warning: This function operates on binary data. If you pass a string literal,
  /// it will be processed as a normal array. This means that the final '\0' will
  /// be processed.
  ///
  /// Thus beware that:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// assert(sha1("youpi") != sha1(std::string{"youpi"}));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Linearizable<T> L, with T having the size of one byte
  template<typename L>
  sha1_digest_t sha1(L const& l) {
    using std::begin;
    using std::end;
    return sha1(begin(l), end(l));
  }
} // namespace ka

#endif // KA_SHA1_HPP
