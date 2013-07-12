#include <boost/locale.hpp>
#include <locale>
#include <iostream>

/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
int main(int argc, char *argv[])
{
  // Create string with one weird (ñ) char
  char iso[] = {'L', 'a', ' ', 'P', 'e', 0xF1, 'a', 0};

  // Use latin1 to convert string to UTF8
  std::string utf8 = boost::locale::conv::to_utf<char>(iso, "Latin1");

  std::cout << "The UTF8 string is: " << utf8 << std::endl;
  std::cout << "The Latin1 string is: " << iso << std::endl;

  return 0;
}

