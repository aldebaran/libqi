/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <set>
#include <fstream>
#include <boost/filesystem.hpp>

#include <qi/qi.hpp>

namespace qi {
  namespace path {
    namespace detail {

// recursive helper for parseQiPathConf();
static void recParseQiPathConf(const std::string &pathConf, std::set<std::string>& res,
                              std::set<std::string>& filesSeen);

std::set<std::string> parseQiPathConf(const std::string &pathConf)
{
  std::set<std::string> res;
  std::set<std::string> filesSeen;
  recParseQiPathConf(pathConf, res, filesSeen);
  return res;
}


static void recParseQiPathConf(const std::string &prefix, std::set<std::string>& res,
                              std::set<std::string>& filesSeen)
{
  boost::filesystem::path bpathConf(prefix, qi::unicodeFacet());
  bpathConf /= "share/qi/path.conf";
  std::string pathConf = bpathConf.string(qi::unicodeFacet());
  std::set<std::string>::iterator it;
  it = filesSeen.find(pathConf);
  if (it != filesSeen.end()) {
    return;
  }
  filesSeen.insert(pathConf);

  std::ifstream is(pathConf.c_str());
  while (is.good()) {
    std::string path;
    std::getline(is, path);
    if (path.empty() || path[0] == '#') {
      continue;
    }
    boost::filesystem::path bpath(path, qi::unicodeFacet());
    if (!boost::filesystem::exists(bpath)) {
      continue;
    }
    std::string newPrefix = bpath.string(qi::unicodeFacet());
    std::set<std::string>::iterator it = res.find(newPrefix);
    if (it != res.end()) {
      continue;
    }
    res.insert(path);
    recParseQiPathConf(newPrefix, res, filesSeen);
  }
  is.close();
}

    } // detail
  } // path
} // qi

