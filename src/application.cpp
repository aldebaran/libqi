/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>
#include <iostream>
#include <numeric>

#include <src/application_p.hpp>
#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qi/atomic.hpp>
#include <qi/log.hpp>
#include <qi/path_conf.hpp>
#include <src/sdklayout.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "utils.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

qiLogCategory("qi.Application");

namespace bfs = boost::filesystem;

static std::string _sdkPath;
static boost::program_options::options_description _options;

static void parseArguments(int argc, char* argv[])
{
  namespace po = boost::program_options;
  po::options_description desc("Application options");

  desc.add_options()
      ("qi-sdk-prefix", po::value<std::string>(&_sdkPath), "The path of the SDK to use")
      ;
  qi::Application::options().add(desc);

  std::vector<std::string> argsLeft;

  // `boost::program_options::basic_command_line_parser` does not handle the case where argc == 0.
  if (argc > 0)
  {
    po::variables_map vm;
    auto parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);
    argsLeft = po::collect_unrecognized(parsed.options, po::include_positional);
  }
  qi::Application::setArguments(argsLeft);
}

namespace qi {
  static int         globalArgc = -1;
  static std::vector<char*> globalArgv;
  static bool        globalInitialized = false;
  static bool        globalTerminated = false;

  static std::string globalName;
  static std::vector<std::string>* globalArguments;
  static std::string globalPrefix;
  static std::string globalProgram;
  static std::string globalRealProgram;

  using FunctionList = std::vector<std::function<void()>>;
  using FunctionWithArgValueList = std::vector<std::pair<std::function<void(int)>, int>>;
  static FunctionList* globalAtExit = nullptr;
  static FunctionList* globalAtEnter = nullptr;
  static FunctionWithArgValueList* globalAtSignal = nullptr;
  static FunctionList* globalAtRun = nullptr;
  static FunctionList* globalAtStop = nullptr;

  static boost::optional<boost::asio::io_service> globalIoService;

  static void readPathConf()
  {
    std::string prefix = ::qi::path::sdkPrefix();
    std::vector<std::string> toAdd =  ::qi::path::parseQiPathConf(prefix);
    std::vector<std::string>::const_iterator it;
    for (it = toAdd.begin(); it != toAdd.end(); ++it) {
      ::qi::path::detail::addOptionalSdkPrefix(it->c_str());
    }
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
    namespace ph = std::placeholders;
    switch (sigcount) {
      case 1:
        qiLogVerbose() << "Sending the stop command...";
        //register the signal again to call exit the next time if stop did not succeed
        Application::atSignal(boost::bind<void>(&stop_handler, ph::_1), signal_number);
        // Stop might immediately trigger application destruction, so it has
        // to go after atSignal.
        Application::stop();
        return;
      default:
        //even for SIGTERM this is an error, so return 1.
        qiLogVerbose() << "signal " << signal_number << " received a second time, calling exit(1).";
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

  template<typename T> static T& lazyGet(T* & ptr)
  {
    if (!ptr)
      ptr = new T;
    return *ptr;
  }
  bool Application::atSignal(std::function<void (int)> func, int signal)
  {
    lazyGet(globalAtSignal).push_back(std::make_pair(std::move(func), signal));
    return true;
  }
  qi::Path details::searchExecutableAbsolutePath(const qi::Path& path,
                                                 const bfs::path& currentDirectory,
                                                 std::vector<bfs::path> environmentPaths)
  {
    const bfs::path boostPath(path);
    if (boostPath.is_relative() && !boostPath.has_parent_path())
    {
      environmentPaths.insert(environmentPaths.begin(), currentDirectory);
      return boost::process::search_path(boostPath, environmentPaths).make_preferred();
    }
    return bfs::absolute(boostPath, currentDirectory).make_preferred();
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
      globalProgram = details::searchExecutableAbsolutePath(qi::Path::fromNative(argv[0])).str();
      qiLogVerbose() << "Program path guessed as " << globalProgram;
    }

    globalProgram = path::detail::normalize(globalProgram).str();

    parseArguments(argc, argv);

    readPathConf();

    if (globalInitialized)
      throw std::logic_error("Application was already initialized");
    globalInitialized = true;

    // We use only one thread (the main thread) for the application `io_service` that is used
    // to process signals.
    constexpr const auto ioServiceConcurrencyHint = 1;
    globalIoService.emplace(ioServiceConcurrencyHint);

    globalArgc = argc;
    globalArgv = std::vector<char*>(argv, argv + argc);
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

    {
      // Add the help option
      namespace po = boost::program_options;
      po::options_description helpDesc("Help options");
      helpDesc.add_options() ("help,h", "Produces help message");
      _options.add(helpDesc);

      po::variables_map vm;
      try
      {
        po::parsed_options parsed = po::command_line_parser(Application::arguments())
          .options(_options)
          .allow_unregistered()
          .run();

        po::store(parsed, vm);
        po::notify(vm);

        std::vector<std::string> args
          = po::collect_unrecognized(parsed.options, po::include_positional);

        if (vm.count("help"))
        {
          std::cout << _options << std::endl;
          args.push_back("--help"); // Put the help argument back.
        }

        /* Set arguments to what was not used */
        ::qi::Application::setArguments(args);
        argc = Application::argc();
        argv = globalArgv.data();
      }
      catch (po::error& e)
      {
        qiLogError() << e.what();
      }


    }
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

    globalIoService = boost::none;
    globalTerminated = true;
  }

  void Application::run()
  {
    QI_ASSERT_TRUE(globalIoService);

    namespace ph = std::placeholders;

    // We use a list because signal_set is not moveable.
    std::list<boost::asio::signal_set> signalSets;

    auto atSignal = lazyGet(globalAtSignal);

    // run is called, so we catch sigint/sigterm, the default
    // implementation call Application::stop that
    // will make this loop exit.
    atSignal.emplace_back(boost::bind(stop_handler, ph::_1), SIGTERM);
    atSignal.emplace_back(boost::bind(stop_handler, ph::_1), SIGINT);

    for(const auto& func: atSignal)
      signalSets.emplace(signalSets.end(), *globalIoService, func.second)->async_wait(
                  boost::bind(signal_handler, ph::_1, ph::_2, std::move(func.first)));

    // Call every function registered as "atRun"
    for(auto& function: lazyGet(globalAtRun))
      function();

    globalIoService->run();
  }

  void Application::stop()
  {
    static qi::Atomic<bool> atStopHandlerCall{false};
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

      QI_ASSERT_TRUE(globalIoService);
      globalIoService->stop();
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
    globalArgv.resize(args.size() + 1);
    for (unsigned i=0; i<args.size(); ++i)
      globalArgv[i] = qi::os::strdup(args[i].c_str());
    globalArgv[args.size()] = 0;
  }

  void Application::setArguments(int argc, char** argv)
  {
    globalArgc = argc;
    globalArgv = std::vector<char*>(argv, argv + argc);
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
    return const_cast<const char**>(globalArgv.data());
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
          globalRealProgram = path::detail::normalize(globalRealProgram).str();
        }
        else
        {
          globalRealProgram = details::searchExecutableAbsolutePath(qi::Path::fromNative(::qi::Application::argv()[0])).str();
        }
        free(fname);
      }
#elif __linux__
      boost::filesystem::path p("/proc/self/exe");
      boost::filesystem::path fname = boost::filesystem::read_symlink(p);

      if (!boost::filesystem::is_empty(fname))
        globalRealProgram = fname.string().c_str();
      else
        globalRealProgram = details::searchExecutableAbsolutePath(qi::Path::fromNative(::qi::Application::argv()[0])).str();
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
        globalRealProgram = details::searchExecutableAbsolutePath(qi::Path::fromNative(::qi::Application::argv()[0])).str();
      }
#else
      globalRealProgram = details::searchExecutableAbsolutePath(qi::Path::fromNative(::qi::Application::argv()[0])).str();
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

  boost::program_options::options_description& Application::options()
  {
    return _options;
  }

  std::string Application::helpText()
  {
    std::ostringstream ss;
    ss << _options;
    return ss.str();
  }

}
