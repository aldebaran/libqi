/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <iostream>
#include <fstream>
#include <vector>

#include <qi/os.hpp>
#include <qi/path.hpp>
#include <qi/qi.hpp>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <locale>

int main(int argc, char *argv[])
{
  // Set the global locale to loc
  std::locale::global(boost::locale::generator().generate(""));
  // Make boost.filesystem use it
  boost::filesystem::path::imbue(std::locale());

  // Get the prefix name from argv0
  std::cout << "argv0 is: " << argv[0] << std::endl;
  // Get the prefix name from argv0
  // Performs various initializations.
  // This is usually called by qi::init()
  qi::init(argc, argv);

  // Get sdk prefix
  std::cout << "SDK prefix is: " << qi::path::sdkPrefix() << std::endl;

  // First argument is the name of the application, used
  // to build various paths later.
  std::string fooCfgPath = qi::path::findConf("foo", "foo.cfg");
  if (fooCfgPath == "")
  {
    std::cerr << "Could not find foo.cfg" << std::endl;
    std::cerr << "Looked in: " << std::endl;
    std::vector<std::string > configPaths = qi::path::confPaths("foo");
    std::vector<std::string>::const_iterator it;
    for (it = configPaths.begin(); it != configPaths.end(); ++it)
    {
      std::cerr << "\t" << *it << std::endl;
    }
  }
  else
  {
    std::cout << "Found foo.cfg: " << fooCfgPath << std::endl;
    std::cout << "Contents: " << std::endl;
    char buf[250];
    std::ifstream ifs;

    // Set stream to the right charset
    ifs.imbue(std::locale());
    ifs.open(fooCfgPath.c_str(), std::fstream::in);
    while (! ifs.eof())
    {
      ifs.getline(buf, 250);
      std::cout << buf << std::endl;
    }
    ifs.close();
  }


  // ... Write back the configuration to userCfgPath
  std::string userCfgPath = qi::path::userWritableConfPath("foo", "foo.cfg");
  std::cout << "Writing config file to: " << userCfgPath << std::endl;
  std::ofstream ofs(userCfgPath.c_str(), std::fstream::out | std::fstream::trunc);
  ofs << "Hi, this is foo.cfg" << std::endl;
  ofs.close();

  return 0;
}

