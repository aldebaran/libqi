/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qi/translator.hpp>


int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);
  app.setName("qitranslate");

  qi::Translator &tl = qi::defaultTranslator(app.name());
  tl.setCurrentLocale("en_US");
  tl.setDefaultDomain("translate");

  std::cout << qi::tr("Hi, my name is NAO.") << std::endl;
  std::cout << qi::tr("Where is Brian?") << std::endl;
  std::cout << qi::tr("Brian is in the kitchen.") << std::endl;

  tl.setCurrentLocale("fr_FR");
  std::cout << qi::tr("Hi, my name is NAO.") << std::endl;
  std::cout << qi::tr("Where is Brian?") << std::endl;
  std::cout << qi::tr("Brian is in the kitchen.") << std::endl;

  std::cout << qi::tr("Brian is in the kitchen.", "", "en_US") << std::endl;
  std::cout << qi::tr("Brian is in the kitchen.", "", "fr_FR") << std::endl;
}
