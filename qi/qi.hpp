#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_QI_HPP_
# define _QI_QI_HPP_

# include <qi/api.hpp>
# include <locale>

/**
 * \brief Tools to handle utf8 charset.
 * \includename{qi/qi.hpp}
 */

namespace qi {
  /**
   * \brief Standard std::codecvt type accepted by STL and boost.
   *
   * Typedef for std::codecvt<wchar_t, char, std::mbstate_t> that can be used
   * with boost::filesystem::path and std::locale.
   */
  typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_type;

  /**
   * \brief UTF-8 facet object getter.
   * \return UTF-8 implementation for std::codecvt<wchar_t, char, std::mbstate_t>
   *
   * Return a facet object that can be used by stl (iostream, locale, ...)
   * and std::locale compliant library like boost::filesystem.
   *
   * This class allow conversion between UTF-8 (char) and UTF-16/UTF-32 (wchar).
   */
  QI_API const codecvt_type &unicodeFacet();
}

#endif  // _QI_QI_HPP_
