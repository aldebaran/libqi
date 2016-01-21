#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/**
 * \file
 * \brief dll import/export and compiler message
 */

#ifndef _QI_API_HPP_
# define _QI_API_HPP_

# include <qi/macro.hpp>
# include <qi/config.hpp>

/**
 * qi_EXPORTS controls which symbols are exported when libqi
 * is compiled as a SHARED lib.
 *
 * \verbatim
 *
 * To set your own QI_API macro, adapt the following line:
 *
 * .. code-block:: cpp
 *
 *   #define MYLIB_API QI_LIB_API(mylib)
 *
 * \endverbatim
 */
# define QI_API QI_LIB_API(qi)

# define QI_API_LEVEL 2

/* dynamic_casting template partial specializations
 * between shared objects poses problems with
 * clang under macos.
 * This macro is a workaround that seems to work
 * for now.
 */
#ifdef __clang__
# define QITYPE_TEMPLATE_API QI_API
#else
# define QITYPE_TEMPLATE_API
#endif



#endif  // _QI_API_HPP_
