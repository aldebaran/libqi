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
# include <qi/config.hpp>

/**
 *  @namespace qi::path
 * \ingroup qipath
 *  @brief Set of tools to handle SDK layouts.
 *
 * \warning Every string MUST be encoded in UTF8. Every returned string are encoded in UTF-8.
 *
 */
namespace qi
{
  namespace path
  {

    /**
     * \brief Return the default SDK prefix path.
     * \ingroup qipath
     * It's always a complete, native path.
     */
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


    /** \brief Look for a binary.
     * \ingroup qipath
     * This will search in all SDK prefixes for a file named 'name'.
     * It will then add '.exe' suffix if needed.
     * \param name The full name of the binary, or just the name
     * (without '.exe') (in UTF-8).
     * \return The complete, native path to the file found,
     * an empty string otherwise.
     */
    QI_API std::string findBin(const std::string& name);

    /** \brief Look for a library.
     * \ingroup qipath
     *
     * This will search in all SDK prefixes for a file named 'name'.
     * It will then add 'lib' prefix, and appropriated suffixes
     * ('.dll' on windows, '.so' on linux, '.dylib' on mac).
     * \param name The full name of the library, or just the name
     * (without '.dll', '.so') (in UTF-8).
     * \return The complete, native path to the file found,
     * an empty string otherwise.
     *
     * You can specify subdirectories using "/" as directory separator
     * (in UTF-8).
     */
    QI_API std::string findLib(const std::string& name);

    /**
     * \brief Look for a configuration file.
     * \ingroup qipath
     *
     * The file is searched in a list of possible directories,
     * the first match is returned.
     * \verbatim
     * The list of paths is constructed like this:
     * - first, a standard path in the home directory (like
     *  ~/.config/<applicationName>/<filename>)
     * - then:
     *     <sdk_prefix>/etc/<applicationName>/<filename>
     *     for each known SDK prefix
     * - then a standard path in the system. (like
     *   /etc/<applicationName>/<filename>)
     * \endverbatim
     * @param applicationName Name of the application (in UTF-8).
     * @param filename Name of the file to look for (in UTF-8).
     * You can specify subdirectories using "/" as directory separator.
     * @return The complete, native path of the file if it was found,
     * an empty string otherwise.
     */
    QI_API std::string findConf(const std::string& applicationName,
                                const std::string& filename);

    /**
     * \brief Look for a data file.
     * \ingroup qipath
     *
     * The file is search in a list of possible directories,
     * the first match is returned.
     * The list of paths is constructed like this:
     * \verbatim
     * - first, a standard path in the home directory (like
     *  ~/.local/share/<applicationName>/<filename>)
     * - then
     *   <sdk_prefix>/share/<applicationName>/<filename>
     *     for each known SDK prefix
     * \endverbatim
     *
     * @param applicationName Name of the application (in UTF-8).
     * @param filename Name of the file to look for (in UTF-8).
     * You can specify subdirectories using "/" as directory separator.
     * @return The complete, native path of the file if it was found,
     * an empty string otherwise.
     */
    QI_API std::string findData(const std::string& applicationName,
                                const std::string& filename);


    /**
     * \brief Get the list of directories used when searching for
     *  configuration files for the given application name.
     * \ingroup qipath
     *
     * This is used by the qi::path::findConfigurationPath method.
     * @param applicationName Name of the application (in UTF-8).
     * @return List of configuration directories.
     *
     * Warning: you should not assume those directories exist, nor that they are
     * writeable.
     */
    QI_API std::vector<std::string> confPaths(const std::string& applicationName="");

    /**
     * \brief Get the list of directories used when searching for data files
     * for the given application name.
     * \ingroup qipath
     *
     * This is used by the qi::path::findDataPath method.
     * @param applicationName Name of the application (in UTF-8).
     * @return A list of directories.
     *
     * Warning: you should not assume those directories exist, nor that they are
     * writeable.
     */
    QI_API std::vector<std::string> dataPaths(const std::string& applicationName="");

    /**
     * \brief Get the list of directories used when searching for
     *  binaries.
     * \ingroup qipath
     *
     * This is used by the qi::path::findBin() method.
     * @return A list of directories.
     *
     * Warning: you should not assume those directories exist, nor that they are
     * writeable.
     */
    QI_API std::vector<std::string> binPaths();

    /**
     * \brief Get the list of directories used when searching for
     *  libraries.
     * \ingroup qipath
     *
     * This is used by the qi::path::findLib() method.
     * @return A list of directories.
     *
     * Warning: you should not assume those directories exist, nor that they are
     * writeable.
     */
    QI_API std::vector<std::string> libPaths();


    /**
     * \brief Get the writable data files path for users.
     * \ingroup qipath
     *
     * @param applicationName The name of the application (in UTF-8).
     * @param filename If filename is empty, return the directory
     * in which to write. Otherwise the path is constructed like this:
     * \verbatim
     * <home>/.local/share/<applicatioName>/<filename> (on linux)
     * %AppData%\<applicatioName>\<filename> (windows)
     * You can specify subdirectories using "/" as directory separator
     * (in UTF-8).
     * \endverbatim
     */
    QI_API std::string userWritableDataPath(const std::string& applicationName,
                                            const std::string& filename);

    /**
     * \brief Get the writable configuration files path for users.
     * \ingroup qipath
     *
     * @param applicationName The name of the application.
     * @param filename If filename is empty, return the directory
     * in which to write. Otherwise the path is constructed like this:
     * \verbatim
     * <home>/.config/<applicatioName>/<filename> (on linux)
     * %AppData%\<applicatioName>\<filename> (windows)
     * You can specify subdirectories using "/" as directory separator
     * (in UTF-8).
     * \endverbatim
     */
    QI_API std::string userWritableConfPath(const std::string& applicationName,
                                            const std::string& filename="");


  };
};

#endif  // _LIBQI_QI_PATH_HPP_
