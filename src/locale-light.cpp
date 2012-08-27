/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <qi/macro.hpp>
#include <qi/qi.hpp>

#define BOOST_UTF8_BEGIN_NAMESPACE  namespace qi { namespace detail {
#define BOOST_UTF8_END_NAMESPACE    }}
#define BOOST_UTF8_DECL
#include "utf8_codecvt_facet.hpp"
#include "utf8_codecvt_facet_impl.hpp"

namespace qi {

  //this is initialized once.. and will be reported to leak memory.
  //but that okay for a global to be freed by the program termination
  static const detail::utf8_codecvt_facet *gUtf8CodecvtFacet = 0;

  const codecvt_type &unicodeFacet()
  {
    if (!gUtf8CodecvtFacet)
      gUtf8CodecvtFacet = new detail::utf8_codecvt_facet();
    const codecvt_type *ret = gUtf8CodecvtFacet;
    return *ret;
  }

}
