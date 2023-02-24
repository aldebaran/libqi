// Disable deprecation warnings about `std::auto_ptr`.
#define BOOST_LOCALE_HIDE_AUTO_PTR
#include <boost/locale.hpp>
#undef BOOST_LOCALE_HIDE_AUTO_PTR
#include <locale>
#include <iostream>

/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
int main(int, char *[])
{
  // Create string with one weird (ñ) char
  char iso[] = "La Pe\361a";

  // Use latin1 to convert string to UTF8
  std::string utf8 = boost::locale::conv::to_utf<char>(iso, "Latin1");

  std::cout << "The UTF8 string is: " << utf8 << std::endl;
  std::cout << "The Latin1 string is: " << iso << std::endl;

  return 0;
}

