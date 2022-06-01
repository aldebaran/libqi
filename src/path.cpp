/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "sdklayout.hpp"

#include <numeric>

#include <qi/application.hpp>
#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/atomic.hpp>

#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#ifdef _WIN32
#  include <windows.h>
#  include "os_win32.hpp"
#endif

qiLogCategory("qi.path");

namespace bfs = boost::filesystem;

namespace qi
{

  class PrivatePath {
  public:
    PrivatePath(const std::string& unicodePath)
      : path(unicodePath, qi::unicodeFacet())
    {}

    PrivatePath(const bfs::path& path)
      : path(path)
    {}

    bfs::path path;
  };

  Path::Path(const std::string& unicodePath)
    : _p(new PrivatePath(unicodePath))
  {}

  Path::Path(const char* unicodePath)
    : _p(new PrivatePath(std::string(unicodePath)))
  {}

  Path::Path(const Path& path)
    : _p(new PrivatePath(path._p->path))
  {}

  Path::Path(const bfs::path& path)
    : _p(new PrivatePath(path))
  {

  }

  Path::~Path()
  {
  }

  bool Path::isEmpty() const
  {
    return _p->path.empty();
  }

  bool Path::exists() const
  {
    return bfs::exists(_p->path);
  }

  bool Path::isDir() const
  {
    boost::system::error_code ec;
    return bfs::is_directory(_p->path, ec);
  }

  bool Path::isRegularFile() const
  {
    boost::system::error_code ec;
    return bfs::is_regular_file(_p->path, ec);
  }

  bool Path::isSymlink() const
  {
    boost::system::error_code ec;
    return bfs::is_symlink(_p->path, ec);
  }

  std::string Path::extension() const
  {
    return _p->path.extension().string(qi::unicodeFacet());
  }

  Path Path::parent() const
  {
    return _p->path.parent_path();
  }

  std::string Path::filename() const
  {
    return _p->path.filename().string(qi::unicodeFacet());
  }

  Path Path::absolute() const
  {
    return bfs::absolute(_p->path);
  }

  PathVector Path::files() const
  {
    PathVector ret;
    bfs::directory_iterator dit(_p->path);

    for (; dit != bfs::directory_iterator(); ++dit) {
      if (bfs::is_regular_file(*dit))
        ret.push_back(Path(*dit));
    }
    return ret;
  }

  PathVector Path::recursiveFiles() const
  {
    PathVector ret;
    bfs::recursive_directory_iterator dit(_p->path);

    for (; dit != bfs::recursive_directory_iterator(); ++dit) {
      if (bfs::is_regular_file(*dit))
        ret.push_back(Path(*dit));
    }
    return ret;
  }

  PathVector Path::dirs() const
  {
    PathVector ret;
    bfs::directory_iterator dit(_p->path);

    for (; dit != bfs::directory_iterator(); ++dit) {
      if (bfs::is_directory(*dit))
        ret.push_back(Path(*dit));
    }
    return ret;
  }

  Path Path::operator/(const Path &rhs) const
  {
    return _p->path / rhs._p->path;
  }

  const Path& Path::operator/=(const Path &rhs) const
  {
    _p->path /= rhs._p->path;
    return *this;
  }

  const Path& Path::operator=(const Path &rhs) const
  {
    _p->path = rhs._p->path;
    return *this;
  }

  bool Path::operator==(const qi::Path& rhs) const
  {
    return _p->path == rhs._p->path;
  }

  bool Path::operator!=(const qi::Path& rhs) const
  {
    return _p->path != rhs._p->path;
  }

  Path::operator std::string() const
  {
    return _p->path.string(qi::unicodeFacet());
  }

  Path::operator boost::filesystem::path() const
  {
    return _p->path;
  }

  std::string Path::str() const
  {
    return _p->path.string(qi::unicodeFacet());
  }

  const boost::filesystem::path& Path::bfsPath() const
  {
    return _p->path;
  }

  Path Path::fromNative(const char* nativeCharsPath)
  {
    return bfs::path(nativeCharsPath);
  }

  Path Path::fromNative(const wchar_t* nativeCharsPath)
  {
    return bfs::path(nativeCharsPath);
  }

  Path Path::fromNative(const std::string& nativeCharsPath)
  {
    return bfs::path(nativeCharsPath);
  }

  Path Path::fromNative(const std::wstring& nativeCharsPath)
  {
    return bfs::path(nativeCharsPath);
  }



  SDKLayout* gInstance = NULL;
  SDKLayout* getInstance();

  //only needed to make valgrind happy.
  static void cleanupSdkLayout() {
    if (!qi::os::getenv("VALGRIND").empty()) {
      delete gInstance;
      gInstance = 0;
    }
  }

  QI_AT_EXIT(&cleanupSdkLayout);

  namespace path
  {

    // ScopedDir
    ScopedDir::ScopedDir(qi::Path path)
      : _path(path)
    {
      if (_path.isEmpty())
        _path = qi::Path(qi::os::mktmpdir());
    }

    ScopedDir::~ScopedDir()
    {
      removeAll(10);
    }

    ScopedDir::operator qi::Path() const
    {
      return _path;
    }

    const Path& ScopedDir::path() const
    {
      return _path;
    }


    void ScopedDir::removeAll(int retry)
    {
      bool success = false;
      std::string err;
      while (!success && retry != 0)
      {
        bfs::path p(_path.str(), qi::unicodeFacet());
        try
        {
          bfs::remove_all(p);
          success = true;
        }
        catch (const bfs::filesystem_error &e)
        {
          err = e.what();
          retry--;
        }
      }
      if (!success)
      {
        qiLogError() << "Could not remove " << _path.str()
                     << ":" << err;
      }
    }

    // ScopedFile
    ScopedFile::ScopedFile(qi::Path path)
      : _path(path)
    {
      if (_path.isEmpty())
        _path = _dir.path() / "tmpfile";
    }

    // if path is !empty the directory created by ScopedDir will be
    // delete when existing the scope.
    ScopedFile::~ScopedFile()
    {
      bfs::path p(_path.str(), qi::unicodeFacet());
      bfs::remove(p);
    }

    const Path& ScopedFile::path() const
    {
      return _path;
    }

    boost::filesystem::path absPath(const std::string &pathString)
    {
      boost::filesystem::path path(pathString, qi::unicodeFacet());
      return boost::filesystem::system_complete(path).make_preferred();
    }

    namespace detail {

      void addOptionalSdkPrefix(const char *prefix)
      {
        return getInstance()->addOptionalSdkPrefix(prefix);
      }

      void clearOptionalSdkPrefix()
      {
        return getInstance()->clearOptionalSdkPrefix();
      }


      std::vector<std::string> getSdkPrefixes()
      {
        return getInstance()->getSdkPrefixes();
      }

      void setWritablePath(const std::string &path)
      {
        qiLogVerbose() << "Writable path set to " << path;
        getInstance()->setWritablePath(path);
      }

#ifdef _WIN32
      std::string dosCompatiblePath(const std::string &pathString)
      {
        // Some libs of our toolchain do not support UTF-8 on Windows
        // for Py_SetPythonHome for example, we need to clean path from UTF-8 chars
        // to make it usable.
        // The fix found is to convert path into short DOS 8.3 path
        long length = 0;
        wchar_t* buffer = NULL;
        boost::filesystem::path path(pathString, qi::unicodeFacet());

        // first, get size necessary for wchar_t* buffer
        length = GetShortPathNameW(path.wstring(qi::unicodeFacet()).c_str(), NULL, 0);
        if(length == 0)
        {
          const auto error = qi::os::lastErrorMessage();
          qiLogVerbose() << "Cannot retrieve short path length for '"
                         << pathString.c_str()
                         << "', error: " << error;
          return std::string();
        }

        // now, get the path compatible DOS 8.3, ASCII compliant
        buffer = new wchar_t[length];
        length = GetShortPathNameW(path.wstring(qi::unicodeFacet()).c_str(), buffer, length);
        if(length == 0)
        {
          const auto error = qi::os::lastErrorMessage();
          qiLogVerbose() << "Cannot retrieve short path for '"
                         << pathString.c_str()
                         << "', error: " << error;
          return std::string();
        }

        // whatever the buffer is wchar_t*, all characters inside are ASCII
        boost::filesystem::path shortPath(std::wstring(buffer), qi::unicodeFacet());
        delete [] buffer;

        return shortPath.string(qi::unicodeFacet());
      }
#endif
      namespace {
        static boost::filesystem::path normalizeCombined(boost::filesystem::path path1,
                                                        boost::filesystem::path path2)
        {
          if (*path2.begin() == ".")
            return path1;
          if (*path2.begin() == "..")
            return path1.parent_path();
          else
            return path1 /= path2;
        }
      }
      Path normalize(const Path& path)
      {
        boost::filesystem::path p = std::accumulate(path.bfsPath().begin(), path.bfsPath().end(),
                                                    boost::filesystem::path(), normalizeCombined);
        return p.make_preferred();
      }

    }

    std::string sdkPrefix()
    {
      return getInstance()->sdkPrefix();
    }

    std::string findBin(const std::string &name, bool searchInPath)
    {
      return getInstance()->findBin(name, searchInPath);
    }

    std::string findLib(const std::string &name)
    {
      return getInstance()->findLib(name);
    }

    std::string findConf(const std::string &applicationName,
                         const std::string &filename,
                         bool excludeUserWritablePath)
    {
      if(filename == "") {
        qiLogError() << "Filename cannot be empty!";
        return std::string();
      }
      return getInstance()->findConf(applicationName, filename, excludeUserWritablePath);
    }

    std::string findData(const std::string &applicationName,
                         const std::string &filename,
                         bool excludeUserWritablePath)
    {
      if (filename == "") {
        qiLogError() << "Filename cannot be empty!";
        return std::string();
      }

      return getInstance()->findData(applicationName, filename, excludeUserWritablePath);
    }

    std::vector<std::string> listLib(const std::string &subfolder,
                                     const std::string &pattern)
    {
      return getInstance()->listLib(subfolder, pattern);
    }

    std::vector<std::string> listData(const std::string &applicationName,
                                      const std::string &pattern,
                                      bool excludeUserWritablePath)
    {
      return getInstance()->listData(applicationName, pattern, excludeUserWritablePath);
    }

    std::vector<std::string> confPaths(const std::string &applicationName,
                                       bool excludeUserWritablePath)
    {
      return getInstance()->confPaths(applicationName,
                                      excludeUserWritablePath);
    }

    std::vector<std::string> dataPaths(const std::string &applicationName,
                                       bool excludeUserWritablePath)
    {
      return getInstance()->dataPaths(applicationName, excludeUserWritablePath);
    }

    std::vector<std::string> binPaths()
    {
      return getInstance()->binPaths();
    }

    std::vector<std::string> libPaths()
    {
      return getInstance()->libPaths();
    }

    void setWritablePath(const std::string &path)
    {
      qi::path::detail::setWritablePath(path);
    }

    std::string userWritableDataPath(const std::string &applicationName,
                                     const std::string &filename)
    {
      return getInstance()->userWritableDataPath(applicationName, filename);
    }

    std::string userWritableConfPath(const std::string &applicationName,
                                     const std::string &filename)
    {
      return qi::getInstance()->userWritableConfPath(applicationName, filename);
    }

    std::string convertToDosPath(const std::string &pathString)
    {
#ifdef _WIN32
      // Windows cannot describe an unicode path with a narrow string.
      // Yet, for DOS compatibility, it can return an ASCII narrow string
      // describing the path, provided a file/directory already exists at that path.
      return detail::dosCompatiblePath(pathString);
#else
      // just ignore
      return pathString;
#endif
    }
  }

  SDKLayout* getInstance()
  {
    QI_THREADSAFE_NEW(gInstance);
    return gInstance;
  }

  namespace {
    _QI_COMMAND_LINE_OPTIONS(
      "Chrooting",
      ("writable-path", value<std::string>()->notifier(&qi::path::detail::setWritablePath), "Set the writable path.")
      )
  }
}
