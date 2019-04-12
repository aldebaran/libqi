/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "utils.hpp"

#include <algorithm>
#include <iterator>
#include <random>

#include <boost/filesystem.hpp>
#include <boost/utility/string_ref.hpp>

#include <qi/os.hpp>
#include <qi/path.hpp>

namespace {

  static auto randomSource = [] {
    std::random_device trueRand;
    std::seed_seq seed{ trueRand(), trueRand(), trueRand(), trueRand()
                      , trueRand(), trueRand(), trueRand(), trueRand()
                      };
    return std::default_random_engine{ seed };
  }();

  // Returns a random number inside `[min, max]` (`min` and `max` are included).
  //
  // Precondition: min <= max
  //
  // N is one of {short, int, long, long long,
  //  unsigned short, unsigned int, unsigned long, unsigned long long}
  template<typename N>
  N randomNumber(N min, N max)
  {
    std::uniform_int_distribution<N> dice(min, max);
    return dice(randomSource);
  }

  // Returns a random character inside `[0-9a-zA-Z]`.
  char randomAlphaNum()
  {
    static const boost::string_ref alphanums
      = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    const auto randomIdx = randomNumber<int>(0, alphanums.size() - 1);
    return alphanums[randomIdx];
  }

}

std::string randomstr(std::string::size_type sz)
{
  std::string s;
  s.reserve(sz);
  generate_n(std::back_inserter(s), sz, randomAlphaNum);
  return s;
}

std::wstring wrandomstr(std::wstring::size_type sz)
{
  std::wstring s;
  s.reserve(sz);
  generate_n(std::back_inserter(s), sz, randomAlphaNum);
  return s;
}

std::string fsconcat(const std::vector<std::string>& parts)
{
  boost::filesystem::path p;
  for (const auto& part : parts)
  {
    if (!part.empty())
    {
      p.append(part, qi::unicodeFacet());
    }
  }
  return p.make_preferred().string(qi::unicodeFacet());
}
