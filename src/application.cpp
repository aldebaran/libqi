/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <fstream>

#include <qi/application.hpp>
#include <qi/qi.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/path.hpp>
#include <src/sdklayout.hpp>
#include <numeric>
#include <locale>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "filesystem.hpp"
#include "utils.hpp"
#include "path_conf.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef WITH_INTL
# include <libintl.h>
extern int _nl_msg_cat_cntr;
#endif



namespace qi {
  static int         globalArgc = -1;
  static char**      globalArgv = 0;
  static bool        globalInitialized = false;

  static std::string globalName;
  static std::vector<std::string>* globalArguments;
  static std::string globalPrefix;
  static std::string globalProgram;

  typedef std::vector<boost::function<void()> > FunctionList;
  static FunctionList* globalAtExit = 0;
  static FunctionList* globalAtEnter = 0;
  static FunctionList* globalAtStop = 0;


  static boost::condition_variable globalCond;

  static boost::asio::io_service*             globalIoService = 0;
  static boost::thread*                       globalIoThread = 0;
  static boost::asio::io_service::work*       globalIoWork = 0;
  static std::list<boost::asio::signal_set*>* globalSignalSet = 0;


  static void readPathConf()
  {
    std::string prefix = ::qi::path::sdkPrefix();
    std::set<std::string> toAdd =  ::qi::path::detail::parseQiPathConf(prefix);
    std::set<std::string>::const_iterator it;
    for (it = toAdd.begin(); it != toAdd.end(); ++it) {
      ::qi::path::detail::addOptionalSdkPrefix(it->c_str());
    }
  }

  static void stop_io_service()
  {
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
    globalIoService->run();
  }

  static void stop_handler(int signal_number) {
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
        qiLogInfo("qi.application") << "Sending the stop command...";
        //register the signal again to call exit the next time if stop did not succeed
        Application::atSignal(boost::bind<void>(&stop_handler, _1), signal_number);
        // Stop might immediately trigger application destruction, so it has
        // to go after atSignal.
        Application::stop();
        return;
      default:
        //even for SIGTERM this is an error, so return 1.
        qiLogInfo("qi.application") << "signal " << signal_number << " received a second time, calling exit(1).";
        exit(1);
        return;
    }
  }

  static void signal_handler(const boost::system::error_code& error, int signal_number, boost::function<void (int)> fun)
  {
    //when cancel is called the signal handler is raised with an error. catch it!
    if (!error) {
      fun(signal_number);
    }
  }

  bool Application::atSignal(boost::function<void (int)> func, int signal)
  {
    if (!globalIoService)
    {
      globalIoService = new boost::asio::io_service;
      // Prevent run from exiting
      globalIoWork = new boost::asio::io_service::work(*globalIoService);
      // Start io_service in a thread. It will call our handlers.
      globalIoThread = new boost::thread(boost::bind(&run_io_service));
      atExit(&stop_io_service);
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
    readPathConf();
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
    globalCond.notify_all();
  }

  static void initSigIntSigTermCatcher() {
    bool signalInit = false;

    if (!signalInit) {
      qiLogVerbose("qi.Application") << "Registering SIGINT/SIGTERM handler within qi::Application";
      // kill with no signal sends TERM, control-c sends INT.
      Application::atSignal(boost::bind(&stop_handler, _1), SIGTERM);
      Application::atSignal(boost::bind(&stop_handler, _1), SIGINT);
      signalInit = true;
    }
  }

  void Application::run()
  {
    //run is called, so we catch sigint/sigterm, the default implementation call Application::stop that
    //will make this loop exit.
    initSigIntSigTermCatcher();

    // We just need a barrier, so no need to share the mutex
    boost::mutex m;
    boost::unique_lock<boost::mutex> l(m);
    globalCond.wait(l);
    l.unlock();
  }

  void Application::stop()
  {
    globalCond.notify_all();
    FunctionList& fl = lazyGet(globalAtStop);
    qiLogDebug("qi.Application") << "Executing " << fl.size() << " atStop handlers";
    for (FunctionList::iterator i = fl.begin(); i!= fl.end(); ++i)
      (*i)();
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


  void Application::loadTranslationDict(const std::string &dictName)
  {
    // find conf file to know dictionary path
    std::string applicationData = fsconcat("locale", qi::Application::name());
    boost::filesystem::path intlConfPath(::qi::path::findData(applicationData, ".confintl"),
                                         ::qi::unicodeFacet());
    std::string parentDirDict = intlConfPath.parent_path().string(::qi::unicodeFacet());

#ifdef WITH_INTL
    // default local initialisation: take environment variable into account.
    setlocale(LC_ALL, "");
    bindtextdomain(dictName.c_str(), parentDirDict.c_str());
    textdomain(dictName.c_str());
#endif
  }

  bool Application::setTranslationLocale(const std::string &locale)
  {
    std::string appName = qi::Application::name();
    std::string relAppData = fsconcat("locale", appName);

    std::string dictPath = ::qi::path::findData(relAppData, fsconcat(locale, "LC_MESSAGES", appName + ".mo"));
#ifdef WITH_INTL
    if (!dictPath.empty())
    {
      // For more explanation take a look at
      // http://www.gnu.org/software/gettext/manual/html_node/gettext-grok.html
      ::qi::os::setenv("LANGUAGE", locale.c_str());

      ++_nl_msg_cat_cntr;
      return true;
    }
#endif
    return false;
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

  bool Application::atStop(boost::function<void()> func)
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

  //this is not threadsafe
  void init(int argc, char* argv[])
  {
    static qi::Application *app = 0;

    qiLogError("qi") << "qi::init() is deprecated, use qi::Application";
    if (!app)
      app = new qi::Application(argc, argv);
  }
}
