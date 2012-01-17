/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "utils.hpp"

#include <cctype>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <qi/os.hpp>

static char rand_alnum()
{
  unsigned char c;
  while (!std::isalnum(c = static_cast<unsigned char>(std::rand())))
    ;
  return c;
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

static wchar_t wrand_alnum()
{
  wchar_t c;
  while (!std::isalnum(c = static_cast<wchar_t>(std::rand())))
    ;
  return c;
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
