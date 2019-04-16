#include <iterator>
#include <chrono>
#include <cstdint>
#include <boost/range/algorithm/copy.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/algorithm/hex.hpp>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/ark/inputiter.hpp>
#include <ka/range.hpp>
#include <ka/sha1.hpp>

TEST(Sha1, Range) {
  using namespace ka;
  ASSERT_EQ(sha1(std::string{"youpiyoup"}),
    (sha1_digest_t{{0xb5, 0x99, 0xe7, 0xda, 0xd5, 0x29, 0x33, 0xe3, 0x35, 0x4c,
                    0x17, 0x66, 0x18, 0xe8, 0x2c, 0xd5, 0xc1, 0x80, 0xd8, 0xa2}}));
}

namespace {
  const ka::sha1_digest_t youpi_digest {{
    0x0e, 0x83, 0x19, 0x99, 0xc6, 0xd5, 0xd4, 0x4f, 0x6b, 0x89,
    0x26, 0xfe, 0x65, 0x75, 0xc1, 0xfd, 0x5d, 0x63, 0xdc, 0x6e
  }};
}

TEST(Sha1, RandIter) {
  using namespace ka;
  //                   0    5
  const std::string s{"youpiyoup"};
  ASSERT_EQ(sha1(begin(s), begin(s) + 5), youpi_digest);
}

TEST(Sha1, InputIter) {
  using namespace ka;
  using I = std::istream_iterator<uint8_t>;
  std::istringstream s{"youpi"};
  ASSERT_EQ(sha1(I{s}, I{}), youpi_digest);
}

TEST(Sha1, Pointers) {
  using namespace ka;
  uint8_t s[] = "youpi";
  ASSERT_EQ(sha1(std::begin(s), std::end(s) - 1), youpi_digest); // discards final '\0'
}

namespace {
  const ka::sha1_digest_t youpi_null_terminated_digest {{
    0xaf, 0x62, 0xce, 0x0f, 0x92, 0x2a, 0xfa, 0xb2, 0xe1, 0xc1,
    0xb5, 0xb3, 0xc6, 0x42, 0xb2, 0xdd, 0xa3, 0xcb, 0xa1, 0xa8
  }};
}

// Beware that literal strings contain a null terminating character, that will
// be processed.
TEST(Sha1, RawString) {
  using namespace ka;
  ASSERT_EQ(sha1("youpi"), youpi_null_terminated_digest);
}

// Idem char arrays.
TEST(Sha1, CharArray) {
  using namespace ka;
  char s[] = "youpi";
  ASSERT_EQ(sha1(s), youpi_null_terminated_digest);
}

TEST(Sha1, StringRef) {
  using namespace ka;
  ASSERT_EQ(sha1(boost::string_ref{"youpi"}), youpi_digest);
}

// This test must fail to compile.
// Uncomment the assertion to check it...
TEST(Sha1, BiggestThanChar) {
  using namespace ka;
  struct biggest_than_char_t {
    char c[2];
  };
  //std::array<biggest_than_char_t, 2> a;
  //ASSERT_EQ(sha1(a), sha1_digest_t{0});
}

TEST(Sha1, DigestRegular) {
  using namespace ka;
  sha1_digest_t begin {{}};
  sha1_digest_t end {{}};
  const std::size_t N = 5;
  for (auto i = 0u; i != N; ++i) end[i] = 10;
  ASSERT_TRUE(is_regular(bounded_range(begin, end,
    [=](sha1_digest_t& d) {
      for (auto i = 0u; i != N; ++i) ++d[i];
    }
  )));
}

namespace {
  const std::size_t iteration_count = 10000;
}

// The following tests are only to have an idea of the performance of `sha1` with
// different kinds of iterators.
// We expect that raw pointers will be faster due to the underlying C
// implementation.
// We also expect the performance of non-pointer iterators (random access or not)
// will be the same.
TEST(Sha1, PerformanceRIter) {
  using namespace ka;
  using namespace std::chrono;
  std::string s{"abcd"};
  using clock = high_resolution_clock;
  const auto start = clock::now();
  for (auto i = iteration_count; i != 0; --i) {
    s[0] = static_cast<char>(sha1(s)[0]);
  }
  std::cout << duration_cast<nanoseconds>(clock::now() - start).count() << " ns\n";
  // Print to avoid the computation to be optimized out.
  // `hex` returns the output iterator.
  *boost::algorithm::hex(s, std::ostream_iterator<char>{std::cout}) = '\n';
}

namespace std {
  template<typename I>
  I begin(const pair<I, I>& x) {
    return x.first;
  }

  template<typename I>
  I end(const pair<I, I>& x) {
    return x.second;
  }
} // namespace std

TEST(Sha1, PerformancePointers) {
  using namespace ka;
  using namespace std::chrono;
  uint8_t s[] = "abcd";
  const auto range = std::make_pair(std::begin(s), std::end(s) - 1);
  using clock = high_resolution_clock;
  const auto start = clock::now();
  for (auto i = iteration_count; i != 0; --i) {
    s[0] = sha1(range)[0];
  }
  std::cout << duration_cast<nanoseconds>(clock::now() - start).count() << " ns\n";
  // Print to avoid the computation to be optimized out.
  // `hex` returns the output iterator.
  *boost::algorithm::hex(s, std::ostream_iterator<char>{std::cout}) = '\n';
}

TEST(Sha1, PerformanceIIter) {
  using namespace ka;
  using namespace std::chrono;
  // Make the string iterator an input iterator.
  using I = ark::input_iter_t<std::string::iterator>;
  std::string s{"abcd"};
  const auto range = std::make_pair(I{s.begin()}, I{s.end()});
  using clock = high_resolution_clock;
  const auto start = clock::now();
  for (auto i = iteration_count; i != 0; --i) {
    s[0] = static_cast<char>(sha1(range)[0]);
  }
  std::cout << duration_cast<nanoseconds>(clock::now() - start).count() << " ns\n";
  // Print to avoid the computation to be optimized out.
  // `hex` returns the output iterator.
  *boost::algorithm::hex(s, std::ostream_iterator<char>{std::cout}) = '\n';
}
