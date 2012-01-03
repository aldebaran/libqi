/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef   	LIBQI_QI_HPP_
# define   	LIBQI_QI_HPP_

#include <qi/config.hpp>
#include <locale>

namespace qi {

  QI_API void init(int argc, char *argv[]);
  QI_API int argc();
  QI_API const char** argv();
  QI_API const char *program();

  typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_type;
  QI_API const codecvt_type &unicodeFacet();
}

#endif	    /* !QI_PP_ */
