/*
 * sdklayout.cpp
 * Login : <hcuche@hcuche-de>
 * Started on  Fri Apr 15 10:44:48 2011 Herve Cuche
 * $Id$
 *
 * Author(s):
 *  - Herve Cuche <hcuche@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Herve Cuche
 */

#include <iostream>
#include <numeric>

#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/path/sdklayout.hpp>
#include <qi/application.hpp>

#include <boost/filesystem.hpp>
#include <qi/locale.hpp>
#include <locale>
#include "src/filesystem.hpp"

namespace qi {


  class PrivateSDKLayout
  {
  public:
    std::vector<std::string> _sdkPrefixes;
    std::string _mode;

    PrivateSDKLayout()
      : _sdkPrefixes(),
        _mode()
    {
    }

    void initSDKlayout()
    {
      const char *program = qi::program();

      if (!boost::filesystem::exists(program)) {
        _mode = "error";
        return;
      }

      // We may use argc, argv to elaborate command line parsing, but,
      // right now only argv[0] is used.
      boost::filesystem::path execPath(program, qi::unicodeFacet());
      execPath = boost::filesystem::system_complete(execPath).make_preferred();
      _sdkPrefixes.push_back(execPath.parent_path().parent_path().string(qi::unicodeFacet()));
      if (execPath.parent_path().filename().string(qi::unicodeFacet()) != "bin")
        _mode = execPath.parent_path().filename().string(qi::unicodeFacet());
      else
        _mode = "";
    }

    void checkInit()
    {
      if (_mode == "error" || _sdkPrefixes.size() == 0)
        throw PathException("qipath not initialized\n"
                            "Please call qipath::init first");
    }
  };

  SDKLayout::~SDKLayout()
  {
    delete _private;
  }

  SDKLayout::SDKLayout()
    : _private(new PrivateSDKLayout)
  {
    _private->initSDKlayout();
  }

  SDKLayout::SDKLayout(const SDKLayout &rhs)
    : _private(new PrivateSDKLayout)
  {
    *_private = *rhs._private;
  }

  SDKLayout & SDKLayout::operator=(const SDKLayout &rhs) {
    *_private = *rhs._private;
    return *this;
  }

  // FIXME: Add exception if prefix == ""
  SDKLayout::SDKLayout(const std::string &prefix, const std::string &mode)
    : _private(new PrivateSDKLayout)
  {
    boost::filesystem::path prefixPath(prefix, qi::unicodeFacet());
    prefixPath = boost::filesystem::system_complete(prefixPath).make_preferred();
    _private->_sdkPrefixes.push_back(prefixPath.string(qi::unicodeFacet()));
    _private->_mode = mode;
  }

  void SDKLayout::addOptionalSdkPrefix(const char *prefix)
  {
    _private->checkInit();
    boost::filesystem::path prefixPath(prefix, qi::unicodeFacet());
    prefixPath = boost::filesystem::system_complete(prefixPath).make_preferred();

    _private->_sdkPrefixes.push_back(prefixPath.string(qi::unicodeFacet()));
  }

  void SDKLayout::clearOptionalSdkPrefix()
  {
    _private->checkInit();
    if (_private->_sdkPrefixes.size() > 0)
    {
      std::string sdkPrefixPath = _private->_sdkPrefixes[0];
      _private->_sdkPrefixes.clear();
      _private->_sdkPrefixes.push_back(sdkPrefixPath);
    }
  }

  std::string SDKLayout::sdkPrefix() const
  {
    _private->checkInit();
    return _private->_sdkPrefixes[0];
  }

  std::vector<std::string> SDKLayout::getSdkPrefixes() const
  {
    _private->checkInit();
    return _private->_sdkPrefixes;
  }

  std::string SDKLayout::findBin(const std::string &name) const
  {
    _private->checkInit();

    boost::filesystem::path bin(name, qi::unicodeFacet());
    if (boost::filesystem::exists(bin)
        && !boost::filesystem::is_directory(bin))
      return bin.string(qi::unicodeFacet());

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      boost::filesystem::path p(*it, qi::unicodeFacet());
     #ifdef _MSC_VER
      p = p / _private->_mode / name;
      p = p.make_preferred();

      if (boost::filesystem::exists(p))
        return p.string(qi::unicodeFacet());
//find _d, fallback on release stuff
#ifndef NDEBUG
      if (boost::filesystem::exists(boost::filesystem::path(p.string(qi::unicodeFacet()) + "_d.exe", qi::unicodeFacet())))
        return (p.string(qi::unicodeFacet()) + "_d.exe");
#endif
      if (boost::filesystem::exists(boost::filesystem::path(p.string(qi::unicodeFacet()) + ".exe", qi::unicodeFacet())))
        return (p.string(qi::unicodeFacet()) + ".exe");
     #endif

      p = *it;
      p = p / "bin" / name;
      p = p.make_preferred();


      if (boost::filesystem::exists(p)
          && !boost::filesystem::is_directory(p))
        return p.string(qi::unicodeFacet());
#ifndef NDEBUG
      if (boost::filesystem::exists(boost::filesystem::path(p.string(qi::unicodeFacet()) + "_d.exe", qi::unicodeFacet())))
        return (p.string(qi::unicodeFacet()) + "_d.exe");
#endif
      if (boost::filesystem::exists(boost::filesystem::path(p.string(qi::unicodeFacet()) + ".exe", qi::unicodeFacet())))
        return (p.string(qi::unicodeFacet()) + ".exe");
    }

    return std::string();
  }

  static std::string existsLib(boost::filesystem::path prefix,
                               const std::string& libName)
  {
      boost::filesystem::path lib(libName, qi::unicodeFacet());

      if (boost::filesystem::exists((prefix / lib).make_preferred())
          && !boost::filesystem::is_directory((prefix / lib).make_preferred()))
        return ((prefix / lib).make_preferred().string(qi::unicodeFacet()));

      return std::string();
  }

  std::string SDKLayout::findLib(const std::string &name) const
  {
    _private->checkInit();

    boost::filesystem::path module = boost::filesystem::path(name, qi::unicodeFacet());
    boost::filesystem::path prefix = module.parent_path().make_preferred();
    std::string libName = module.filename().make_preferred().string(qi::unicodeFacet());
    std::string res;

    res = existsLib(prefix.string(qi::unicodeFacet()), libName);
    if (res != std::string())
      return res;


    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      boost::filesystem::path p(*it, qi::unicodeFacet());
     #ifdef _MSC_VER
      p = p / _private->_mode / prefix;
      p = p.make_preferred();

      res = existsLib(p, libName);
      if (res != std::string())
        return res;

//DEBUG
#ifndef NDEBUG
      res = existsLib(p, libName + "_d.dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName + "_d.dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName);
      if (res != std::string())
        return res;
#endif

      res = existsLib(p, libName + ".dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName + ".dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName);
      if (res != std::string())
        return res;
      #endif

      p = *it;
      p = p / "lib" / prefix;
      p = p.make_preferred();

      res = existsLib(p, libName);
      if (res != std::string())
        return res;
      res = existsLib(p, libName + ".so");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName + ".so");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName);
      if (res != std::string())
        return res;
     #ifdef __APPLE__
      res = existsLib(p, libName + ".dylib");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName + ".dylib");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName);
      if (res != std::string())
        return res;
     #endif
     #ifdef _WIN32
//DEBUG
#ifndef NDEBUG
      res = existsLib(p, libName + "_d.dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName + "_d.dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName);
      if (res != std::string())
        return res;
#endif

      res = existsLib(p, libName + ".dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName + ".dll");
      if (res != std::string())
        return res;
      res = existsLib(p, "lib" + libName);
      if (res != std::string())
        return res;
     #endif
    }

    return std::string();
  }

  std::string SDKLayout::findConf(const std::string &applicationName,
                                           const std::string &filename) const
  {
    _private->checkInit();
    std::vector<std::string> paths = confPaths(applicationName);

    std::vector<std::string>::const_iterator it;
    for (it = paths.begin(); it != paths.end(); ++it)
    {
      boost::filesystem::path p(*it, qi::unicodeFacet());
      p /= filename;
      p = p.make_preferred();
      if (boost::filesystem::exists(p))
        return p.string(qi::unicodeFacet());
    }

    return std::string();
  }

  std::string SDKLayout::findData(const std::string &applicationName,
                                  const std::string &filename) const
  {
    _private->checkInit();
    std::vector<std::string> paths = dataPaths(applicationName);

    std::vector<std::string>::const_iterator it;
    for (it = paths.begin(); it != paths.end(); ++it)
    {
      boost::filesystem::path p(*it, qi::unicodeFacet());

      p /= filename;
      p = p.make_preferred();
      if (boost::filesystem::exists(p))
        return p.string(qi::unicodeFacet());
    }
    return std::string();
  }

  std::vector<std::string> SDKLayout::confPaths(const std::string &applicationName) const
  {
    _private->checkInit();
    std::vector<std::string> res;

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin(); it != _private->_sdkPrefixes.end(); ++it)
    {
      boost::filesystem::path prefix(*it, qi::unicodeFacet());
      res.push_back((prefix / "preferences" / applicationName).make_preferred().string(qi::unicodeFacet()));
      res.push_back((prefix / "preferences").make_preferred().string(qi::unicodeFacet()));
      res.push_back((prefix / "etc" / applicationName).make_preferred().string(qi::unicodeFacet()));
      res.push_back((prefix / "etc").make_preferred().string(qi::unicodeFacet()));

     #ifdef _MSC_VER
      boost::filesystem::path multiConfigPrefPath(*it, qi::unicodeFacet());
      multiConfigPrefPath = multiConfigPrefPath / _private->_mode;
      multiConfigPrefPath = multiConfigPrefPath.make_preferred();
      res.push_back((multiConfigPrefPath / "preferences" / applicationName).make_preferred().string(qi::unicodeFacet()));
      res.push_back((multiConfigPrefPath / "preferences").make_preferred().string(qi::unicodeFacet()));
      res.push_back((multiConfigPrefPath / "etc" / applicationName).make_preferred().string(qi::unicodeFacet()));
      res.push_back((multiConfigPrefPath / "etc").make_preferred().string(qi::unicodeFacet()));
     #endif
    }

    // Pass an empty string to get the directory:
    res.push_back(userWritableConfPath(applicationName, ""));

   #ifndef _WIN32
    boost::filesystem::path systemPath("/etc", qi::unicodeFacet());
    res.push_back((systemPath / applicationName).make_preferred().string(qi::unicodeFacet()));
   #endif
    return res;
  }

  // FIXME: Auto-test needed
  std::vector<std::string> SDKLayout::dataPaths(const std::string &applicationName) const
  {
    _private->checkInit();
    std::vector<std::string> res;

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      boost::filesystem::path prefix(*it, qi::unicodeFacet());
      res.push_back((prefix / "share" / applicationName).make_preferred().string(qi::unicodeFacet()));
    }

    // Pass an empty string to get the directory:
    res.push_back(userWritableDataPath(applicationName, ""));

    return res;
  }


  std::vector<std::string> SDKLayout::binPaths() const
  {
    _private->checkInit();
    std::vector<std::string> binPaths;

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      boost::filesystem::path p(*it, qi::unicodeFacet());
      if (_private->_mode != "")
        binPaths.push_back((p / _private->_mode).make_preferred().string(qi::unicodeFacet()));

      binPaths.push_back((p / "bin").make_preferred().string(qi::unicodeFacet()));
    }

    return binPaths;
  }

  std::vector<std::string> SDKLayout::libPaths() const
  {
    _private->checkInit();
    std::vector<std::string> libPaths;

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      boost::filesystem::path p(*it, qi::unicodeFacet());

      if (_private->_mode != "")
        libPaths.push_back((p / _private->_mode).make_preferred().string(qi::unicodeFacet()));

      libPaths.push_back((p / "lib").make_preferred().string(qi::unicodeFacet()));
    }

    return libPaths;
  }


  std::string SDKLayout::userWritableDataPath(const std::string &applicationName,
                                              const std::string &filename) const
  {
    _private->checkInit();
    boost::filesystem::path path(::qi::os::home(), qi::unicodeFacet());

   #ifndef _WIN32
    path = path / ".local" / "share" / applicationName / filename;
    path = path.make_preferred();
   #else
    boost::filesystem::path envUserAppData(qi::os::getenv("AppData"),
                                           qi::unicodeFacet());
    path = envUserAppData / applicationName / filename;
    path = path.make_preferred();
   #endif

    boost::filesystem::path dest = path;
    if (!filename.empty())
      dest = path.parent_path();

    if (!boost::filesystem::exists(dest)) {
      try {
        boost::filesystem::create_directories(dest);
      }
      catch (boost::filesystem::filesystem_error) {
      }
    }
    return path.string(qi::unicodeFacet());
  }


  std::string SDKLayout::userWritableConfPath(const std::string &applicationName,
                                                          const std::string &filename) const
  {
    _private->checkInit();
    boost::filesystem::path path(::qi::os::home(), qi::unicodeFacet());

    path = path / ".config" / applicationName / filename;
    path = path.make_preferred();

    boost::filesystem::path dest = path;
    if (!filename.empty())
      dest = path.parent_path();

    if (!boost::filesystem::exists(dest)) {
      try {
        boost::filesystem::create_directories(dest);
      }
      catch (boost::filesystem::filesystem_error) {
      }
    }
    return path.string(qi::unicodeFacet());
  }
}; // qi
