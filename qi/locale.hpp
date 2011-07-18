#pragma once
/**
 * Author(s):
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief standard utf8 locale and codecvt
 */

#ifndef   	LOCALE_HPP_
# define   	LOCALE_HPP_

#include <locale>
#include <qi/config.hpp>

namespace qi {

  /** \brief utf8 std::locale implementation
   *
   *  return an std::locale object that can be used by stl (iostream, locale, ..)
   *  and std::locale compliant library like boost::filesystem.
   *
   *  this class allow convertion between utf8 and utf16.
   *
   *  \return utf8 implementation for std::locale
   */
  QI_API const std::locale                                 &utf8locale();


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
   *  this class allow convertion between utf8 and utf16.
   *
   *  \return utf8 implementation for std::codecvt<wchar_t, char, std::mbstate_t>
   */
  QI_API const codecvt_type &utf8facet();
}

#endif	    /* !LOCALE_PP_ */
