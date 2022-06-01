#include "uri_common.hpp"
#include <ka/uri/parsing.hpp>
#include <ka/empty.hpp>
#include <boost/range/size.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <gmock/gmock.h>

using namespace ka;
using namespace ka::parse;
using namespace ka::detail_uri;
using namespace ka::detail_uri::parsing;
using namespace testing;
using std::begin; using std::end;

namespace ka
{
namespace parse
{
  template<typename T, typename I>
  void PrintTo(ka::parse::res_t<T, I> const& res, std::ostream* os) {
    auto const empty = ka::empty(res);
    if (empty) {
      *os << "err(";
    }
    else {
      *os << "ok(" << PrintToString(ka::src(res)) << ", ";
    }
    *os << PrintToString(iter(res)) << ")";
  }
} // namespace parse
} // namespace ka

TEST(UriParsingFilter, Succeeds) {
  int const in[] = { 6 };
  auto const expected = *in;
  auto const p = filter([](int i){ return i % 2 == 0; }, parse::symbol);
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingFilter, SucceedsWhenComposed) {
  int const in[] = { 10 };
  auto const expected = *in;
  auto const p = filter([](int i){ return i % 2 == 0; },
                        filter([](int i) { return i % 5 == 0; }, parse::symbol));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingFilter, Fails) {
  auto const t = type_t<int>{};
  int const in[] = { 4 };
  auto const p = filter([](int i){ return i % 3 == 0; }, parse::symbol);
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingLiteral, Succeeds) {
  int const in[] = { 42 };
  auto const expected = *in;
  auto const p = literal(42);
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingLiteral, Fails) {
  auto const t = type_t<int>{};
  int const in[] = { 42 };
  auto const p = literal(15);
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingLiteralsInt, Succeeds) {
  int const in[] = { 4, 12, 22 };
  auto const expected = ka::product(4, 12, 22);
  auto const p = literals(in);
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingLiteralsInt, Fails) {
  auto const t = type_t<ka::product_t<int, int>>{};
  int const in[] = { 4, 12, 22 };
  int const args[] = { 2983, 4387 };
  auto const p = literals(args);
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingLiteralsString, Succeeds) {
  std::string const in = "ab";
  auto const expected = ka::product('a', 'b');
  auto const p = literals("ab");
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingLiteralsString, Fails) {
  auto const t = type_t<ka::product_t<char, char>>{};
  std::string const in = "ab";
  auto const p = literals("ac");
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtLeast, SucceedsOnExactCount) {
  std::string const in = "aaa";
  std::vector<char> const expected = {'a', 'a', 'a'};
  auto const p = at_least(3, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtLeast, SucceedsWhenMore) {
  std::string const in = "aaaaa";
  std::vector<char> const expected = {'a', 'a', 'a', 'a', 'a'};
  auto const p = at_least(3, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtLeast, Fails) {
  auto const t = type_t<std::vector<char>>{};
  std::string const in = "aa";
  auto const p = at_least(3, literal('a'));
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtMost, SucceedsOnExactCount) {
  std::string const in = "aaa";
  std::vector<char> const expected = {'a', 'a', 'a'};
  auto const p = at_most(3, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtMost, SucceedsWhenLess) {
  std::string const in = "aa";
  std::vector<char> const expected = {'a', 'a'};
  auto const p = at_most(3, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtMost, SucceedsWithZero) {
  std::string const in = "";
  std::vector<char> expected;
  auto const p = at_most(3, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingAtMost, SucceedsWithMoreButStops) {
  std::string const in = "aaaa";
  std::vector<char> const expected = {'a', 'a', 'a'};
  auto const p = at_most(3, literal('a'));
  EXPECT_EQ(p(begin(in), end(in)), ok(expected, begin(in) + 3));
}

TEST(UriParsingBetween, Succeeds) {
  std::string const in = "aaa";
  std::vector<char> const expected = {'a', 'a', 'a'};
  auto const p = between(2, 4, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingBetween, SucceedsWhenExactlyLowerBound) {
  std::string const in = "aa";
  std::vector<char> const expected = {'a', 'a'};
  auto const p = between(2, 4, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingBetween, SucceedsWhenExactlyUpperBound) {
  std::string const in = "aaaa";
  std::vector<char> const expected = {'a', 'a', 'a', 'a'};
  auto const p = between(2, 4, literal('a'));
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingBetween, SucceedsWithMoreButStops) {
  std::string const in = "aaaaab";
  std::vector<char> const expected = {'a', 'a', 'a', 'a'};
  auto const p = between(2, 4, literal('a'));
  EXPECT_EQ(p(begin(in), end(in)), ok(expected, begin(in) + 4));
}

TEST(UriParsingBetween, FailsWithLess) {
  auto const t = type_t<std::vector<char>>{};
  std::string const in = "a";
  auto const p = between(2, 4, literal('a'));
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingBetween, FailsWithZero) {
  auto const t = type_t<std::vector<char>>{};
  std::string const in = "b";
  auto const p = between(2, 4, literal('a'));
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

namespace test_uri_parsing {
  /// Generates a string containing all characters that are not in the parameter.
  std::string const all_chars_but(std::string except) {
    static auto const all = []{
      std::string res;
      static_assert(std::numeric_limits<int>::max() > std::numeric_limits<char>::max(), "");
      for (int c = std::numeric_limits<char>::min();
               c <= std::numeric_limits<char>::max();
               ++c) {
        res += static_cast<char>(c);
      }
      return res;
    }();

    std::sort(except.begin(), except.end());
    except.erase(std::unique(except.begin(), except.end()), except.end());
    std::string res;
    std::set_difference(all.begin(), all.end(), except.begin(), except.end(),
                        std::back_inserter(res));
    return res;
  }

  std::string to_string(char c) {
    return std::string(1u, c);
  }

  std::string const alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::string const digit = "0123456789";
  std::string const hexdig = "0123456789abcdefABCDEF";
  std::string const unreserved_other = "-._~";
  std::string const gen_delims = ":/?#[]@";
  std::string const sub_delims = "!$&'()*+,;=";
} // namespace test_uri_parsing

using namespace test_uri_parsing;

struct UriParsingAlpha : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(Alpha,
                        UriParsingAlpha,
                        ValuesIn(alpha));

TEST_P(UriParsingAlpha, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::alpha;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingAlphaFailure : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(AllButAlpha,
                        UriParsingAlphaFailure,
                        ValuesIn(all_chars_but(alpha)));

TEST_P(UriParsingAlphaFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::alpha;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingAlphaNum : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(Alpha,
                        UriParsingAlphaNum,
                        ValuesIn(alpha));
INSTANTIATE_TEST_SUITE_P(Digit,
                        UriParsingAlphaNum,
                        ValuesIn(digit));

TEST_P(UriParsingAlphaNum, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::alphanum;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingAlphaNumFailure : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(AllButAlphanum,
                        UriParsingAlphaNumFailure,
                        ValuesIn(all_chars_but(alpha + digit)));

TEST_P(UriParsingAlphaNumFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::alphanum;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingDigit : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(Digit,
                        UriParsingDigit,
                        ValuesIn(digit));

TEST_P(UriParsingDigit, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::digit;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingDigitFailure : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(AllButDigit,
                        UriParsingDigitFailure,
                        ValuesIn(all_chars_but(digit)));

TEST_P(UriParsingDigitFailure, Succeeds) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::digit;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingHexdig : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(Hexdig,
                        UriParsingHexdig,
                        ValuesIn(hexdig));

TEST_P(UriParsingHexdig, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::hexdig;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingHexdigFailure : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(AllButHexdig,
                        UriParsingHexdigFailure,
                        ValuesIn(all_chars_but(hexdig)));

TEST_P(UriParsingHexdigFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::hexdig;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingColon, SucceedsOnColon) {
  char const in[] = { ':' };
  auto const expected = *in;
  auto const p = grammar::colon;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingColonFailure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllButColon,
                        UriParsingColonFailure,
                        ValuesIn(all_chars_but(":")));

TEST_P(UriParsingColonFailure, FailsOnOthers) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::colon;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingDbColon, Succeeds) {
  std::string const in = "::";
  auto const expected = ka::product(':', ':');
  auto const p = grammar::dblcolon;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingDblColon, FailsWithOnlyOne) {
  auto const t = type_t<ka::product_t<char, char>>{};
  std::string const in = ":;";
  auto const p = grammar::dblcolon;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingDblColon, Fails) {
  auto const t = type_t<ka::product_t<char, char>>{};
  std::string const in = "a:";
  auto const p = grammar::dblcolon;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingPeriod, SucceedsOnPeriod) {
  char const in[] = { '.' };
  auto const expected = *in;
  auto const p = grammar::period;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingPeriodFailure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllButPeriod,
                        UriParsingPeriodFailure,
                        ValuesIn(all_chars_but(".")));

TEST_P(UriParsingPeriodFailure, FailsOnOthers) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::period;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingSlash, SucceedsOnSlash) {
  char const in[] = { '/' };
  auto const expected = *in;
  auto const p = grammar::slash;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingSlashFailure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllButSlash,
                        UriParsingSlashFailure,
                        ValuesIn(all_chars_but("/")));

TEST_P(UriParsingSlashFailure, FailsOnOthers) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::slash;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingUnreserved : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(Alpha,
                        UriParsingUnreserved,
                        ValuesIn(alpha));
INSTANTIATE_TEST_SUITE_P(Digit,
                        UriParsingUnreserved,
                        ValuesIn(digit));
INSTANTIATE_TEST_SUITE_P(UnreservedOther,
                        UriParsingUnreserved,
                        ValuesIn(unreserved_other));

TEST_P(UriParsingUnreserved, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::unreserved;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingUnreservedFailure : TestWithParam<char>{};
INSTANTIATE_TEST_SUITE_P(AllButUnreserved,
                        UriParsingUnreservedFailure,
                        ValuesIn(all_chars_but(alpha + digit + unreserved_other)));

TEST_P(UriParsingUnreservedFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::unreserved;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

namespace test_uri_parsing {
  namespace pct_encoded {
  std::vector<std::string> all() {
    std::vector<std::string> res;
    for (auto const c1 : hexdig) {
      for (auto const c2 : hexdig) {
        res.push_back(std::string{} + '%' + c1 + c2);
      }
    }
    return res;
  }

  std::string const sub_delims [] = {
    "%21", /* ! */ "%24", /* $ */ "%26", /* & */ "%27", /* ' */ "%28", /* ( */ "%29", /* ) */
    "%2A", /* * */ "%2B", /* + */ "%2C", /* , */ "%3B", /* ; */ "%3D", /* = */
  };
  std::string const gen_delims [] = {
    "%3A", /* : */ "%2F", /* / */ "%3F", /* ? */ "%23", /* # */ "%5B", /* [ */ "%5D", /* ] */
    "%40", /* @ */
  };
  std::pair<std::string, char> const alpha[] = {
    { "%61", 'a' }, { "%62", 'b' }, { "%63", 'c' }, { "%64", 'd' }, { "%65", 'e' }, { "%66", 'f' },
    { "%67", 'g' }, { "%68", 'h' }, { "%69", 'i' }, { "%6A", 'j' }, { "%6B", 'k' }, { "%6C", 'l' },
    { "%6D", 'm' }, { "%6E", 'n' }, { "%6F", 'o' }, { "%70", 'p' }, { "%71", 'q' }, { "%72", 'r' },
    { "%73", 's' }, { "%74", 't' }, { "%75", 'u' }, { "%76", 'v' }, { "%77", 'w' }, { "%78", 'x' },
    { "%79", 'y' }, { "%7A", 'z' }, { "%41", 'A' }, { "%42", 'B' }, { "%43", 'C' }, { "%44", 'D' },
    { "%45", 'E' }, { "%46", 'F' }, { "%47", 'G' }, { "%48", 'H' }, { "%49", 'I' }, { "%4A", 'J' },
    { "%4B", 'K' }, { "%4C", 'L' }, { "%4D", 'M' }, { "%4E", 'N' }, { "%4F", 'O' }, { "%50", 'P' },
    { "%51", 'Q' }, { "%52", 'R' }, { "%53", 'S' }, { "%54", 'T' }, { "%55", 'U' }, { "%56", 'V' },
    { "%57", 'W' }, { "%58", 'X' }, { "%59", 'Y' }, { "%5A", 'Z' },
  };
  std::pair<std::string, char> const numeric[] = {
    { "%30", '0' }, { "%31", '1' }, { "%32", '2' }, { "%33", '3' }, { "%34", '4' }, { "%35", '5' },
    { "%36", '6' }, { "%37", '7' }, { "%38", '8' }, { "%39", '9' },
  };
  std::pair<std::string, char> const other_unreserved[] = {
    { "%2D", '-' }, { "%2E", '.' }, { "%5F", '_' }, { "%7E", '~' },
  };
  std::pair<std::string, char> percent = { "%25", '%' };
  } // namespace pct_encoded
} // namespace test_uri_parsing

struct UriParsingPctEncodedReserved : TestWithParam<std::string> {};
INSTANTIATE_TEST_SUITE_P(SubDelims,
                        UriParsingPctEncodedReserved,
                        ValuesIn(pct_encoded::sub_delims));
INSTANTIATE_TEST_SUITE_P(GenDelims,
                        UriParsingPctEncodedReserved,
                        ValuesIn(pct_encoded::gen_delims));

TEST_P(UriParsingPctEncodedReserved, CharactersAreLeftEncoded) {
  auto const in = GetParam();
  auto const expected = in;
  auto const p = grammar::pct_encoded;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingPctEncodedUnreserved : TestWithParam<std::pair<std::string, char>> {};
INSTANTIATE_TEST_SUITE_P(Alpha,
                        UriParsingPctEncodedUnreserved,
                        ValuesIn(pct_encoded::alpha));
INSTANTIATE_TEST_SUITE_P(Numeric,
                        UriParsingPctEncodedUnreserved,
                        ValuesIn(pct_encoded::numeric));
INSTANTIATE_TEST_SUITE_P(OtherUnreserved,
                        UriParsingPctEncodedUnreserved,
                        ValuesIn(pct_encoded::other_unreserved));
INSTANTIATE_TEST_SUITE_P(Percent,
                        UriParsingPctEncodedUnreserved,
                        Values(pct_encoded::percent));

TEST_P(UriParsingPctEncodedUnreserved, CharactersAreDecoded) {
  auto const param = GetParam();
  auto const in = param.first;
  auto const expected = test_uri_parsing::to_string(param.second);
  auto const p = grammar::pct_encoded;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingPctEncoded, CaseNormalization) {
  std::string const in = "%2f"; // '/' character.
  std::string const expected = "%2F";
  auto const p = grammar::pct_encoded;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingGenDelims : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(GenDelims,
                        UriParsingGenDelims,
                        ValuesIn(gen_delims));

TEST_P(UriParsingGenDelims, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::gen_delims;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingGenDelimsFailure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllButGenDelims,
                        UriParsingGenDelimsFailure,
                        ValuesIn(all_chars_but(gen_delims)));

TEST_P(UriParsingGenDelimsFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::gen_delims;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingSubDelims : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(SubDelims,
                        UriParsingSubDelims,
                        ValuesIn(sub_delims));

TEST_P(UriParsingSubDelims, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::sub_delims;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingSubDelimsFailure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllButSubDelims,
                        UriParsingSubDelimsFailure,
                        ValuesIn(all_chars_but(sub_delims)));

TEST_P(UriParsingSubDelimsFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::sub_delims;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingReserved : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(GenDelims,
                        UriParsingReserved,
                        ValuesIn(gen_delims));
INSTANTIATE_TEST_SUITE_P(SubDelims,
                        UriParsingReserved,
                        ValuesIn(sub_delims));

TEST_P(UriParsingReserved, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::reserved;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingReservedFailure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllButReserved,
                        UriParsingReservedFailure,
                        ValuesIn(all_chars_but(gen_delims + sub_delims)));

TEST_P(UriParsingReservedFailure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::reserved;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

namespace test_uri_parsing {
  std::string const pchar_other = ":@";
}

using namespace boost::adaptors;

struct UriParsingPChar : TestWithParam<std::string> {};
INSTANTIATE_TEST_SUITE_P(Alpha,
                        UriParsingPChar,
                        ValuesIn(alpha | transformed(test_uri_parsing::to_string)));
INSTANTIATE_TEST_SUITE_P(Digit,
                        UriParsingPChar,
                        ValuesIn(digit | transformed(test_uri_parsing::to_string)));
INSTANTIATE_TEST_SUITE_P(UnreservedOther,
                        UriParsingPChar,
                        ValuesIn(unreserved_other | transformed(test_uri_parsing::to_string)));
INSTANTIATE_TEST_SUITE_P(PctEncodedSubDelims,
                        UriParsingPChar,
                        ValuesIn(pct_encoded::sub_delims));
INSTANTIATE_TEST_SUITE_P(PctEncodedGenDelims,
                        UriParsingPChar,
                        ValuesIn(pct_encoded::gen_delims));
INSTANTIATE_TEST_SUITE_P(SubDelims,
                        UriParsingPChar,
                        ValuesIn(sub_delims | transformed(test_uri_parsing::to_string)));
INSTANTIATE_TEST_SUITE_P(PCharOther,
                        UriParsingPChar,
                        ValuesIn(pchar_other | transformed(test_uri_parsing::to_string)));

TEST_P(UriParsingPChar, Succeeds) {
  auto const in = GetParam();
  auto const expected = in;
  auto const p = grammar::pchar;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingPCharFailure : TestWithParam<std::string> {};
INSTANTIATE_TEST_SUITE_P(AllButPChar,
                        UriParsingPCharFailure,
                        ValuesIn(all_chars_but(alpha + digit + unreserved_other + sub_delims +
                                               pchar_other)
                                 | transformed(test_uri_parsing::to_string)));

TEST_P(UriParsingPCharFailure, Fails) {
  auto const t = type_t<std::string>{};
  auto const in = GetParam();
  auto const p = grammar::pchar;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

namespace test_uri_parsing {
  std::string const r1_to_9 = "123456789";
  std::string const r0_to_4 = "01234";
  std::string const r0_to_5 = "012345";
}

struct UriParsingR1To9 : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(Char1To9,
                        UriParsingR1To9,
                        ValuesIn(r1_to_9));

TEST_P(UriParsingR1To9, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::r1_to_9;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingR1To9Failure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllBut1To9,
                        UriParsingR1To9Failure,
                        ValuesIn(all_chars_but(r1_to_9)));

TEST_P(UriParsingR1To9Failure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::r1_to_9;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingR0To4 : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(Char0To4,
                        UriParsingR0To4,
                        ValuesIn(r0_to_4));

TEST_P(UriParsingR0To4, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::r0_to_4;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingR0To4Failure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllBut0To4,
                        UriParsingR0To4Failure,
                        ValuesIn(all_chars_but(r0_to_4)));

TEST_P(UriParsingR0To4Failure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::r0_to_4;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

struct UriParsingR0To5 : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(Char0To5,
                        UriParsingR0To5,
                        ValuesIn(r0_to_5));

TEST_P(UriParsingR0To5, Succeeds) {
  char const in[] = { GetParam() };
  auto const expected = *in;
  auto const p = grammar::r0_to_5;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingR0To5Failure : TestWithParam<char> {};
INSTANTIATE_TEST_SUITE_P(AllBut0To5,
                        UriParsingR0To5Failure,
                        ValuesIn(all_chars_but(r0_to_5)));

TEST_P(UriParsingR0To5Failure, Fails) {
  auto const t = type_t<char>{};
  char const in[] = { GetParam() };
  auto const p = grammar::r0_to_5;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

namespace test_uri_parsing {
  std::vector<std::string> dec_octet() {
    std::vector<std::string> res;
    for (int i = 0; i <= 255; ++i) {
      std::ostringstream oss;
      oss << i;
      res.push_back(oss.str());
    }
    return res;
  }

  std::string const not_dec_octet[] = {
    "abc", "ABC", "%25", ""
  };
}

struct UriParsingDecOctet : TestWithParam<std::string> {};
INSTANTIATE_TEST_SUITE_P(DecOctet,
                        UriParsingDecOctet,
                        ValuesIn(dec_octet()));

TEST_P(UriParsingDecOctet, Succeeds) {
  auto const in = GetParam();
  auto const expected = in;
  auto const p = grammar::dec_octet;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

struct UriParsingDecOctetFailure : TestWithParam<std::string> {};
INSTANTIATE_TEST_SUITE_P(NotDecOctet,
                        UriParsingDecOctetFailure,
                        ValuesIn(not_dec_octet));

TEST_P(UriParsingDecOctetFailure, Fails) {
  auto const t = type_t<std::string>{};
  auto const in = GetParam();
  auto const p = grammar::dec_octet;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingDecOctet, SucceedsOnBeginningOver255) {
  auto const p = grammar::dec_octet;
  {
    std::string const in = "256";
    std::string const expected = "25";
    EXPECT_EQ(p(begin(in), end(in)), ok(expected, begin(in) + 2));
  } {
    std::string const in = "500";
    std::string const expected = "50";
    EXPECT_EQ(p(begin(in), end(in)), ok(expected, begin(in) + 2));
  } {
    std::string const in = "99999";
    std::string const expected = "99";
    EXPECT_EQ(p(begin(in), end(in)), ok(expected, begin(in) + 2));
  } {
    std::string const in = "199999";
    std::string const expected = "199";
    EXPECT_EQ(p(begin(in), end(in)), ok(expected, begin(in) + 3));
  }
}

TEST(UriParsingScheme, CaseNormalization) {
  std::string const in = "CaSeInSeNsItIvE";
  std::string const expected = "caseinsensitive";
  auto const p = grammar::scheme;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

namespace test_uri_parsing {
  std::string const ipv6addresses[] = {
    // The goal here is not to test `ls32`, so we always use the same value for it.
    //                            6( h16 ":" ) ls32
    "a1b2:c3d4:e5f6:a7b8:c9d0:e1f2:a3b4:c5d6",
    //                       "::" 5( h16 ":" ) ls32
       "::c3d4:e5f6:a7b8:c9d0:e1f2:a3b4:c5d6",
    //                [h16 ] "::" 4( h16 ":" ) ls32
            "::e5f6:a7b8:c9d0:e1f2:a3b4:c5d6",
        "c3d4::e5f6:a7b8:c9d0:e1f2:a3b4:c5d6",
    // [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
                 "::a7b8:c9d0:e1f2:a3b4:c5d6",
             "e5f6::a7b8:c9d0:e1f2:a3b4:c5d6",
        "c3d4:e5f6::a7b8:c9d0:e1f2:a3b4:c5d6",
    // [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
                      "::c9d0:e1f2:a3b4:c5d6",
                  "a7b8::c9d0:e1f2:a3b4:c5d6",
             "e5f6:a7b8::c9d0:e1f2:a3b4:c5d6",
        "c3d4:e5f6:a7b8::c9d0:e1f2:a3b4:c5d6",
    // [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
                           "::e1f2:a3b4:c5d6",
                       "c9d0::e1f2:a3b4:c5d6",
                  "a7b8:c9d0::e1f2:a3b4:c5d6",
             "e5f6:a7b8:c9d0::e1f2:a3b4:c5d6",
        "c3d4:e5f6:a7b8:c9d0::e1f2:a3b4:c5d6",
    // [ *4( h16 ":" ) h16 ] "::"              ls32
                                "::a3b4:c5d6",
                            "e1f2::a3b4:c5d6",
                       "c9d0:e1f2::a3b4:c5d6",
                  "a7b8:c9d0:e1f2::a3b4:c5d6",
             "e5f6:a7b8:c9d0:e1f2::a3b4:c5d6",
        "c3d4:e5f6:a7b8:c9d0:e1f2::a3b4:c5d6",
    // [ *5( h16 ":" ) h16 ] "::"              h16
                                     "::c5d6",
                                 "a3b4::c5d6",
                            "e1f2:a3b4::c5d6",
                       "c9d0:e1f2:a3b4::c5d6",
                  "a7b8:c9d0:e1f2:a3b4::c5d6",
             "e5f6:a7b8:c9d0:e1f2:a3b4::c5d6",
        "c3d4:e5f6:a7b8:c9d0:e1f2:a3b4::c5d6",
    // [ *6( h16 ":" ) h16 ] "::"
                                         "::",
                                     "c5d6::",
                                "a3b4:c5d6::",
                           "e1f2:a3b4:c5d6::",
                      "c9d0:e1f2:a3b4:c5d6::",
                 "a7b8:c9d0:e1f2:a3b4:c5d6::",
            "e5f6:a7b8:c9d0:e1f2:a3b4:c5d6::",
       "c3d4:e5f6:a7b8:c9d0:e1f2:a3b4:c5d6::",
  };

  struct Ipv6Address : TestWithParam<std::string> {};
} // namespace test_uri_parsing

using UriParsingIpv6Address = Ipv6Address;
INSTANTIATE_TEST_SUITE_P(Ipv6Addresses,
                        UriParsingIpv6Address,
                        ValuesIn(ipv6addresses));

TEST_P(UriParsingIpv6Address, Succeeds) {
  auto const in = GetParam();
  auto const expected = in;
  auto const p = grammar::ipv6address;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingIpv6Address, Fails) {
  auto const t = type_t<std::string>{};
  std::string const in = "abc";
  auto const p = grammar::ipv6address;
  EXPECT_EQ(err(t, begin(in)), p(begin(in), end(in)));
}

TEST(UriParsingHost, CaseNormalization) {
  std::string const in = "SOMEhost";
  std::string const expected = "somehost";
  auto const p = grammar::host;
  EXPECT_EQ(ok(expected, end(in)), p(begin(in), end(in)));
}

TEST(UriParsingLexicallyNormalPath, Basic) {
  EXPECT_EQ("a",       lexically_normal_path("./a"));
  EXPECT_EQ("a/",      lexically_normal_path("./a/"));
  EXPECT_EQ("a/b",     lexically_normal_path("a/./b"));
  EXPECT_EQ("/a",      lexically_normal_path("/./a"));
  EXPECT_EQ("b",       lexically_normal_path("a/../b"));
  EXPECT_EQ("/b",      lexically_normal_path("/a/../b"));
  EXPECT_EQ("c",       lexically_normal_path("a/b/../../c"));
  EXPECT_EQ("a/",      lexically_normal_path("a/.///b/../"));
  EXPECT_EQ("/c/",     lexically_normal_path("///a//.///b/..////../c/////"));
  EXPECT_EQ("/a",      lexically_normal_path("/..//..////a"));
  EXPECT_EQ("/a/",     lexically_normal_path("/..//..////a/"));
  EXPECT_EQ("../../a", lexically_normal_path("..//..////a"));
  EXPECT_EQ("/",       lexically_normal_path("/"));
  EXPECT_EQ(".",       lexically_normal_path("./"));
  EXPECT_EQ("..",      lexically_normal_path("..///"));
  EXPECT_EQ("..",      lexically_normal_path("..///a/../"));
  EXPECT_EQ(".",       lexically_normal_path("a/.."));
  EXPECT_EQ(".",       lexically_normal_path("a/../"));
  EXPECT_EQ(".",       lexically_normal_path("."));
  EXPECT_EQ("",        lexically_normal_path(""));
}
