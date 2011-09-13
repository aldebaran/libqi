#include <qi/path.hpp>
#include <qi/os.hpp>
#include "src/sdklayout.hpp"
#include <qi/qi.hpp>
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
      boost::filesystem::path path(pathString, qi::unicodeFacet());
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

    std::string sdkPrefix()
    {
      return getInstance()->sdkPrefix();
    }

    std::string findBin(const std::string &name)
    {
      return getInstance()->findBin(name);
    }

    std::string findLib(const std::string &name)
    {
      return getInstance()->findLib(name);
    }

    std::string findConf(const std::string &applicationName,
                         const std::string &filename)
    {
      if(filename == "")
        throw std::invalid_argument("Filename cannot be empty!");
      return getInstance()->findConf(applicationName, filename);
    }

    std::string findData(const std::string &applicationName,
                         const std::string &filename)
    {
      if(filename == "")
        throw std::invalid_argument("Filename cannot be empty!");
      return getInstance()->findData(applicationName, filename);
    }

    std::vector<std::string> confPaths(const std::string &applicationName)
    {
      return getInstance()->confPaths(applicationName);
    }

    std::vector<std::string> dataPaths(const std::string &applicationName)
    {
      return getInstance()->dataPaths(applicationName);
    }

    std::vector<std::string> binPaths()
    {
      return getInstance()->binPaths();
    }

    std::vector<std::string> libPaths()
    {
      return getInstance()->libPaths();
    }

    std::string userWritableDataPath(const std::string &applicationName,
                                     const std::string &filename)
    {
      return getInstance()->userWritableDataPath(applicationName, filename);
    }

    std::string userWritableConfPath(const std::string &applicationName,
                                     const std::string &filename)
    {
      return qi::getInstance()->userWritableConfPath(applicationName, filename);
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

      boost::filesystem::path execPath(program, qi::unicodeFacet());
      execPath = boost::filesystem::system_complete(execPath).make_preferred();
      prefix = execPath.parent_path().parent_path().string(qi::unicodeFacet());
      if (execPath.parent_path().filename().string(qi::unicodeFacet()) != "bin")
        mode = execPath.parent_path().filename().string(qi::unicodeFacet());
      else
        mode = "";

      gInstance = new SDKLayout(prefix, mode);
    }

    return gInstance;
  }

};

