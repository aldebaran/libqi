/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <iostream>
#include <fstream>
#include <vector>

#include <qi/os.hpp>
#include <qi/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <qi/application.hpp>

int main(int argc, char *argv[])
{
  // Get the prefix name from argv0
  // Performs various initializations.
  qi::Application app(argc, argv);

  // Get sdk prefix
  std::cout << "SDK prefix is: \"" << qi::path::sdkPrefix() << "\"" << std::endl;

  // First argument is the name of the application, used
  // to build various paths later.
  qi::Path fooCfgPath = qi::path::findConf("foo", "foo.cfg");
  if (fooCfgPath.isEmpty())
  {
    std::cerr << "Could not find foo.cfg" << std::endl;
    std::cerr << "Looked in: " << std::endl;
    std::vector<std::string > configPaths = qi::path::confPaths("foo");
    std::vector<std::string>::const_iterator it;
    for (it = configPaths.begin(); it != configPaths.end(); ++it)
    {
      std::cerr << "\t\"" << *it << "\"" << std::endl;
    }
  }
  else
  {
    std::cout << "Found foo.cfg: \"" << fooCfgPath << "\"" << std::endl;
    std::cout << "Contents: " << std::endl;
    char buf[250];
    boost::filesystem::ifstream ifs(fooCfgPath);
    while (ifs.getline(buf, sizeof(buf)))
    {
      std::cout << buf << std::endl;
    }
  }

  // ... Write back the configuration to userCfgPath
  qi::Path userCfgPath = qi::path::userWritableConfPath("foo", "foo.cfg");
  std::cout << "Writing config file to: \"" << userCfgPath << "\"" << std::endl;
  boost::filesystem::ofstream ofs(userCfgPath, std::fstream::out | std::fstream::trunc);
  ofs << "Hi, this is foo.cfg" << std::endl;
  return 0;
}

