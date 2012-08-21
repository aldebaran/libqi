/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <fstream>

#include <qi/application.hpp>
#include <qi/qi.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/path.hpp>
#include <numeric>
#include <boost/filesystem.hpp>
#include "src/filesystem.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

namespace qi {
  static int         globalArgc = -1;
  static char**      globalArgv = 0;
  static bool        globalInitialized = false;

  static std::vector<std::string>* globalArguments;
  static std::string globalPrefix;
  static std::string globalProgram;
  typedef std::vector<boost::function<void()> > FunctionList;
  static FunctionList* globalAtExit = 0;
  static FunctionList* globalAtEnter = 0;

  template<typename T> static T& lazyGet(T* & ptr)
  {
    if (!ptr)
      ptr = new T;
    return *ptr;
  }

  static std::string guess_app_from_path(int argc, const char *argv[])
  {
    boost::filesystem::path execPath(argv[0], qi::unicodeFacet());
    execPath = boost::filesystem::system_complete(execPath).make_preferred();
    execPath = boost::filesystem::path(detail::normalizePath(execPath.string(qi::unicodeFacet())), qi::unicodeFacet());

    //arg0 does not exists, or is not a program (directory)
    if (!boost::filesystem::exists(execPath) || boost::filesystem::is_directory(execPath))
    {
      std::string filename = execPath.filename().string(qi::unicodeFacet());
      std::string envPath = qi::os::getenv("PATH");
      size_t begin = 0;
#ifndef _WIN32
      size_t end = envPath.find(":", begin);
#else
      size_t end = envPath.find(";", begin);
#endif
      while (end != std::string::npos)
      {
        std::string realPath = "";

        realPath = envPath.substr(begin, end - begin);
        boost::filesystem::path p(realPath, qi::unicodeFacet());
        p /= filename;
        p = boost::filesystem::system_complete(p).make_preferred();
        p = boost::filesystem::path(detail::normalizePath(p.string(qi::unicodeFacet())), qi::unicodeFacet());

        if (boost::filesystem::exists(p) && !boost::filesystem::is_directory(p))
          return p.string(qi::unicodeFacet());

        begin = end + 1;
#ifndef _WIN32
        end = envPath.find(":", begin);
#else
        end = envPath.find(";", begin);
#endif
      }
    }
    else
      return execPath.string(qi::unicodeFacet());
    return std::string();
  }

  Application::Application(int& argc, char ** &argv)
  {
    //Feed qi::path prefix from share/qi/path.conf if present
    //(automatically created when using qiBuild)
    std::string pathConf = ::qi::path::findData("qi", "path.conf");
    if (!pathConf.empty())
    {
      std::ifstream is(pathConf.c_str());
      while (is.good())
      {
        std::string path;
        std::getline(is, path);
        if (!path.empty() && path[0] != '#')
        {
          boost::filesystem::path bpath(path, qi::unicodeFacet());
          if (boost::filesystem::exists(bpath))
          {
            qiLogDebug("Application") << "Adding to path " << path;
            ::qi::path::detail::addOptionalSdkPrefix(path.c_str());
          }
        }
      }
    }
    if (globalInitialized)
      qiLogError("Application") << "Application was already initialized";
    globalInitialized = true;
    globalArgc = argc;
    globalArgv = argv;
    std::vector<std::string>& args = lazyGet(globalArguments);
    args.clear();
    for (int i=0; i<argc; ++i)
      args.push_back(argv[i]);

    FunctionList& fl = lazyGet(globalAtEnter);
    qiLogDebug("Application") << "Executing " << fl.size() << " atEnter handlers";
    for (FunctionList::iterator i = fl.begin(); i!= fl.end(); ++i)
      (*i)();
    fl.clear();
    argc = Application::argc();
    argv = globalArgv;
  }

  void* Application::loadModule(const std::string& moduleName, int flags)
  {
    void* handle = os::dlopen(moduleName.c_str(), flags);
    qiLogDebug("qi.Application") << "Loadmodule " << handle;
    if (!handle)
      qiLogVerbose("qi.Application") << "dlopen failed with " << os::dlerror();
    // Reprocess atEnter list in case the module had AT_ENTER
    FunctionList& fl = lazyGet(globalAtEnter);
    qiLogDebug("qi.Application") << "Executing " << fl.size() << " atEnter handlers";
    for (FunctionList::iterator i = fl.begin(); i!= fl.end(); ++i)
      (*i)();
    fl.clear();
    return handle;
  }

  void Application::unloadModule(void* handle)
  {
    os::dlclose(handle);
  }

  Application::~Application()
  {
    FunctionList& fl = lazyGet(globalAtExit);
    for (FunctionList::iterator i = fl.begin(); i!= fl.end(); ++i)
      (*i)();
  }

  void Application::run()
  {
    while (true)
      os::sleep(1000);
  }

  void Application::setArguments(const std::vector<std::string>& args)
  {
    globalArgc = args.size();
    lazyGet(globalArguments) = args;
    globalArgv = new char*[args.size() + 1];
    for (unsigned i=0; i<args.size(); ++i)
      globalArgv[i] = strdup(args[i].c_str());
    globalArgv[args.size()] = 0;
  }

  void Application::setArguments(int argc, char** argv)
  {
    globalArgc = argc;
    globalArgv = argv;
    std::vector<std::string>& args = lazyGet(globalArguments);
    args.resize(argc);
    for (int i=0; i<argc; ++i)
      args[i] = argv[i];
  }

  bool Application::initialized()
  {
    return globalInitialized;
  }

  int Application::argc()
  {
    return globalArgc;
  }

  const char** Application::argv()
  {
    return (const char**)globalArgv;
  }

  bool Application::atEnter(boost::function<void()> func)
  {
    qiLogDebug("qi.Application") << "atEnter";
    lazyGet(globalAtEnter).push_back(func);
    return true;
  }

  bool Application::atExit(boost::function<void()> func)
  {
    lazyGet(globalAtExit).push_back(func);
    return true;
  }

  const std::vector<std::string>& Application::arguments()
  {
    return lazyGet(globalArguments);
  }

/*
  http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
  Some OS-specific interfaces:
  Mac OS X: _NSGetExecutablePath() (man 3 dyld)
  Linux   : readlink /proc/self/exe
  Solaris : getexecname()
  FreeBSD : sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
  BSD with procfs: readlink /proc/curproc/file
  Windows : GetModuleFileName() with hModule = NULL

  The portable (but less reliable) method is to use argv[0].
  Although it could be set to anything by the calling program,
  by convention it is set to either a path name of the executable
  or a name that was found using $PATH.

  Some shells, including bash and ksh, set the environment variable "_"
  to the full path of the executable before it is executed. In that case
  you can use getenv("_") to get it. However this is unreliable because
  not all shells do this, and it could be set to anything or be left over
  from a parent process which did not change it before executing your program.
*/
  const char *Application::program()
  {
    try
    {
      if (!globalProgram.empty())
        return globalProgram.c_str();

#ifdef __APPLE__
      {
        char *fname = (char *)malloc(PATH_MAX);
        uint32_t sz = PATH_MAX;
        fname[0] = 0;
        int ret;
        ret = _NSGetExecutablePath(fname, &sz);
        if (ret == 0)
        {
          globalProgram = fname;
          globalProgram = detail::normalizePath(globalProgram);
        }
        else
        {
          globalProgram = guess_app_from_path(::qi::Application::argc(),
            ::qi::Application::argv());
        }
        free(fname);
      }
#elif __linux__
      boost::filesystem::path p("/proc/self/exe");
      boost::filesystem::path fname = boost::filesystem::read_symlink(p);

      if (!boost::filesystem::is_empty(fname))
        globalProgram = fname.string().c_str();
      else
        globalProgram = guess_app_from_path(::qi::Application::argc(),
          ::qi::Application::argv());
#elif _WIN32
      WCHAR fname[MAX_PATH];
      int ret = GetModuleFileNameW(NULL, fname, MAX_PATH);
      if (ret > 0)
      {
        fname[ret] = '\0';
        boost::filesystem::path programPath(fname, qi::unicodeFacet());
        globalProgram = programPath.string(qi::unicodeFacet());
      }
      else
      {
        // GetModuleFileName failed, trying to guess from argc, argv...
        globalProgram = guess_app_from_path(::qi::Application::argc(),
          ::qi::Application::argv());
      }
#else
      globalProgram = guess_app_from_path(::qi::Application::argc(),
        ::qi::Application::argv());
#endif
      return globalProgram.c_str();
    }
    catch (...)
    {
      return NULL;
    }
  }

  int argc()
  {
    return Application::argc();
  }
  const char** argv()
  {
    return Application::argv();
  }
  void init(int argc, char* argv[])
  {
    qiLogError("qi") << "qi::init() is deprecated, use qi::Application";
    new qi::Application(argc, argv);
  }
}
