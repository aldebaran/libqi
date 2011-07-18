#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/path/sdklayout.hpp>
#include <qi/locale.hpp>
#include <qi/application.hpp>
#include <boost/filesystem.hpp>
#include "src/filesystem.hpp"

namespace qi
{
  SDKLayout* gInstance = NULL;
  SDKLayout* getInstance();

  namespace path
  {

    boost::filesystem::path absPath(const std::string &pathString)
    {
      boost::filesystem::path path(pathString, qi::utf8facet());
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
    }

    std::string getSdkPrefix()
    {
      return getInstance()->getSdkPrefix();
    }

    std::string findBinary(const std::string &name)
    {
      return getInstance()->findBinary(name);
    }

    std::string findLibrary(const std::string &name)
    {
      return getInstance()->findLibrary(name);
    }

    std::string findConfiguration(const std::string &applicationName,
                                  const std::string &filename)
    {
      return getInstance()->findConfiguration(applicationName, filename);
    }

    std::string findData(const std::string &applicationName,
                         const std::string &filename)
    {
      return getInstance()->findData(applicationName, filename);
    }

    std::vector<std::string> getConfigurationPaths(const std::string &applicationName)
    {
      return getInstance()->getConfigurationPaths(applicationName);
    }

    std::vector<std::string> getDataPaths(const std::string &applicationName)
    {
      return getInstance()->getDataPaths(applicationName);
    }

    std::vector<std::string> getBinaryPaths()
    {
      return getInstance()->getBinaryPaths();
    }

    std::vector<std::string> getLibraryPaths()
    {
      return getInstance()->getLibraryPaths();
    }

    std::string getUserWritableDataPath(const std::string &applicationName,
                                        const std::string &filename)
    {
      return getInstance()->getUserWritableDataPath(applicationName, filename);
    }

    std::string getUserWritableConfigurationPath(const std::string &applicationName,
                                                        const std::string &filename)
    {
      return qi::getInstance()->getUserWritableConfigurationPath(applicationName, filename);
    }
  };



  SDKLayout* getInstance()
  {
    if (gInstance == NULL) {
      std::string prefix;
      std::string mode;
      const char *program = qi::program();

      if (!boost::filesystem::exists(program)) {
        mode = "error";
      }

      boost::filesystem::path execPath(program, qi::utf8facet());
      execPath = boost::filesystem::system_complete(execPath).make_preferred();
      prefix = execPath.parent_path().parent_path().string(qi::utf8facet());
      if (execPath.parent_path().filename().string(qi::utf8facet()) != "bin")
        mode = execPath.parent_path().filename().string(qi::utf8facet());
      else
        mode = "";

      gInstance = new SDKLayout(prefix, mode);
    }

    return gInstance;
  }

};

