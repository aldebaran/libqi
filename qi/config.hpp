/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/** @file
 *  @brief dll import/export and compiler message
 */

#pragma once
#ifndef _LIBQI_QI_CONFIG_HPP_
#define _LIBQI_QI_CONFIG_HPP_

#include <qi/macro.hpp>

// qi_EXPORTS controls which symbols are exported when libqi
// is compiled as a SHARED lib.
// DO NOT USE OUTSIDE LIBQI
#ifdef qi_EXPORTS
# define QI_API QI_EXPORT_API
#elif defined(qi_IMPORTS)
# define QI_API QI_IMPORT_API
#else
# define QI_API
#endif


#define QI_API_LEVEL 1


#endif  // _LIBQI_QI_CONFIG_HPP_

