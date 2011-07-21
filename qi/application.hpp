#pragma once
/**
 * Author(s):
 *  - Herve CUCHE <hcuche@aldebaran-robotics.com>
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief Tools to init SDK layouts & path.
 */

#ifndef _QI_APPLICATION_HPP_
#define _QI_APPLICATION_HPP_

# include <qi/config.hpp>

/**
 *  @namespace qi
 */
namespace qi
{
  /** \brief Store argc and argv
   */
  QI_API void init(int argc, char *argv[]);

  /** \brief Get argc
   * \return return argc if qi::init is call, -1 otherwise
   */
  QI_API int argc();

  /** \brief Get argv
   * \return return argv if qi::init is call, NULL otherwise
   */
  QI_API const char** argv();

  /** \brief Return the current program.
   * Computed using specific OS API:
   * Apple  : _NSGetExecutablePath
   * Linux  : reading "/proc/self/exe"
   * Windows: GetModuleFileName
   *
   * If the former API fail it will try to guess the value from argv[0].
   * For this method to work qi::init(argc, argv) should have been called in the main.
   *  \return full path to the current running program
   */
  QI_API const char *program();
};

#endif  // _QI_APPLICATION_HPP_
