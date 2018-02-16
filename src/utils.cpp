/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "utils.hpp"

#include <boost/filesystem.hpp>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <qi/os.hpp>
#include <qi/path.hpp>

static char rand_alnum()
{
  unsigned char c;
  while (true)
  {
    c = static_cast<unsigned char>(std::rand());
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9'))
      return c;
  }
}

std::string randomstr(std::string::size_type sz) {

  std::string s;
  s.reserve(sz);
  //need to have different seeds to avoid DDOS attack
  unsigned int seed = static_cast<unsigned int>(
      qi::SystemClock::now().time_since_epoch().count()/1000);
  srand(seed);
  generate_n(std::back_inserter(s), sz, rand_alnum);
  return s;
}

std::wstring wrandomstr(std::wstring::size_type sz) {
  std::wstring s;
  s.reserve(sz);
  //need to have different seeds to avoid DDOS attack
  unsigned int seed = static_cast<unsigned int>(
      qi::SystemClock::now().time_since_epoch().count()/1000);
  srand(seed);
  generate_n(std::back_inserter(s), sz, rand_alnum);
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
