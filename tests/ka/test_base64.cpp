#include <forward_list>
#include <sstream>
#include <string>
#include <cstring>
#include <ka/base64.hpp>
#include <gtest/gtest.h>
#include <gtest/gtest-param-test.h>

using std::strlen;

struct Base64WithoutPadding : testing::TestWithParam<std::pair<char const*, char const*>> {
};

namespace {
  auto pair = [](char const* a, char const* b) {
    return std::make_pair(a, b);
  };
}

INSTANTIATE_TEST_SUITE_P(
  DefaultInstantiation,
  Base64WithoutPadding,
  testing::Values(
    pair("", ""),
    pair("TQ", "M"),
    pair("TWE", "Ma"),
    pair("TWFu", "Man"),
    pair("Zg", "f"),
    pair("Zm8", "fo"),
    pair("Zm9v", "foo"),
    pair("Zm9vYg", "foob"),
    pair("Zm9vYmE", "fooba"),
    pair("Zm9vYmFy", "foobar"),
    pair("TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
        "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
        "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
        "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
        "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4",
        "Man is distinguished, not only by his reason, but by this singular passion "
        "from other animals, which is a lust of the mind, that by a perseverance of "
        "delight in the continued and indefatigable generation of knowledge, exceeds"
        " the short vehemence of any carnal pleasure."),
    pair("YW55IGNhcm5hbCBwbGVhc3VyZS4", "any carnal pleasure."),
    pair("YW55IGNhcm5hbCBwbGVhc3VyZQ", "any carnal pleasure"),
    pair("YW55IGNhcm5hbCBwbGVhc3Vy", "any carnal pleasur"),
    pair("YW55IGNhcm5hbCBwbGVhc3U", "any carnal pleasu"),
    pair("YW55IGNhcm5hbCBwbGVhcw", "any carnal pleas"),
    pair("cGxlYXN1cmUu", "pleasure."),
    pair("bGVhc3VyZS4", "leasure."),
    pair("ZWFzdXJlLg", "easure."),
    pair("YXN1cmUu", "asure."),
    pair("c3VyZS4", "sure.")
  )
);

TEST_P(Base64WithoutPadding, ForwardIterator) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::forward_list<char> l(non_encoded + 0, non_encoded + strlen(non_encoded));
  EXPECT_TRUE(std::equal(encoded, encoded + strlen(encoded),
    ka::base64_begin(l.begin(), l.end())));
}

TEST_P(Base64WithoutPadding, ForwardIteratorDistance) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::forward_list<char> const l(non_encoded + 0, non_encoded + strlen(non_encoded));
  auto b0 = l.begin();
  auto e0 = l.end();
  std::forward_list<char>::difference_type len_encoded = strlen(encoded);
  auto b = ka::base64_begin(b0, e0);
  auto e = ka::base64_end(b0, e0);
  while (b != e) {
    ASSERT_EQ(len_encoded, std::distance(b, e));
    --len_encoded;
    ++b;
  }
}

TEST_P(Base64WithoutPadding, ForwardIteratorJumps) {
  auto encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::forward_list<char> const l(non_encoded + 0, non_encoded + strlen(non_encoded));
  auto b = ka::base64_begin(l.begin(), l.end());
  auto e = ka::base64_end(l.begin(), l.end());
  auto max = strlen(encoded);
  auto min = decltype(max)(0);
  while (b != e) {
    for (auto i = min; i != max; ++i) {
      ASSERT_EQ(*(encoded + i), *(std::next(b, i)));
    }
    --max;
    ++encoded;
    ++b;
  }
}

TEST_P(Base64WithoutPadding, RandomAccessIterator) {
  using ka::base64_begin;
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  auto b = non_encoded + 0;
  auto e = non_encoded + strlen(non_encoded);
  EXPECT_TRUE(std::equal(encoded, encoded + strlen(encoded), base64_begin(b, e)));
}

TEST_P(Base64WithoutPadding, RandomAccessIteratorDistance) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::ptrdiff_t len_encoded = strlen(encoded);
  auto b0 = non_encoded + 0;
  auto e0 = non_encoded + strlen(non_encoded);
  auto b = ka::base64_begin(b0, e0);
  auto e = ka::base64_end(b0, e0);
  while (b != e) {
    auto n = len_encoded;
    for (auto i = e; i != b; --i) {
      ASSERT_EQ(n, std::distance(b, i));
      --n;
    }
    --len_encoded;
    ++b;
  }
}

TEST_P(Base64WithoutPadding, RandomAccessIteratorJumps) {
  auto encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  auto const b0 = non_encoded + 0;
  auto const e0 = non_encoded + strlen(non_encoded);
  auto b = ka::base64_begin(b0, e0);
  auto e = ka::base64_end(b0, e0);
  auto max = strlen(encoded);
  auto min = decltype(max)(0);
  while (b != e) {
    for (auto i = min; i != max; ++i) {
      ASSERT_EQ(*(encoded + i), *(b + i));
    }
    --min;
    --max;
    ++encoded;
    ++b;
  }
}

TEST_P(Base64WithoutPadding, Length) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  EXPECT_EQ(strlen(encoded), ka::base64_encoded_byte_count(strlen(non_encoded)));
}

TEST_P(Base64WithoutPadding, EncodeIter) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::string enc(ka::base64_encoded_byte_count(strlen(non_encoded)), '\0');
  auto const i = ka::base64_encode(non_encoded + 0, non_encoded + strlen(non_encoded), enc.begin());
  EXPECT_EQ(enc.end(), i);
  EXPECT_EQ(encoded, enc);
}

TEST_P(Base64WithoutPadding, EncodeString) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  auto const enc = ka::base64_encode(non_encoded + 0, non_encoded + strlen(non_encoded));
  EXPECT_EQ(encoded, enc);
}

struct Base64WithPadding : testing::TestWithParam<std::pair<char const*, char const*>> {
};

INSTANTIATE_TEST_SUITE_P(
  DefaultInstantiation,
  Base64WithPadding,
  testing::Values(
    pair("", ""),
    pair("TQ==", "M"),
    pair("TWE=", "Ma"),
    pair("TWFu", "Man"),
    pair("Zg==", "f"),
    pair("Zm8=", "fo"),
    pair("Zm9v", "foo"),
    pair("Zm9vYg==", "foob"),
    pair("Zm9vYmE=", "fooba"),
    pair("Zm9vYmFy", "foobar"),
    pair("TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
      "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
      "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
      "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
      "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=",
      "Man is distinguished, not only by his reason, but by this singular passion "
      "from other animals, which is a lust of the mind, that by a perseverance of "
      "delight in the continued and indefatigable generation of knowledge, exceeds"
      " the short vehemence of any carnal pleasure."),
    pair("YW55IGNhcm5hbCBwbGVhc3VyZS4=", "any carnal pleasure."),
    pair("YW55IGNhcm5hbCBwbGVhc3VyZQ==", "any carnal pleasure"),
    pair("YW55IGNhcm5hbCBwbGVhc3Vy", "any carnal pleasur"),
    pair("YW55IGNhcm5hbCBwbGVhc3U=", "any carnal pleasu"),
    pair("YW55IGNhcm5hbCBwbGVhcw==", "any carnal pleas"),
    pair("cGxlYXN1cmUu", "pleasure."),
    pair("bGVhc3VyZS4=", "leasure."),
    pair("ZWFzdXJlLg==", "easure."),
    pair("YXN1cmUu", "asure."),
    pair("c3VyZS4=", "sure.")
  )
);

TEST_P(Base64WithPadding, ByteCount) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  EXPECT_EQ(strlen(encoded), ka::base64_encoded_with_padding_byte_count(strlen(non_encoded)));
}

TEST_P(Base64WithPadding, PaddingByteCount) {
  auto rev = [](char const* a) {
    return std::reverse_iterator<char const*>(a);
  };
  auto const encoded = GetParam().first;
  auto const rb = rev(encoded + strlen(encoded));
  auto const re = rev(encoded + 0);
  std::size_t const padlen = std::find_if(rb, re, [](char a) {return a != '=';}) - rb;
  EXPECT_EQ(padlen, ka::base64_padding_byte_count(strlen(encoded) - padlen));
}

TEST_P(Base64WithPadding, ForwardIterator) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::forward_list<char> const l(non_encoded + 0, non_encoded + strlen(non_encoded));
  std::string enc(ka::base64_encoded_with_padding_byte_count(strlen(non_encoded)), '\0');
  auto const i = ka::base64_encode_with_padding(l.begin(), l.end(), enc.begin());
  EXPECT_EQ(enc.end(), i);
  EXPECT_EQ(encoded, enc);
}

TEST_P(Base64WithPadding, EncodeIter) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  std::string enc(ka::base64_encoded_with_padding_byte_count(strlen(non_encoded)), '\0');
  auto const i = ka::base64_encode_with_padding(non_encoded + 0, non_encoded + strlen(non_encoded), enc.begin());
  EXPECT_EQ(enc.end(), i);
  EXPECT_EQ(encoded, enc);
}

TEST_P(Base64WithPadding, EncodeString) {
  auto const encoded = GetParam().first;
  auto const non_encoded = GetParam().second;
  auto const enc = ka::base64_encode_with_padding(non_encoded + 0, non_encoded + strlen(non_encoded));
  EXPECT_EQ(encoded, enc);
}
