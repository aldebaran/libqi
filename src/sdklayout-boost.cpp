/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>
#include <sstream>
#include <numeric>

#include <qi/application.hpp>
#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/qi.hpp>
#include <qi/log.hpp>

#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <locale>
#include <set>
#include "sdklayout.hpp"
#include "filesystem.hpp"
#include "utils.hpp"
#include <boost/system/error_code.hpp>

qiLogCategory("qi.path");

namespace {

// returns a regex which corresponds to the provided glob pattern,
static std::string globToRegex(std::string glob)
{
  boost::trim(glob);
  const char expression[] = "(\\*)|(\\?)|(\\\\)|(\\.)";
  const char format[] = "(?1[\\\\w\\\\.-]*)(?2[\\\\w]{1})(?3\\\\\\\\)(?4\\\\.)";
  std::stringstream final;
  std::ostream_iterator<char, char> oi(final);
  boost::regex re;
  re.assign(expression);
  boost::regex_replace(oi, glob.begin(), glob.end(),
                       re, format, boost::match_default | boost::format_all);
  return final.str();
}

// return the path "relative' such that full == base / relative
// This function assumes that full is a child path of base.
static std::string relative(const boost::filesystem::path &base,
                            const boost::filesystem::path &full)
{
  boost::filesystem::path relative, parent(full), cleanBase(base);
  if(cleanBase.filename() == ".")
  {
    // base ends with "/", let remove it.
    cleanBase.remove_filename();
  }
  while (parent != cleanBase)
  {
    relative = parent.filename() / relative;
    parent.remove_filename();
  }
  return relative.string(qi::unicodeFacet());
}

}

namespace qi {

  class PrivateSDKLayout
  {
  public:
    std::vector<std::string> _sdkPrefixes;
    std::string _mode;
    std::string _writablePath;

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
        qiLogDebug() << "please call qi::init first before using qi::path";
      }
    }

    std::string writablePath(const std::string &applicationName,
                             const std::string &filename,
                             const std::string &type)
    {
      std::string prefix;
      if (!_writablePath.empty())
      {
        prefix = fsconcat(_writablePath, type);
      }
      else
      {
        prefix = qi::os::getenv("QI_WRITABLE_PATH");
        if (!prefix.empty())
        {
          prefix = fsconcat(prefix, type);
        }
        else
        {
  #ifndef _WIN32
          std::string prefix2; // for backward compatibility
          if (type == "data")
          {
            prefix2 = fsconcat(".local", "share");
          }
          else if (type == "config")
          {
            prefix2 = ".config";
          }

          prefix = fsconcat(qi::os::home(), prefix2);
  #else
          prefix = qi::os::getenv("AppData");
  #endif
        }
      }

      boost::filesystem::path path;
      path = boost::filesystem::path(fsconcat(prefix, applicationName, filename),
                                     qi::unicodeFacet());

      boost::filesystem::path dest = path;
      if (!filename.empty())
        dest = path.parent_path();

      if (!boost::filesystem::exists(dest)) {
        try {
          boost::filesystem::create_directories(dest);
        }
        catch (const boost::filesystem::filesystem_error &e)
        {
          qiLogDebug() << e.what();
          return std::string();
        }
      }
      return path.string(qi::unicodeFacet());
    }
  };

  SDKLayout::~SDKLayout()
  {
    delete _p;
  }

  SDKLayout::SDKLayout()
    : _p(new PrivateSDKLayout)
  {
    _p->initSDKlayout();
    _p->checkInit();
  }

  SDKLayout::SDKLayout(const SDKLayout &rhs)
    : _p(new PrivateSDKLayout)
  {
    *_p = *rhs._p;
  }

  SDKLayout & SDKLayout::operator=(const SDKLayout &rhs) {
    *_p = *rhs._p;
    return *this;
  }

  // FIXME: Add exception if prefix == ""
  SDKLayout::SDKLayout(const std::string &prefix, const std::string &mode)
    : _p(new PrivateSDKLayout)
  {
    boost::filesystem::path prefixPath(prefix, qi::unicodeFacet());
    prefixPath = boost::filesystem::system_complete(prefixPath).make_preferred();
    _p->_sdkPrefixes.push_back(prefixPath.string(qi::unicodeFacet()));
    _p->_mode = mode;
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
      qiLogDebug() << e.what();
    }
    _p->_sdkPrefixes.push_back(prefixPath.string(qi::unicodeFacet()));
  }

  void SDKLayout::clearOptionalSdkPrefix()
  {
    if (_p->_sdkPrefixes.size() > 0)
    {
      std::string sdkPrefixPath = _p->_sdkPrefixes[0];
      _p->_sdkPrefixes.clear();
      _p->_sdkPrefixes.push_back(sdkPrefixPath);
    }
  }

  std::string SDKLayout::sdkPrefix() const
  {
    if (_p->_sdkPrefixes.size() <= 0)
      return std::string();
    return _p->_sdkPrefixes[0];
  }

  std::vector<std::string> SDKLayout::getSdkPrefixes() const
  {
    return _p->_sdkPrefixes;
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
      for (it = _p->_sdkPrefixes.begin();
           it != _p->_sdkPrefixes.end();
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
      qiLogDebug() << e.what();
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
      qiLogDebug() << e.what();
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
      for (it = _p->_sdkPrefixes.begin();
           it != _p->_sdkPrefixes.end();
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

        // If it's not in lib/, it's in bin/
        p = boost::filesystem::path(fsconcat(*it, "bin", prefix.string(qi::unicodeFacet())), qi::unicodeFacet());

        res = existsLib(p, libName + ".dll");
        if (res != std::string())
          return res;
        res = existsLib(p, "lib" + libName + ".dll");
        if (res != std::string())
          return res;
        res = existsLib(p, "lib" + libName);
        if (res != std::string())
          return res;

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
#endif
      }
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      qiLogDebug() << e.what();
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
      qiLogDebug() << e.what();
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
      qiLogDebug() << e.what();
    }
    return std::string();
  }

  std::vector<std::string> SDKLayout::listData(const std::string &applicationName,
                                               const std::string &pattern) const
  {

    std::set<std::string> matchedPaths;
    std::vector<std::string> fullPaths;
    std::vector<std::string> paths = dataPaths(applicationName);
    for (std::vector<std::string>::const_iterator it = paths.begin();
         it != paths.end();
         ++it)
    {
      boost::filesystem::path dataPath(*it, qi::unicodeFacet());
      // Note that the call to fsconcat is crucial: it will convert the
      // pattern to a boost::filesystem::path and call make_preferred() which
      // ensures the pattern is formatted in the same way than the input.
      // Otherwise on Windows we might fail when trying to match
      // foo\data\model.txt with foo\data/*.txt (instead of foo\data\*.txt)
      boost::regex pathRegex(globToRegex(fsconcat(*it, pattern)));
      try
      {
        boost::system::error_code ec;
        boost::filesystem::recursive_directory_iterator itD(dataPath,
            boost::filesystem::symlink_option::none, ec);
        if (ec)
        {
          if (ec == boost::system::errc::no_such_file_or_directory)
          {
            // The directory made up by dataPaths() does not exist.
            // This is expected. Continue with the next one.
            continue;
          }
          else
          {
            // This error is truly not expected, throw it
            throw boost::filesystem::filesystem_error(
                "boost::filesystem::directory_iterator::construct", ec);
          }
        }
        for (;
             itD != boost::filesystem::recursive_directory_iterator(); // end
             ++itD)
        {
          if (!boost::filesystem::is_directory(itD->path()))
          {
            const std::string fullPath = itD->path().string(qi::unicodeFacet());
            if (boost::regex_match(fullPath, pathRegex))
            {
              std::string relativePath = relative(dataPath, itD->path());
              if (matchedPaths.find(relativePath) == matchedPaths.end())
              {
                // we only add the match if it was not found in a previous
                // dataPath.
                matchedPaths.insert(relativePath);
                fullPaths.push_back(fullPath);
              }
            }
          }
        }
      }
      catch (const boost::filesystem::filesystem_error &e)
      {
        // log and continue
        qiLogDebug() << e.what();
      }
    }
    return fullPaths;
  }

  std::vector<std::string> SDKLayout::confPaths(const std::string &applicationName) const
  {
    std::vector<std::string> res;

    // Pass an empty string to get the directory:
    res.push_back(userWritableConfPath(applicationName, ""));

    try
    {
      std::vector<std::string>::const_iterator it;
      for (it = _p->_sdkPrefixes.begin(); it != _p->_sdkPrefixes.end(); ++it)
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
      qiLogDebug() << e.what();
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
    for (it = _p->_sdkPrefixes.begin();
         it != _p->_sdkPrefixes.end();
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
    for (it = _p->_sdkPrefixes.begin();
         it != _p->_sdkPrefixes.end();
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
    for (it = _p->_sdkPrefixes.begin();
         it != _p->_sdkPrefixes.end();
         ++it)
    {
      libPaths.push_back(fsconcat(*it, "lib"));
    }

    return libPaths;
  }

  void SDKLayout::setWritablePath(const std::string &path)
  {
    boost::filesystem::path p(path, qi::unicodeFacet());
    _p->_writablePath = p.string(qi::unicodeFacet());
  }

  std::string SDKLayout::userWritableDataPath(const std::string &applicationName,
                                              const std::string &filename) const
  {
    return _p->writablePath(applicationName, filename, "data");
  }

  std::string SDKLayout::userWritableConfPath(const std::string &applicationName,
                                              const std::string &filename) const
  {
    return _p->writablePath(applicationName, filename, "config");
  }
}
