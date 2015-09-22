/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <set>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

#include <qi/path.hpp>

namespace qi {
  namespace path {
    namespace detail {

// recursive helper for parseQiPathConf();
static void recParseQiPathConf(const std::string &pathConf, std::vector<std::string> &res,
                              std::set<std::string>& filesSeen);

std::vector<std::string> parseQiPathConf(const std::string &pathConf)
{
  std::vector<std::string> res;
  std::set<std::string> filesSeen;
  recParseQiPathConf(pathConf, res, filesSeen);
  return res;
}


static void recParseQiPathConf(const std::string &prefix, std::vector<std::string>& res,
                              std::set<std::string>& filesSeen)
{
  const qi::Path pathConf = qi::Path(prefix) / "share/qi/path.conf";
  std::set<std::string>::iterator it;
  it = filesSeen.find(pathConf.str());
  if (it != filesSeen.end()) {
    return;
  }
  filesSeen.insert(pathConf.str());

  boost::filesystem::ifstream is(pathConf);
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
    std::vector<std::string>::iterator it = std::find(res.begin(), res.end(), newPrefix);
    if (it != res.end()) {
      continue;
    }
    res.push_back(path);
    recParseQiPathConf(newPrefix, res, filesSeen);
  }
}

    } // detail
  } // path
} // qi

