/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "sdklayout.hpp"
#include "filesystem.hpp"

#include <qi/application.hpp>
#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#ifdef _WIN32
  #include <windows.h>
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

  Path::Path(PrivatePath* p)
    : _p(p)
  {}

  Path::Path(const std::string& unicodePath)
    : _p(new PrivatePath(unicodePath))
  {}

  Path::Path(const char* unicodePath)
    : _p(new PrivatePath(std::string(unicodePath)))
  {}

  Path::Path(const Path& path)
    : _p(new PrivatePath(path._p->path))
  {}

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

  Path Path::parent()
  {
    return Path(new PrivatePath(_p->path.parent_path()));
  }

  std::string Path::filename() const
  {
    return _p->path.filename().string(qi::unicodeFacet());
  }

  Path Path::absolute()
  {
    return Path(new PrivatePath(bfs::absolute(_p->path)));
  }

  PathVector Path::files()
  {
    PathVector ret;
    bfs::directory_iterator dit(_p->path);

    for (; dit != bfs::directory_iterator(); ++dit) {
      if (bfs::is_regular_file(*dit))
        ret.push_back(Path(new PrivatePath(*dit)));
    }
    return ret;
  }

  PathVector Path::recursiveFiles()
  {
    PathVector ret;
    bfs::recursive_directory_iterator dit(_p->path);

    for (; dit != bfs::recursive_directory_iterator(); ++dit) {
      if (bfs::is_regular_file(*dit))
        ret.push_back(Path(new PrivatePath(*dit)));
    }
    return ret;
  }

  PathVector Path::dirs()
  {
    PathVector ret;
    bfs::directory_iterator dit(_p->path);

    for (; dit != bfs::directory_iterator(); ++dit) {
      if (bfs::is_directory(*dit))
        ret.push_back(Path(new PrivatePath(*dit)));
    }
    return ret;
  }

  Path Path::operator/(const Path &rhs) const
  {
    return Path(new PrivatePath(_p->path / rhs._p->path));
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

  Path::operator std::string() const
  {
    return _p->path.string(qi::unicodeFacet());
  }

  std::string Path::str() const
  {
    return _p->path.string(qi::unicodeFacet());
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
        // Some libs of our toolchain does not support UTF-8 on Windows
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
          qiLogVerbose() << "Cannot retrieve short path for "
                         << pathString.c_str();
          return std::string();
        }

        // now, get the path compatible DOS 8.3, ASCII compliant
        buffer = new wchar_t[length];
        length = GetShortPathNameW(path.wstring(qi::unicodeFacet()).c_str(), buffer, length);
        if(length == 0)
        {
          qiLogVerbose() << "Cannot retrieve short path for "
                         << pathString.c_str();
          return std::string();
        }

        // whatever the buffer is wchar_t*, all characters inside are ASCII
        boost::filesystem::path shortPath(std::wstring(buffer), qi::unicodeFacet());
        delete [] buffer;

        return shortPath.string(qi::unicodeFacet());
      }
#endif
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
                         const std::string &filename)
    {
      if(filename == "") {
        qiLogError() << "Filename cannot be empty!";
        return std::string();
      }
      return getInstance()->findConf(applicationName, filename);
    }

    std::string findData(const std::string &applicationName,
                         const std::string &filename)
    {
      if(filename == "") {
        qiLogError() << "Filename cannot be empty!";
        return std::string();
      }
      return getInstance()->findData(applicationName, filename);
    }

    std::vector<std::string> listLib(const std::string &subfolder,
                                     const std::string &pattern)
    {
      return getInstance()->listLib(subfolder, pattern);
    }

    std::vector<std::string> listData(const std::string &applicationName,
                                      const std::string &pattern)
    {
      return getInstance()->listData(applicationName, pattern);
    }

    std::vector<std::string> confPaths(const std::string &applicationName)
    {
      return getInstance()->confPaths(applicationName);
    }

    std::vector<std::string> dataPaths(const std::string &applicationName)
    {
      return getInstance()->dataPaths(applicationName);
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
      // Windows doesn't natively support unicode path. Returns an ASCII one
      return detail::dosCompatiblePath(pathString);
#else
      // just ignore
      return pathString;
#endif
    }
  }

  SDKLayout* getInstance()
  {
    if (gInstance == NULL) {
      gInstance = new SDKLayout();
    }

    return gInstance;
  }

  namespace {
    _QI_COMMAND_LINE_OPTIONS(
      "Chrooting",
      ("writable-path", value<std::string>()->notifier(&qi::path::detail::setWritablePath), "Set the writable path.")
      )
  }
}
