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
#include <qi/qi.hpp>



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
  qi::os::timeval tv;
  qi::os::gettimeofday(&tv);
  srand(tv.tv_sec + tv.tv_usec);
  generate_n(std::back_inserter(s), sz, rand_alnum);
  return s;
}

std::wstring wrandomstr(std::wstring::size_type sz) {
  std::wstring s;
  s.reserve(sz);
  //need to have different seeds to avoid DDOS attack
  qi::os::timeval tv;
  qi::os::gettimeofday(&tv);
  srand(tv.tv_sec + tv.tv_usec);
  generate_n(std::back_inserter(s), sz, rand_alnum);
  return s;
}

std::string fsconcat(const std::string &p0,
                     const std::string &p1,
                     const std::string &p2,
                     const std::string &p3,
                     const std::string &p4,
                     const std::string &p5)
{
  boost::filesystem::path p(p0, qi::unicodeFacet());
  p.append(p1, qi::unicodeFacet());
  p.append(p2, qi::unicodeFacet());
  p.append(p3, qi::unicodeFacet());
  p.append(p4, qi::unicodeFacet());
  p.append(p5, qi::unicodeFacet());

  return p.make_preferred().string(qi::unicodeFacet());
}
