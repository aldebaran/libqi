/**
 * Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief standard utf8 locale and codecvt
 */

#pragma once
#ifndef _LIBQI_QI_LOCALE_HPP_
#define _LIBQI_QI_LOCALE_HPP_

#include <locale>
#include <qi/config.hpp>

namespace qi {


  /** \brief standard std::codecvt accept by STL and boost.
   * typedef for std::codecvt<wchar_t, char, std::mbstate_t>
   * could be used with boost::filesystem::path or std::locale.
   */
  typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_type;

  /** \brief utf8 facet
   *
   *  return a facet object that can be used by stl (iostream, locale, ..)
   *  and std::locale compliant library like boost::filesystem.
   *
   *  this class allow convertion between utf8(char) and utf16/32(wchar).
   *
   *  \return utf8 implementation for std::codecvt<wchar_t, char, std::mbstate_t>
   */
  QI_API const codecvt_type &unicodeFacet();

}

#endif  // _LIBQI_QI_LOCALE_HPP_
