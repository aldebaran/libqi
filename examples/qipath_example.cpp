#include <iostream>
#include <fstream>
#include <vector>

#include <qi/os.hpp>
#include <qi/path.hpp>
#include <qi/application.hpp>

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

