#pragma once
/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_APPLICATION_HPP_
# define _QI_APPLICATION_HPP_

# include <functional>
# include <boost/program_options.hpp>
# include <vector>
# include <string>
# include <qi/api.hpp>

namespace qi {

  /**
   * \includename{qi/application.hpp}
   * \brief Class handling startup and teardown of an application.
   *
   * \verbatim
   * The :cpp:class:`qi::Application` class is designed to ease
   * startup and teardown of an executable.
   *
   * All executables using qi classes should create an instance of
   * :cpp:class:`qi::Application` on the stack of the main() function.
   * \endverbatim
   */
  class QI_API Application
  {
  public:
    /**
     * \brief Application constructor. Must be the first thing called by main().
     * \param argc Argument counter of the program.
     * \param argv Arguments of the program (given to main).
     * \param name The name of the program. It will be returned by name().
     * \param path The full path to the program if you wish to override it. It
     * will be returned by program() but not realProgram().
     * \throw std::logic_error When the constructor is called twice.
     */
    Application(int& argc, char** &argv, const std::string& name = "", const std::string& path = "");
    /**
     * \brief Application constructor. Must be the first thing called by main().
     * \param name Name of the application.
     * \param argc Argument counter of the program.
     * \param argv Arguments of the program (given to main).
     * \throw std::logic_error When the constructor is called twice.
     * \deprecated Use Application(int&, char**&, const std::string&, const
     * std::string&)
     */
    QI_API_DEPRECATED_MSG(Use 'Application(int, char**, string, string)' instead)
    Application(const std::string &name, int& argc, char** &argv);
    /**
     * \brief Application destructor. It executes atExit() callbacks.
     * \see qi:Application::atExit
     * \see QI_AT_EXIT
     */
    ~Application();

    // non-copyable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * \brief Wait until the end of the program.
     *
     * \verbatim
     * Wait until one of those conditions becomes true:
     * - stop() is called.
     * - TERM or INT signal is received.
     * - the Application instance is destroyed, which means main() is exiting.
     *
     * Run can be called by multiple threads simultaneously.
     * \endverbatim
     */
    static void run();
    /**
     * \brief Stop the application. Call all atStop handlers.
     */
    static void stop();

    /**
     * \brief Get arguments of the program as an std::vector of std::string.
     * \return List of arguments of the program.
     */
    static const std::vector<std::string>& arguments();
    /**
     * \brief Get argument counter of the program.
     * \return Argument counter of the program if Application was initialized,
     *         -1 otherwise.
     */
    static int argc();
    /**
     * \brief Get string arguments of the program (including program name).
     * \return Arguments of the program if Application was initialized, 0 otherwise.
     */
    static const char** argv();
    /**
     * \brief Set application name.
     * \param name The application's name.
     */
    static void setName(const std::string &name);
    /**
     * \brief Get application name.
     * \return A string with the application name,
     *         empty string if setName isn't call.
     */
    static std::string name();
    /**
     * \brief Set arguments of the program with argc as argument counter and argv as
     *        argument values.
     * \param argc Argument counter of the program.
     * \param argv Arguments of the program (given to main).
     */
    static void setArguments(int argc, char** argv);
    /**
     * \brief Set arguments ot the program as an std::vector of std::string.
     * \param arguments Sets arguments with a vector of strings.
     */
    static void setArguments(const std::vector<std::string>& arguments);

    /**
     * \brief Load a module into the current process.
     * \param name The module path and name. If no extension is used, the
     *        correct extension for a library on the current platform is used.
     * \param flags Extra flags to pass to the dlopen function.
     * \return A handle, to be used by qi::os::dlsym() or unloadModule().
     *
     * \verbatim
     * The module can execute code when loaded by using :cpp:macro:`QI_AT_ENTER`.
     * \endverbatim
     */
    static void* loadModule(const std::string& name, int flags=-1);
    /**
     * \brief Unload a module from the current process.
     * \param handle Handle on the loaded module.
     */
    static void unloadModule(void* handle);
    /**
     * \brief Check whether the Application instance is terminated or not.
     * \return True if it is stop, false otherwise.
     */
    static bool terminated();
    /**
     * \brief Check whether the Application instance was initialized or not.
     * \return True if it was initialized, false otherwise.
     */
    static bool initialized();

    /**
     * \brief Return the current program full path according to argv[0]
     * \return full path to the current running program, symbolic links are not
     * resolved.
     * \see realProgram
     */
    static const char* program();

    /**
     * \brief Physical absolute path of the current program.
     * \return The path or an empty string in case of errors (missing file or
     * access denial).
     *
     * The path of the current program is transformed to get a complete path
     * from the root of the filesystem, without any symbolic link.
     *
     * When using this function from a Python application for example on
     * gentoo, it will return something like /usr/bin/python2.7 (because python
     * is a symlink to python-wrapper which will exec() /usr/bin/python2.7). On
     * the other hand, program() will return the full path to the script run by
     * the Python interpreter.
     *
     * \verbatim
     * Computed using specific OS API:
     *
     * - Apple  : _NSGetExecutablePath
     * - Linux  : reading "/proc/self/exe"
     * - Windows: GetModuleFileName
     *
     * If the former API fail it will try to guess the value from argv[0].
     * For this method to work :cpp:func:`qi::Application(int&, char**&)` should
     * have been called in your main().
     * \endverbatim
     */
    static const char* realProgram();

    /**
     * \brief Return the SDK path given through --qi-sdk-prefix or QI_SDK_PREFIX
     *
     * Used internally, you should not need this.
     */
    static const char* _suggestedSdkPath();

    /**
     * \brief Register a function to be executed at Application creation.
     * \param func Callback function at Application creation.
     * \return True if registering succeeded, false otherwise.
     */
    static bool atEnter(std::function<void()> func);

    /**
     * \brief Register a function to be executed at Application destruction.
     * \param func Callback function called at Application destruction.
     * \return True if registering succeeded, false otherwise.
     */
    static bool atExit(std::function<void()> func);

    /**
     * \brief Register a function to be executed when run() is called.
     * The functions are executed sequentially at the beginning of run().
     * \param func Callback function called when stop() is called.
     * \return True if registering succeeded, false otherwise.
     */
    static bool atRun(std::function<void()> func);

    /**
     * \brief Register a function to be executed when stop() is called.
     * The functions are executed sequentially before run() returns.
     * \param func Callback function called when stop() is called.
     * \return True if registering succeeded, false otherwise.
     */
    static bool atStop(std::function<void()> func);

    /**
     * \brief Register a function to be executed when a signal occurs.
     * \param func Callback function called on signal.
     * \param signal Signal number.
     * \return True if registering succeeded, false otherwise.
     *
     * The handler is executed in a thread, not from within the signal handler,
     * so there is no restriction on what can be done by your handler function,
     * except that it should return reasonably quickly.
     */
    static bool atSignal(std::function<void(int)> func, int signal);

    /**
     * \brief Get the registered global program options.
     * \return The options_description of the currently added program options.
     */
    static boost::program_options::options_description& options();

    /**
     * \return Get the help text displayed when the `--help` option is used.
     */
    static std::string helpText();
  };
}

/**
 * \brief calls qi::Application::atEnter(func) at static initialization time.
 * \param func The handler that must be called at enter.
 */
#define QI_AT_ENTER(func)                                               \
  static bool QI_UNIQ_DEF(_qi_atenter) QI_ATTR_UNUSED = ::qi::Application::atEnter(func);

/**
 * \def QI_AT_EXIT(func)
 * \brief calls qi::Application::atExit(func) at static initialization time.
 * \param func The handler that must be called at exit.
 */
#define QI_AT_EXIT(func)                                                \
  static bool QI_UNIQ_DEF(_qi_atexit) QI_ATTR_UNUSED = ::qi::Application::atExit(func);

//THIS IS INTERNAL
//API is not maintained for this function
//The user needs to include <boost/program_options.hpp> and <boost/bind/bind.hpp>
//Use like this:
//namespace {
//  _QI_COMMAND_LINE_OPTIONS(
//    "Name of category",
//    (option1)
//    (option2)
//  )
//}
#define _QI_COMMAND_LINE_OPTIONS(desc, opts)                            \
  static void QI_UNIQ_DEF(_qi_opt_func)() {                             \
    namespace po = boost::program_options;                              \
    po::options_description options(desc);                              \
    {                                                                   \
      using namespace boost::program_options;                           \
      options.add_options() opts;                                       \
    }                                                                   \
    ::qi::Application::options().add(options);                          \
  }                                                                     \
  QI_AT_ENTER(boost::bind(&(QI_UNIQ_DEF(_qi_opt_func))))

#endif  // _QI_APPLICATION_HPP_
