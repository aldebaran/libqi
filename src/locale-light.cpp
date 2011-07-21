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

  const codecvt_type &unicodeFacet()
  {
    const codecvt_type *ret = gUtf8CodecvtFacet;
    return *ret;
  }

}
