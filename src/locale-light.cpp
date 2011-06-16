/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/
#include <qi/api.hpp>
#include <qi/locale.hpp>

#define BOOST_UTF8_BEGIN_NAMESPACE  namespace qi { namespace detail {
#define BOOST_UTF8_END_NAMESPACE    }}
#define BOOST_UTF8_DECL
#include "utf8_codecvt_facet.hpp"
#include "utf8_codecvt_facet_impl.hpp"

namespace qi {

  //this is initialized once.. and will be reported to leak memory.
  //but that okay for a global to be freed by the program termination
  static const detail::utf8_codecvt_facet *gUtf8CodecvtFacet = new detail::utf8_codecvt_facet();
  static const std::locale        gLocale(std::locale(), gUtf8CodecvtFacet);

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
