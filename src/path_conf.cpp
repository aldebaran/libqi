/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <set>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

#include <qi/log.hpp>
#include <qi/path.hpp>
#include <qi/path_conf.hpp>

qiLogCategory("qi.path");

namespace qi {
  namespace path {

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
    boost::system::error_code ec;
    bool exists = boost::filesystem::exists(bpath, ec);
    if (!exists) {
      continue;
    }
    if (ec) {
      qiLogError() << "Cannot access path '" << bpath << "': " << ec.message();
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

std::vector<std::string> parseQiPathConf(const std::string &prefix)
{
  std::vector<std::string> res;
  std::set<std::string> filesSeen;
  recParseQiPathConf(prefix, res, filesSeen);
  return res;
}

  } // path
} // qi

