/**
 * Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief dll import/export and compiler message
 */

#pragma once
#ifndef _LIBQI_QI_CONFIG_HPP_
#define _LIBQI_QI_CONFIG_HPP_

#include <qi/api.hpp>

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


/**
 * \def QI_API_LEVEL
 * \brief Define the API version of the Platform
 * <b>Removed libraries</b>
 * <table border>
 * <tr>
 *    <td><b> API Level </b></td>
 *    <td><b> Version   </b></td>
 * </tr>
 * <tr><td> 0 / undefined </td> <td> Pre 1.12 release   </td></tr>
 * <tr><td> 1             </td> <td> NAOqi release 1.12 </td></tr>
 * </table>
 */
#define QI_API_LEVEL 1


#endif  // _LIBQI_QI_CONFIG_HPP_

