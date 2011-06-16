/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <qi/locale.hpp>
#include <boost/locale.hpp>

namespace qi {

// Use to convert every boost::filesystem::path
  static std::locale                                        gLocale    = \
    boost::locale::generator().generate("");

  static const std::codecvt<wchar_t, char, std::mbstate_t>& gUtf8Facet = \
    std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> > (gLocale);


  const std::locale                                 &utf8locale()
  {
    return gLocale;
  }

  const std::codecvt<wchar_t, char, std::mbstate_t> &utf8facet()
  {
    return gUtf8Facet;
  }

}
