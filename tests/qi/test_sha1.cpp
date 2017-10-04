#include <iterator>
#include <chrono>
#include <cstdint>
#include <boost/range/algorithm/copy.hpp>
#include <boost/utility/string_ref.hpp>
#include <gtest/gtest.h>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/range.hpp>
#include <qi/sha1.hpp>

TEST(Sha1, Range)
{
  using namespace qi;
  ASSERT_EQ(sha1(std::string{"youpiyoup"}),
    (Sha1Digest{0xb5, 0x99, 0xe7, 0xda, 0xd5, 0x29, 0x33, 0xe3, 0x35, 0x4c,
                0x17, 0x66, 0x18, 0xe8, 0x2c, 0xd5, 0xc1, 0x80, 0xd8, 0xa2}));
}

namespace
{
  const qi::Sha1Digest youpiDigest{
    0x0e, 0x83, 0x19, 0x99, 0xc6, 0xd5, 0xd4, 0x4f, 0x6b, 0x89,
    0x26, 0xfe, 0x65, 0x75, 0xc1, 0xfd, 0x5d, 0x63, 0xdc, 0x6e
  };
}

TEST(Sha1, RandIter)
{
  using namespace qi;
  //                   0    5
  const std::string s{"youpiyoup"};
  ASSERT_EQ(sha1(begin(s), begin(s) + 5), youpiDigest);
}

TEST(Sha1, InputIter)
{
  using namespace qi;
  using I = std::istream_iterator<uint8_t>;
  std::istringstream s{"youpi"};
  ASSERT_EQ(sha1(I{s}, I{}), youpiDigest);
}

TEST(Sha1, Pointers)
{
  using namespace qi;
  uint8_t s[] = "youpi";
  ASSERT_EQ(sha1(std::begin(s), std::end(s) - 1), youpiDigest); // discards final '\0'
}

namespace
{
  const qi::Sha1Digest youpiNullTerminatedDigest{
    0xaf, 0x62, 0xce, 0x0f, 0x92, 0x2a, 0xfa, 0xb2, 0xe1, 0xc1,
    0xb5, 0xb3, 0xc6, 0x42, 0xb2, 0xdd, 0xa3, 0xcb, 0xa1, 0xa8
  };
}

// Beware that literal strings contain a null terminating character, that will
// be processed.
TEST(Sha1, RawString)
{
  using namespace qi;
  ASSERT_EQ(sha1("youpi"), youpiNullTerminatedDigest);
}

// Idem char arrays.
TEST(Sha1, CharArray)
{
  using namespace qi;
  char s[] = "youpi";
  ASSERT_EQ(sha1(s), youpiNullTerminatedDigest);
}

TEST(Sha1, StringRef)
{
  using namespace qi;
  ASSERT_EQ(sha1(boost::string_ref{"youpi"}), youpiDigest);
}

// This test must fail to compile.
// Uncomment the assertion to check it...
TEST(Sha1, BiggestThanChar)
{
  using namespace qi;
  struct BiggestThanChar
  {
    char c[2];
  };
  std::array<BiggestThanChar, 2> a;
  //ASSERT_EQ(sha1(a), Sha1Digest{0});
}

TEST(Sha1, DigestRegular)
{
  using namespace qi;
  Sha1Digest begin{0};
  Sha1Digest end{0};
  const std::size_t N = 5;
  for (auto i = 0; i != N; ++i) end[i] = 10;
  ASSERT_TRUE(detail::isRegular(incrRange(begin, end,
    [=](Sha1Digest& d) {
      for (auto i = 0; i != N; ++i) ++d[i];
    }
  )));
}

namespace
{
  const std::size_t iterationCount = 10000;
}

// The following tests are only to have an idea of the performance of `sha1` with
// different kinds of iterators.
// We expect that raw pointers will be faster due to the underlying C
// implementation.
// We also expect the performance of non-pointer iterators (random access or not)
// will be the same.
TEST(Sha1, PerformanceRIter)
{
  using namespace qi;
  using namespace std::chrono;
  std::string s{"abcd"};
  high_resolution_clock clock;
  const auto start = clock.now();
  for (auto i = iterationCount; i != 0; --i)
  {
    s[0] = sha1(s)[0];
  }
  std::cout << duration_cast<nanoseconds>(clock.now() - start).count() << " ns\n";
  std::cout << s << '\n'; // print to avoid the computation to be optimized out
}

namespace std
{
  template<typename I>
  I begin(const pair<I, I>& x)
  {
    return x.first;
  }

  template<typename I>
  I end(const pair<I, I>& x)
  {
    return x.second;
  }
} // namespace std

TEST(Sha1, PerformancePointers)
{
  using namespace qi;
  using namespace std::chrono;
  uint8_t s[] = "abcd";
  const auto range = std::make_pair(std::begin(s), std::end(s) - 1);
  high_resolution_clock clock;
  const auto start = clock.now();
  for (auto i = iterationCount; i != 0; --i)
  {
    s[0] = sha1(range)[0];
  }
  std::cout << duration_cast<nanoseconds>(clock.now() - start).count() << " ns\n";
  std::cout << s << '\n'; // print to avoid the computation to be optimized out
}

TEST(Sha1, PerformanceIIter)
{
  using namespace qi;
  using namespace std::chrono;
  using I = std::istream_iterator<uint8_t>;
  std::string s{"abcd"};
  std::istringstream ss{s};
  const auto range = std::make_pair(I{ss}, I{});
  high_resolution_clock clock;
  const auto start = clock.now();
  for (auto i = iterationCount; i != 0; --i)
  {
    s[0] = sha1(range)[0];
  }
  std::cout << duration_cast<nanoseconds>(clock.now() - start).count() << " ns\n";
  std::cout << s << '\n'; // print to avoid the computation to be optimized out
}
