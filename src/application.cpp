/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>

#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qi/atomic.hpp>
#include <qi/log.hpp>
#include <qi/path.hpp>
#include <src/sdklayout.hpp>
#include <numeric>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "utils.hpp"
#include "path_conf.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
static const char SEPARATOR = ':';
#else
static const char SEPARATOR = ';';
#endif

qiLogCategory("qi.Application");

namespace bfs = boost::filesystem;

static std::string _sdkPath;
static std::vector<std::string> _sdkPaths;

static void parseArguments(int argc, char* argv[])
{
  namespace po = boost::program_options;
  po::options_description desc("Application options");

  desc.add_options()
      ("qi-sdk-prefix", po::value<std::string>(&_sdkPath), "The path of the SDK to use")
      ;
  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);
  qi::Application::setArguments(po::collect_unrecognized(parsed.options, po::include_positional));
}

namespace qi {
  static int         globalArgc = -1;
  static char**      globalArgv = nullptr;
  static bool        globalInitialized = false;
  static bool        globalTerminated = false;
  static bool        globalIsStop = false;

  static std::string globalName;
  static std::vector<std::string>* globalArguments;
  static std::string globalPrefix;
  static std::string globalProgram;
  static std::string globalRealProgram;

  typedef std::vector<std::function<void()> > FunctionList;
  static FunctionList* globalAtExit = nullptr;
  static FunctionList* globalAtEnter = nullptr;
  static FunctionList* globalAtRun = nullptr;
  static FunctionList* globalAtStop = nullptr;


  static boost::mutex globalMutex;
  static boost::condition_variable globalCond;

  static boost::asio::io_service*             globalIoService = nullptr;
  static boost::thread*                       globalIoThread = nullptr;
  static boost::asio::io_service::work*       globalIoWork = nullptr;
  static std::list<boost::asio::signal_set*>* globalSignalSet = nullptr;


  static void readPathConf()
  {
    std::string prefix = ::qi::path::sdkPrefix();
    std::vector<std::string> toAdd =  ::qi::path::detail::parseQiPathConf(prefix);
    std::vector<std::string>::const_iterator it;
    for (it = toAdd.begin(); it != toAdd.end(); ++it) {
      ::qi::path::detail::addOptionalSdkPrefix(it->c_str());
    }
  }

  static void stop_io_service()
  {
    qiLogVerbose() << "Unregistering all signal handlers.";
    //dont call ioservice->stop, just remove all events for the ioservice
    //deleting the object holding the run() method from quitting
    delete globalIoWork;
    globalIoWork = 0;

    if (globalSignalSet) {
      std::list<boost::asio::signal_set*>::iterator it;
      for (it = globalSignalSet->begin(); it != globalSignalSet->end(); ++it) {
        (*it)->cancel();
        delete *it;
      }
      delete globalSignalSet;
      globalSignalSet = 0;
    }
    if (globalIoThread) {
      //wait for the ioservice to terminate
      if (boost::this_thread::get_id() != globalIoThread->get_id())
        globalIoThread->join();
      //we are sure run has stopped so we can delete the io service
      delete globalIoService;
      delete globalIoThread;
      globalIoThread = 0;
      globalIoService = 0;
    }
  }

  static void run_io_service()
  {
    qi::os::setCurrentThreadName("appioservice");
    globalIoService->run();
  }

  static void stop_handler(int signal_number)
  {
    qiLogVerbose() << "Signal " << signal_number << " received!";
    static int  count_int = 0;
    static int count_term = 0;
    int sigcount = 0;
    if (signal_number == SIGINT) {
      count_int++;
      sigcount = count_int;
    }
    else if (signal_number == SIGTERM) {
      count_term++;
      sigcount = count_term;
    }
    switch (sigcount) {
      case 1:
        qiLogInfo() << "Sending the stop command...";
        //register the signal again to call exit the next time if stop did not succeed
        Application::atSignal(boost::bind<void>(&stop_handler, _1), signal_number);
        // Stop might immediately trigger application destruction, so it has
        // to go after atSignal.
        Application::stop();
        return;
      default:
        //even for SIGTERM this is an error, so return 1.
        qiLogInfo() << "signal " << signal_number << " received a second time, calling exit(1).";
        exit(1);
        return;
    }
  }

  static void signal_handler(const boost::system::error_code& error, int signal_number, std::function<void (int)> fun)
  {
    //when cancel is called the signal handler is raised with an error. catch it!
    if (!error) {
      fun(signal_number);
    }
  }

  bool Application::atSignal(std::function<void (int)> func, int signal)
  {
    if (!globalIoService)
    {
      globalIoService = new boost::asio::io_service;
      // Prevent run from exiting
      globalIoWork = new boost::asio::io_service::work(*globalIoService);
      // Start io_service in a thread. It will call our handlers.
      globalIoThread = new boost::thread(&run_io_service);
      // We want signal handlers to work as late as possible.
      ::atexit(&stop_io_service);
      globalSignalSet = new std::list<boost::asio::signal_set*>;
    }

    boost::asio::signal_set *sset = new boost::asio::signal_set(*globalIoService, signal);
    sset->async_wait(boost::bind(signal_handler, _1, _2, func));
    globalSignalSet->push_back(sset);
    return true;
  }

  template<typename T> static T& lazyGet(T* & ptr)
  {
    if (!ptr)
      ptr = new T;
    return *ptr;
  }

  static boost::filesystem::path system_absolute(
      const boost::filesystem::path path)
  {
    if (path.empty())
      return path;

    if (path.is_absolute())
      return path;

    if (path.has_parent_path())
      return bfs::system_complete(path);

    if (!bfs::exists(path) || bfs::is_directory(path))
    {
      std::string envPath = qi::os::getenv("PATH");
      size_t begin = 0;
      for (size_t end = envPath.find(SEPARATOR, begin);
          end != std::string::npos;
          begin = end + 1, end = envPath.find(SEPARATOR, begin))
      {
        std::string realPath = envPath.substr(begin, end - begin);
        bfs::path p(realPath);

        p /= path;
        p = boost::filesystem::system_complete(p);

        if (boost::filesystem::exists(p) &&
            !boost::filesystem::is_directory(p))
          return p.string(qi::unicodeFacet());
      }
    }

    // fallback to something
    return bfs::system_complete(path);
  }

  static std::string guess_app_from_path(const char* path)
  {
    boost::filesystem::path execPath(path, qi::unicodeFacet());
    return system_absolute(execPath).make_preferred()
      .string(qi::unicodeFacet());
  }

  static void initApp(int& argc, char ** &argv, const std::string& path)
  {
    // this must be initialized first because readPathConf uses it (through
    // sdklayout)
    if (!path.empty())
    {
      globalProgram = path;
      qiLogVerbose() << "Program path explicitely set to " << globalProgram;
    }
    else
    {
      globalProgram = guess_app_from_path(argv[0]);
      qiLogVerbose() << "Program path guessed as " << globalProgram;
    }
    globalProgram = path::detail::normalize(globalProgram);

    parseArguments(argc, argv);

    if (_sdkPath.empty())
      _sdkPath = qi::os::getenv("QI_SDK_PREFIX");

    if (_sdkPaths.empty())
    {
      std::string prefixes = qi::os::getenv("QI_ADDITIONAL_SDK_PREFIXES");
      if (!prefixes.empty())
        boost::algorithm::split(_sdkPaths, prefixes, boost::algorithm::is_from_range(SEPARATOR, SEPARATOR));
    }

    readPathConf();
    if (globalInitialized)
      throw std::logic_error("Application was already initialized");
    globalInitialized = true;
    globalArgc = argc;
    globalArgv = argv;
    std::vector<std::string>& args = lazyGet(globalArguments);
    args.clear();
    for (int i=0; i<argc; ++i)
      args.push_back(argv[i]);

    FunctionList& fl = lazyGet(globalAtEnter);
    qiLogDebug() << "Executing " << fl.size() << " atEnter handlers";
    for (FunctionList::iterator i = fl.begin(); i!= fl.end(); ++i)
    {
      try
      {
        (*i)();
      }
      catch (std::exception& e)
      {
        qiLogError() << "Application atEnter callback throw the following error: " << e.what();
      }
    }

    fl.clear();
    argc = Application::argc();
    argv = globalArgv;
  }

  Application::Application(int& argc, char ** &argv, const std::string& name,
      const std::string& path)
  {
    globalName = name;
    initApp(argc, argv, path);
  }

  Application::Application(const std::string &name, int& argc, char ** &argv)
  {
    globalName = name;
    initApp(argc, argv, "");
  }

  void* Application::loadModule(const std::string& moduleName, int flags)
  {
    void* handle = os::dlopen(moduleName.c_str(), flags);
    if (!handle)
    {
      throw std::runtime_error("Module \'" + moduleName + "\' not load: error was " + qi::os::dlerror());
    }
    else
    {
      qiLogDebug() << "Loadmodule " << handle;
    }
    // Reprocess atEnter list in case the module had AT_ENTER
    FunctionList& fl = lazyGet(globalAtEnter);
    qiLogDebug() << "Executing " << fl.size() << " atEnter handlers";
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
    {
      try
      {
        (*i)();
      }
      catch (std::exception& e)
      {
        qiLogError() << "Application atExit callback throw the following error: " << e.what();
      }
    }

    globalCond.notify_all();
    globalTerminated = true;
  }

  static void initSigIntSigTermCatcher() {
    static bool signalInit = false;

    if (!signalInit) {
      qiLogVerbose() << "Registering SIGINT/SIGTERM handler within qi::Application";
      // kill with no signal sends TERM, control-c sends INT.
      Application::atSignal(boost::bind(&stop_handler, _1), SIGTERM);
      Application::atSignal(boost::bind(&stop_handler, _1), SIGINT);
      signalInit = true;
    }
  }

  static bool isStop()
  {
    return globalIsStop;
  }

  void Application::run()
  {
    //run is called, so we catch sigint/sigterm, the default implementation call Application::stop that
    //will make this loop exit.
    initSigIntSigTermCatcher();

    // call every function registered as "atRun"
    for(auto& function: lazyGet(globalAtRun))
      function();

    boost::unique_lock<boost::mutex> l(globalMutex);
    globalCond.wait(l, &isStop);
  }

  void Application::stop()
  {

    static qi::Atomic<bool> atStopHandlerCall = false;
    if (atStopHandlerCall.setIfEquals(false, true))
    {
      FunctionList& fl = lazyGet(globalAtStop);
      qiLogDebug() << "Executing " << fl.size() << " atStop handlers";
      for (FunctionList::iterator i = fl.begin(); i!= fl.end(); ++i)
      {
        try
        {
          (*i)();
        }
        catch (std::exception& e)
        {
          qiLogError() << "Application atStop callback throw the following error: " << e.what();
        }
      }
      boost::unique_lock<boost::mutex> l(globalMutex);
      globalIsStop = true;
      globalCond.notify_all();
    }
  }

  void Application::setName(const std::string &name)
  {
    globalName = name;
  }

  std::string Application::name()
  {
    return globalName;
  }

  void Application::setArguments(const std::vector<std::string>& args)
  {
    globalArgc = static_cast<int>(args.size());
    lazyGet(globalArguments) = args;
    globalArgv = new char*[args.size() + 1];
    for (unsigned i=0; i<args.size(); ++i)
      globalArgv[i] = qi::os::strdup(args[i].c_str());
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

  bool Application::terminated()
  {
    return globalTerminated;
  }

  int Application::argc()
  {
    return globalArgc;
  }

  const char** Application::argv()
  {
    return (const char**)globalArgv;
  }

  bool Application::atEnter(std::function<void()> func)
  {
    qiLogDebug() << "atEnter";
    lazyGet(globalAtEnter).push_back(func);
    return true;
  }

  bool Application::atExit(std::function<void()> func)
  {
    lazyGet(globalAtExit).push_back(func);
    return true;
  }

  bool Application::atRun(std::function<void ()> func)
  {
    lazyGet(globalAtRun).push_back(func);
    return true;
  }

  bool Application::atStop(std::function<void()> func)
  {
    //If the client call atStop, it mean it will handle the proper destruction
    //of the program by itself. So here we catch SigInt/SigTerm to call Application::stop
    //and let the developer properly stop the application as needed.
    initSigIntSigTermCatcher();
    lazyGet(globalAtStop).push_back(func);
    return true;
  }

  const std::vector<std::string>& Application::arguments()
  {
    return lazyGet(globalArguments);
  }

  const char *Application::program()
  {
    return globalProgram.c_str();
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
  const char *Application::realProgram()
  {
    try
    {
      if (!globalRealProgram.empty())
        return globalRealProgram.c_str();

#ifdef __APPLE__
      {
        char *fname = (char *)malloc(PATH_MAX);
        uint32_t sz = PATH_MAX;
        fname[0] = 0;
        int ret;
        ret = _NSGetExecutablePath(fname, &sz);
        if (ret == 0)
        {
          globalRealProgram = fname;
          globalRealProgram = path::detail::normalize(globalRealProgram);
        }
        else
        {
          globalRealProgram = guess_app_from_path(::qi::Application::argv()[0]);
        }
        free(fname);
      }
#elif __linux__
      boost::filesystem::path p("/proc/self/exe");
      boost::filesystem::path fname = boost::filesystem::read_symlink(p);

      if (!boost::filesystem::is_empty(fname))
        globalRealProgram = fname.string().c_str();
      else
        globalRealProgram = guess_app_from_path(::qi::Application::argv()[0]);
#elif _WIN32
      WCHAR fname[MAX_PATH];
      int ret = GetModuleFileNameW(NULL, fname, MAX_PATH);
      if (ret > 0)
      {
        fname[ret] = '\0';
        boost::filesystem::path programPath(fname, qi::unicodeFacet());
        globalRealProgram = programPath.string(qi::unicodeFacet());
      }
      else
      {
        // GetModuleFileName failed, trying to guess from argc, argv...
        globalRealProgram = guess_app_from_path(::qi::Application::argv()[0]);
      }
#else
      globalRealProgram = guess_app_from_path(::qi::Application::argv()[0]);
#endif
      return globalRealProgram.c_str();
    }
    catch (...)
    {
      return NULL;
    }
  }

  const char* Application::_suggestedSdkPath()
  {
    return _sdkPath.c_str();
  }

  const std::vector<std::string>& Application::_suggestedSdkPaths()
  {
    return _sdkPaths;
  }
}
