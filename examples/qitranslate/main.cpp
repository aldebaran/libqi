/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <qi/application.hpp>
#include <qi/os.hpp>

#define _(string) qi::os::gettext(string)

int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);
  app.setName("translate");
  app.loadTranslationDict("translate");

  app.setTranslationLocale("en_US");
  std::cout << _("Hi, my name is NAO.") << std::endl;
  std::cout << _("Where is Brian?") << std::endl;
  std::cout << _("Brian is in the kitchen.") << std::endl;

  app.setTranslationLocale("fr_FR");
  std::cout << _("Hi, my name is NAO.") << std::endl;
  std::cout << _("Where is Brian?") << std::endl;
  std::cout << _("Brian is in the kitchen.") << std::endl;
}
