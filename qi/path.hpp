#pragma once
/*
 * Copyright (c) 2012, 2014 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_PATH_HPP_
# define _QI_PATH_HPP_

# include <string>
# include <vector>
# include <locale>
# include <iosfwd>
# include <boost/scoped_ptr.hpp>
# include <qi/api.hpp>

namespace boost { namespace filesystem {
  class path;
}}

namespace qi
{

  class PrivatePath;
  class Path;
  using PathVector = std::vector<Path>;

  /**
   * \brief The Path class allow handling filesystem path in a cross-platform maner.
   * \includename{qi/path.hpp}
   * The class assume that all string are encoded in UTF-8 if not specified otherwise.
   */
  class QI_API Path {
  public:
    /// Default Constructor
    /// \param unicodePath Path value as UTF-8 string.
    Path(const std::string& unicodePath = std::string());

    /// \param unicodePath Path value as UTF-8 string.
    Path(const char* unicodePath);

    /// Copy Constructor
    Path(const Path& path);

    /// Copy Constructor from boost filesystem.
    Path(const boost::filesystem::path& path);

    /// Default destructor
    ~Path();

    /// is the path empty?
    bool isEmpty() const;

    /// does this file exist?
    bool exists() const;

    /// is the path a directory?
    bool isDir() const;

    /// is the path a regular file?
    bool isRegularFile() const;

    /// is the path a symlink?
    bool isSymlink() const;

    /// @return the last component of the path as an UTF-8 string
    std::string filename() const;

    /// @return the extension of the last component of the path as an UTF-8 string
    std::string extension() const;

    /// @return a Path to the parent
    Path parent() const;

    /// @return an absolute Path of the current path
    Path absolute() const;

    /// @return a vector of files contained in the current path
    PathVector files() const;

    /// @return a vector of absolute path to files contained recursively in the current path
    PathVector recursiveFiles() const;

    /// @return a vector of dirs contained in the current path
    PathVector dirs() const;

    /// @return the path as an UTF-8 string
    explicit operator std::string() const;

    /// @return the path as an UTF-8 string
    std::string str() const;

    /// @return the path as a boost path representation
    operator boost::filesystem::path() const;

    /// @return the path as a boost path representation
    const boost::filesystem::path& bfsPath() const;

    /// @return A unicode-safe path from a string containing a native encoding path instead of UTF-8.
    static Path fromNative(const char* nativeCharsPath);

    /// @return A unicode-safe path from a string containing a native encoding path instead of UTF-8.
    static Path fromNative(const wchar_t* nativeCharsPath);

    /// @return A unicode-safe path from a string containing a native encoding path instead of UTF-8.
    static Path fromNative(const std::string& nativeCharsPath);

    /// @return A unicode-safe path from a string containing a native encoding path instead of UTF-8.
    static Path fromNative(const std::wstring& nativeCharsPath);

    /// concat two paths adding a directory separator between them
    Path operator/(const qi::Path& rhs) const;

    /// concat two paths adding a directory separator between them
    const Path& operator/=(const qi::Path& rhs) const;

    /// copy operator
    const Path& operator=(const qi::Path& rhs) const;

    bool operator==(const qi::Path& rhs) const;
    bool operator!=(const qi::Path& rhs) const;

    /// Standard output stream operator for logging.
    friend std::ostream& operator<<(std::ostream& output, const qi::Path& path)
    {
      output << path.str();
      return output;
    }

  private:
    boost::scoped_ptr<PrivatePath> _p;
  };


  /// Set of tools to handle SDK layouts.
  namespace path
  {
    /// Create a temporary directory that will be removed
    /// once you quit the scope
    class QI_API ScopedDir
    {
    public:
      /// Give a path to the constructor to specifed
      /// where you want the directory to be created
      /// Otherwise qi::os::mktmpdir will be used to
      /// create the directory
      ScopedDir(qi::Path path = qi::Path());
      /// Remove the directory
      ~ScopedDir();

      /// Implicit conversion operator to qi::Path
      operator qi::Path() const;

      /// Get the full path to the directory created
      const qi::Path& path() const;

    private:
      /// Remove current scoped directory
      /// retry is the number of try before quiting if the directory
      /// cannot be removed.
      void removeAll(int retry);

      qi::Path _path;
    };

    /// Create a temporary path to a file that will be removed
    /// once you quit the scope.
    /// Becareful it's only a path it's your call to open it to create it
    class QI_API ScopedFile
    {
    public:
      /// Give a path to the constructor to specifed
      /// where you want the file to be created.
      /// Otherwise the path to the file is construct with
      /// qi::os::mktmpdir / "tmpfile"
      /// If it already exists it will be remove.
      ScopedFile(qi::Path filePath = qi::Path());
      ~ScopedFile();

      /// Get the full path to the directory created
      const qi::Path& path() const;

    private:
      qi::path::ScopedDir _dir;
      qi::Path _path;
    };

    /**
     * \brief Return the default SDK prefix path.
     * \return The SDK prefix path.
     * It's always a complete, native path.
     */
    QI_API std::string sdkPrefix();

    /// \brief Implementation detail
    ///
    /// not thread-safe, must be kept internal
    namespace detail {

      /**
       * \brief Return the SDK prefixes list.
       * It's always complete, native paths.
       */
      QI_API std::vector<std::string> getSdkPrefixes();

      /**
       * \brief Add a new SDK prefix to the list of searchable prefixes.
       *
       * A default SDK prefix is computed using argc, argv when calling
       * qi::Application app(argc, argv).
       *
       * After calling this function, the new SDK prefix will be taken
       * into account by the other methods.
       * \param prefix The new prefix to add (in UTF-8).
       */
      QI_API void addOptionalSdkPrefix(const char* prefix);

      /**
       * \brief Reset the list of additional SDK prefixes.
       *
       * Reset all the SDK added with qi::path::addOptionalSdkPrefix.
       * The list of SDK prefixes will only contain the default SDK
       * prefix.
       */
      QI_API void clearOptionalSdkPrefix();

      /**
       * \brief Set the writable files path for users.
       * \param path Path to the new writable data path
       * \warning This method is only meant to be used in tests.
       */
      QI_API void setWritablePath(const std::string &path);

      /** Normalize a path by deducing ".." and '.' and replacing '\\' by '/'.
          @param path Path to normalize.
          @return A normalized copy of the path passed in arguments.
      */
      QI_API Path normalize(const Path& path);
    }

    /**
     * \brief Look for a binary.
     * \param name The full name of the binary, or just the name.
     * \param searchInPath if true, also search for the binary in the $PATH
     * directories.
     * \return The complete, native path to the file found,
     * an empty string otherwise.
     *
     * \verbatim
     * This will search in all SDK prefixes for a file named 'name'.
     * It will then add '.exe' suffix if needed.
     * (without '.exe') (in UTF-8).
     * \endverbatim
     */
    QI_API std::string findBin(const std::string& name, bool searchInPath=false);

    /**
     * \brief Look for a library.
     * \param name The full name of the library, or just the name.
     * \return The complete, native path to the file found,
     * an empty string otherwise.
     *
     * \verbatim
     * This will search in all SDK prefixes for a file named 'name'.
     * It will then add 'lib' prefix, and appropriated suffixes
     * ('.dll' on windows, '.so' on linux, '.dylib' on mac).
     * (without '.dll', '.so') (in UTF-8).
     *
     * You can specify subdirectories using "/" as directory separator
     * (in UTF-8).
     * \endverbatim
     */
    QI_API std::string findLib(const std::string& name);


    /**
     * \brief Look for a configuration file.
     * \param applicationName Name of the application.
     * \param filename Name of the file to look for.
     * You can specify subdirectories using "/" as directory separator.
     * \param excludeUserWritablePath If true, findConf() won't search into userWritableConfPath.
     * \return The complete, native path of the file if it was found,
     * an empty string otherwise.
     *
     * \verbatim
     * The file is searched in a list of possible directories,
     * the first match is returned.
     *
     * The list of paths is constructed like this:
     *
     * - first, a standard path in the home directory (like
     *   ~/.config/<applicationName>/<filename>)
     * - then: <sdk_prefix>/etc/<applicationName>/<filename> for each known SDK
     *   prefix.
     * - then a standard path in the system. (like
     *   /etc/<applicationName>/<filename>)
     * \endverbatim
     */
    QI_API std::string findConf(const std::string& applicationName,
                                const std::string& filename,
                                bool excludeUserWritablePath = false);

    /**
     * \brief Look for a file in all dataPaths(applicationName) directories,
     * return the first match.
     * \param applicationName Name of the application.
     * \param filename Name of the file to look for.
     * You can specify subdirectories using "/" as directory separator.
     * \param excludeUserWritablePath If true, findData() won't search into userWritableDataPath.
     * \return The complete, native path of the file if it was found,
     * an empty string otherwise.
     * \remark For automatic tests, you can modify the user data path
     * to find data in (e.g. ~/.local/share/ on Unix systems)
     * with :cpp:func:`qi::path::detail::setWritablePath`
     *
     * \verbatim
     * The file is searched in a list of possible directories, provided by the
     * :cpp:func:`qi::path::dataPaths(const std::string&)`.
     * The first match is returned.
     *
     * For instance if you have the following files on a unix system
     *
     * - ~/.local/share/foo/models/nao.xml
     * - /usr/share/foo/models/nao.xml
     *
     * then listData("foo", "models/nao.xml") will return
     *
     * - ~/.local/share/foo/models/nao.xml
     * \endverbatim
     */
    QI_API std::string findData(const std::string& applicationName,
                                const std::string& filename,
                                bool excludeUserWritablePath = false);


    /**
     * \brief List data files matching the given pattern in all dataPaths(applicationName)
     * directories. For each match, return the occurence from the first dataPaths prefix.
     * Directories are discarded.
     * \param applicationName Name of the application.
     * \param pattern wilcard pattern of the files to look for.
     * \param excludeUserWritablePath If true, listData() won't search into userWritableDataPath.
     * You can specify subdirectories using "/" as directory separator.
     * \return An std::vector of the complete, native paths of the files that matched.
     *
     * \verbatim
     * Matches are searched in a list of possible directories, provided by the
     * :cpp:func:`qi::path::dataPaths(const std::string&)`.
     * When several matches collide, the first one is
     * returned.
     *
     * For instance if you have the following files on a unix system
     *
     * - ~/.local/share/foo/models/mynao.xml
     * - ~/.local/share/foo/models/myromeo_with_laser_head.xml
     * - /usr/share/foo/models/mynao.xml
     * - /usr/share/foo/models/myromeo.xml
     *
     * then listData("foo", "models/my*.xml") will return
     *
     * - ~/.local/share/foo/models/mynao.xml
     * - ~/.local/share/foo/models/myromeo_with_laser_head.xml
     * - /usr/share/foo/models/myromeo.xml
     *
     * note that /usr/share/foo/models/mynao.xml is not returned because a nao.xml file is already matched.
     * \endverbatim
     */
    QI_API std::vector<std::string> listData(const std::string& applicationName,
                                             const std::string& pattern="*",
                                             bool excludeUserWritablePath = false);

    /** same as listData but for libraries
     */
    QI_API std::vector<std::string> listLib(const std::string& subfolder,
                                            const std::string& pattern="*");

    /**
     * \brief Get the list of directories used when searching for configuration files for the given application name.
     * \param applicationName Name of the application.
     * \param excludeUserWritablePath If true, confPaths() won't include userWritableConfPath.
     * \return List of configuration directories.
     *
     * \verbatim
     * This is used by the :cpp:func:`qi::path::findConf(const std::string&, const std::string&)`.
     *
     * .. warning:: You should not assume those directories exist, nor
     *    that they are writeable.
     * \endverbatim
     */
    QI_API std::vector<std::string> confPaths(const std::string& applicationName="",
                                              bool excludeUserWritablePath = false);

    /**
     * \brief Get the list of directories used when searching for data files for the given application name.
     * \param applicationName Name of the application.
     * \param excludeUserWritablePath If true, dataPaths() won't include userWritableDataPath.
     * \return A list of directories.
     * \remark For automatic tests, you can modify the user data path
     * (e.g ~/.local/share/ on Unix systems) with :cpp:func:`qi::path::detail::setWritablePath`
     *
     * \verbatim
     * This is used by the :cpp:func:`qi::path::findData(const std::string&, const std::string&)`
     * and the :cpp:func:`qi::path::listData(const std::string&, const std::string&)`.
     *
     * The list of paths is constructed like this:
     *
     * - first, a standard path in the home directory (like
     *   ~/.local/share/<applicationName>/<filename>)
     * - then <sdk_prefix>/share/<applicationName>/<filename> for each known SDK
     *   prefix.
     *
     * .. warning:: You should not assume those directories exist,
     *    nor that they are writeable.
     * \endverbatim
     */
    QI_API std::vector<std::string> dataPaths(const std::string& applicationName="",
                                              bool excludeUserWritablePath = false);

    /**
     * \brief Get the list of directories used when searching for binaries.
     * \return A list of directories.
     *
     * \verbatim
     * This is used by the :cpp:func:`qi::path::findBin(const std::string&)`.
     *
     * .. warning:: You should not assume those directories exist, nor that they are
     *    writeable.
     * \endverbatim
     */
    QI_API std::vector<std::string> binPaths();

    /**
     * \brief Get the list of directories used when searching for libraries.
     * \return A list of directories.
     *
     * \verbatim
     * This is used by the :cpp:func:`qi::path::findLib(const std::string&)`.
     *
     * .. warning:: You should not assume those directories exist, nor that they are
     *    writeable.
     * \endverbatim
     */
    QI_API std::vector<std::string> libPaths();

    /**
     * \brief Get the writable data files path for users.
     * \param applicationName The name of the application.
     * \param filename The filename.
     * \return The directory or the file.
     *
     * \verbatim
     * If filename is empty, return the directory in which to write.
     * Otherwise the path is constructed like this:
     *
     * Linux
     *    <home>/.local/share/<applicationName>/<filename>
     *
     * Windows
     *    %AppData%\<applicatioName>\<filename>
     *
     * You can specify subdirectories using "/" as directory separator.
     * \endverbatim
     */
    QI_API std::string userWritableDataPath(const std::string& applicationName,
                                            const std::string& filename);

    /**
     * \brief Get the writable configuration files path for users.
     * \param applicationName The name of the application.
     * \param filename The filename.
     * \return The directory or the file.
     *
     * \verbatim
     * If filename is empty, return the directory
     * in which to write. Otherwise the path is constructed like this:
     *
     * Linux
     *    <home>/.config/<applicatioName>/<filename>
     *
     * Windows
     *    %AppData%\<applicatioName>\<filename>
     *
     * You can specify subdirectories using "/" as directory separator.
     * \endverbatim
     */
    QI_API std::string userWritableConfPath(const std::string& applicationName,
                                            const std::string& filename="");

    /**
     * \brief Convert given path into DOS 8.3 path if it exists, else returns empty string (Windows only).
     *
     * To use some API that doesn't support unicode on Windows, it is
     * possible to convert a unicode path to an existing file into a DOS
     * path without any accentuated characters.
     * (for ex. "C:\test àé\" becomes "C:\TEST~1\" if it already exists)
     *
     * On other platforms, simply return pathString.
     */
    QI_API std::string convertToDosPath(const std::string &pathString);

    /**
     * \brief Set the writable files path for users.
     * \param path Path to the new writable data path
     * \deprecated since 2.2 Use qi::path::detail::setWritablePath instead
     * \warning This method is only meant to be used in tests.
     */
    QI_API QI_API_DEPRECATED_MSG(Use 'detail::setWritablePath' instead)
    void setWritablePath(const std::string &path);
  }

  /**
   * \brief Standard std::codecvt type accepted by STL and boost.
   *
   * Typedef for std::codecvt<wchar_t, char, std::mbstate_t> that can be used
   * with boost::filesystem::path and std::locale.
   */
  using codecvt_type = std::codecvt<wchar_t, char, std::mbstate_t>;

  /**
   * \brief UTF-8 facet object getter.
   * \return UTF-8 implementation for std::codecvt<wchar_t, char, std::mbstate_t>
   *
   * Return a facet object that can be used by stl (iostream, locale, ...)
   * and std::locale compliant library like boost::filesystem.
   *
   * This class allow conversion between UTF-8 (char) and UTF-16/UTF-32 (wchar).
   */
  QI_API codecvt_type &unicodeFacet();
}

#endif  // _QI_PATH_HPP_
