/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>
#include <numeric>

#include <qi/application.hpp>
#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/qi.hpp>
#include <qi/log.hpp>

#include <boost/filesystem.hpp>
#include <locale>
#include "sdklayout.hpp"
#include "filesystem.hpp"
#include "utils.hpp"

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
      const char *program = qi::Application::program();

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
      if (_mode == "error" || _sdkPrefixes.empty()) {
        qiLogDebug("qi::path") << "please call qi::init first before using qi::path";
      }
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
    _private->checkInit();
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
    boost::filesystem::path prefixPath(prefix, qi::unicodeFacet());
    try
    {
      prefixPath = boost::filesystem::system_complete(prefixPath).make_preferred();
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
    }
    _private->_sdkPrefixes.push_back(prefixPath.string(qi::unicodeFacet()));
  }

  void SDKLayout::clearOptionalSdkPrefix()
  {
    if (_private->_sdkPrefixes.size() > 0)
    {
      std::string sdkPrefixPath = _private->_sdkPrefixes[0];
      _private->_sdkPrefixes.clear();
      _private->_sdkPrefixes.push_back(sdkPrefixPath);
    }
  }

  std::string SDKLayout::sdkPrefix() const
  {
    if (_private->_sdkPrefixes.size() <= 0)
      return std::string();
    return _private->_sdkPrefixes[0];
  }

  std::vector<std::string> SDKLayout::getSdkPrefixes() const
  {
    return _private->_sdkPrefixes;
  }

  std::string SDKLayout::findBin(const std::string &name) const
  {
    boost::filesystem::path bin(name, qi::unicodeFacet());
    try
    {
      // try if the name is a full path
      bin = boost::filesystem::system_complete(bin);
      if (boost::filesystem::exists(bin)
          && !boost::filesystem::is_directory(bin))
        return bin.string(qi::unicodeFacet());

      std::vector<std::string>::const_iterator it;
      for (it = _private->_sdkPrefixes.begin();
           it != _private->_sdkPrefixes.end();
           ++it)
      {
        boost::filesystem::path p;
        p = boost::filesystem::path(fsconcat(*it, "bin", name), qi::unicodeFacet());

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
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
    }
    return std::string();
  }

  static std::string existsLib(boost::filesystem::path prefix,
                               const std::string& libName)
  {
    boost::filesystem::path lib(libName, qi::unicodeFacet());

    try
    {
      boost::filesystem::path p(fsconcat(prefix.string(qi::unicodeFacet()),
                                         lib.string(qi::unicodeFacet())),
                                qi::unicodeFacet());

      p = boost::filesystem::system_complete(p);
      if (boost::filesystem::exists(p)
          && !boost::filesystem::is_directory(p))
        return (p.string(qi::unicodeFacet()));
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
    }
    return std::string();
  }

  std::string SDKLayout::findLib(const std::string &name) const
  {
    try
    {
      // check if it's a full path
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
        boost::filesystem::path p;
        p = boost::filesystem::path(fsconcat(*it, "lib", prefix.string(qi::unicodeFacet())), qi::unicodeFacet());

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
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
    }

    return std::string();
  }

  std::string SDKLayout::findConf(const std::string &applicationName,
                                  const std::string &filename) const
  {
    std::vector<std::string> paths = confPaths(applicationName);
    try
    {
      std::vector<std::string>::const_iterator it;
      for (it = paths.begin(); it != paths.end(); ++it)
      {
        boost::filesystem::path p(fsconcat(*it, filename), qi::unicodeFacet());

        if (boost::filesystem::exists(p))
          return p.string(qi::unicodeFacet());
      }
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
    }
    return std::string();
  }

  std::string SDKLayout::findData(const std::string &applicationName,
                                  const std::string &filename) const
  {
    std::vector<std::string> paths = dataPaths(applicationName);
    try
    {
      std::vector<std::string>::const_iterator it;
      for (it = paths.begin(); it != paths.end(); ++it)
      {
        boost::filesystem::path p(fsconcat(*it, filename), qi::unicodeFacet());

        if (boost::filesystem::exists(p))
          return p.string(qi::unicodeFacet());
      }
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
    }
    return std::string();
  }

  std::vector<std::string> SDKLayout::confPaths(const std::string &applicationName) const
  {
    std::vector<std::string> res;

    // Pass an empty string to get the directory:
    res.push_back(userWritableConfPath(applicationName, ""));

    try
    {
      std::vector<std::string>::const_iterator it;
      for (it = _private->_sdkPrefixes.begin(); it != _private->_sdkPrefixes.end(); ++it)
      {
        res.push_back(fsconcat(*it, "etc", applicationName));
        res.push_back(fsconcat(*it, "etc"));
        res.push_back(fsconcat(*it, "preferences", applicationName));
        res.push_back(fsconcat(*it, "preferences"));
      }

#ifndef _WIN32
      res.push_back(fsconcat("/etc", applicationName));
#endif
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug("qi::path") << e.what();
      return std::vector<std::string>();
    }

    return res;
  }

  // FIXME: Auto-test needed
  std::vector<std::string> SDKLayout::dataPaths(const std::string &applicationName) const
  {
    std::vector<std::string> res;
    // Pass an empty string to get the directory:
    res.push_back(userWritableDataPath(applicationName, ""));

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      res.push_back(fsconcat(*it, "share", applicationName));
    }
    return res;
  }


  std::vector<std::string> SDKLayout::binPaths() const
  {
    std::vector<std::string> binPaths;

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      binPaths.push_back(fsconcat(*it, "bin"));
    }

    return binPaths;
  }

  std::vector<std::string> SDKLayout::libPaths() const
  {
    std::vector<std::string> libPaths;

    std::vector<std::string>::const_iterator it;
    for (it = _private->_sdkPrefixes.begin();
         it != _private->_sdkPrefixes.end();
         ++it)
    {
      libPaths.push_back(fsconcat(*it, "lib"));
    }

    return libPaths;
  }


  std::string SDKLayout::userWritableDataPath(const std::string &applicationName,
                                              const std::string &filename) const
  {
    boost::filesystem::path path;
#ifndef _WIN32
    path = boost::filesystem::path(fsconcat(::qi::os::home(), ".local", "share", applicationName, filename),
                                   qi::unicodeFacet());
#else
    path = boost::filesystem::path(fsconcat(qi::os::getenv("AppData"), applicationName, filename),
                                   qi::unicodeFacet());
#endif

    boost::filesystem::path dest = path;
    if (!filename.empty())
      dest = path.parent_path();

    if (!boost::filesystem::exists(dest)) {
      try {
        boost::filesystem::create_directories(dest);
      }
      catch (const boost::filesystem::filesystem_error &e)
      {
        qiLogDebug("qi::path") << e.what();
        return std::string();
      }
    }
    return path.string(qi::unicodeFacet());
  }


  std::string SDKLayout::userWritableConfPath(const std::string &applicationName,
                                              const std::string &filename) const
  {
    boost::filesystem::path path;

#ifndef _WIN32
    path = boost::filesystem::path(fsconcat(::qi::os::home(), ".config", applicationName, filename),
                                   qi::unicodeFacet());
#else
    path = boost::filesystem::path(fsconcat(qi::os::getenv("AppData"), applicationName, filename),
                                   qi::unicodeFacet());
#endif

    boost::filesystem::path dest = path;
    if (!filename.empty())
      dest = path.parent_path();

    if (!boost::filesystem::exists(dest)) {
      try {
        boost::filesystem::create_directories(dest);
      }
      catch (const boost::filesystem::filesystem_error &e)
      {
        qiLogDebug("qi::path") << e.what();
        return std::string();
      }
    }
    return path.string(qi::unicodeFacet());
  }
}
