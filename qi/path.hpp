/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/** @file
 *  @brief find bin/lib/data/conf for the current application
 */

#pragma once
#ifndef _LIBQI_QI_PATH_HPP_
#define _LIBQI_QI_PATH_HPP_

# include <string>
# include <vector>
# include <qi/api.hpp>

namespace qi
{
  /// Set of tools to handle SDK layouts.
  namespace path
  {

    /// Return the default SDK prefix path.
    QI_API std::string sdkPrefix();

    // not thread-safe, must be kept internal
    namespace detail {

      /**
       * \brief Return the SDK prefixes list.
       * It's always complete, native paths.
       */
      QI_API std::vector<std::string> getSdkPrefixes();

      /** \brief Add a new SDK prefix to the list of searchable prefixes.
       *
       * A default SDK prefix is computed using argc, argv when calling
       * qi::init().
       *
       * After calling this function, the new SDK prefix will be taken
       * into account by the other methods.
       * \param prefix The new prefix to add (in UTF-8).
       */
      QI_API void addOptionalSdkPrefix(const char* prefix);

      /** \brief Reset the list of additional SDK prefixes.
       *
       * Reset all the SDK added with qi::path::addOptionalSdkPrefix.
       * The list of SDK prefixes will only contain the default SDK
       * prefix.
       */
      QI_API void clearOptionalSdkPrefix();

    }

    /// Look for a binary.
    QI_API std::string findBin(const std::string& name);

    /// Look for a library.
    QI_API std::string findLib(const std::string& name);

    /// Look for a configuration file.
    QI_API std::string findConf(const std::string& applicationName,
                                const std::string& filename);

    /// Look for a data file.
    QI_API std::string findData(const std::string& applicationName,
                                const std::string& filename);


    /// Get the list of directories used when searching for configuration files for the given application name.
    QI_API std::vector<std::string> confPaths(const std::string& applicationName="");

    /// Get the list of directories used when searching for data files for the given application name.
    QI_API std::vector<std::string> dataPaths(const std::string& applicationName="");

    /// Get the list of directories used when searching for binaries.
    QI_API std::vector<std::string> binPaths();

    /// Get the list of directories used when searching for libraries.
    QI_API std::vector<std::string> libPaths();


    /// Set the writable files path for users.
    QI_API void setWritablePath(const std::string &path);

    /// Get the writable data files path for users.
    QI_API std::string userWritableDataPath(const std::string& applicationName,
                                            const std::string& filename);

    /// Get the writable configuration files path for users.
    QI_API std::string userWritableConfPath(const std::string& applicationName,
                                            const std::string& filename="");


  }
}

#endif  // _LIBQI_QI_PATH_HPP_
