/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <qi/macro.hpp>
#include <qi/path.hpp>
#include <qi/atomic.hpp>

#define BOOST_UTF8_BEGIN_NAMESPACE  namespace qi { namespace detail {
#define BOOST_UTF8_END_NAMESPACE    }}
#define BOOST_UTF8_DECL
#include <boost/detail/utf8_codecvt_facet.hpp>
#include <boost/detail/utf8_codecvt_facet.ipp>

namespace qi {

  //this is initialized once.. and will be reported to leak memory.
  //but that okay for a global to be freed by the program termination
  static detail::utf8_codecvt_facet *gUtf8CodecvtFacet = nullptr;

  codecvt_type &unicodeFacet()
  {
    QI_THREADSAFE_NEW(gUtf8CodecvtFacet);
    return *gUtf8CodecvtFacet;
  }

}
