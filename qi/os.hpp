#pragma once
/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_OS_HPP_
# define _QI_OS_HPP_

# include <cstdio>
# include <string>
# include <map>
# include <vector>
# include <csignal>
# include <type_traits>
# include <qi/api.hpp>
# include <qi/types.hpp>
# include <qi/path.hpp>
# include <qi/clock.hpp>
# include <qi/uuid.hpp>
# include <qi/ptruid.hpp>

# include <boost/lexical_cast.hpp>

#ifdef _WIN32
#ifndef SIGKILL
#define SIGKILL 9
#endif
#endif

// `PTRDIFF_MIN` and `PTRDIFF_MAX` may not be defined on some platforms (e.g.
// android arm32). As a first step we define the values by hard-coding them, to
// avoid relying on another constant. This guarantees that it will work whatever
// the presence or absence of other constants (INT32_MAX, etc.).
//
// TODO: Remove this when alternatively
//  - qibuild is modified to generate a libqi user CMakeLists.txt that defines
//    correct constants (`#define __STDC_LIMIT_MACROS`)
//  - Android NDK is upgraded to use at least Android API level 21.
//
// The following values are taken from the Android NDK file
// 'platforms/android-21/arch-arm/usr/include/stdint.h'.
#if !defined(PTRDIFF_MIN)
#   if defined(__LP64__) && __LP64__
#     define PTRDIFF_MIN -9223372036854775808LL
#   else
#     define PTRDIFF_MIN -2147483648
#   endif
#endif

#if !defined(PTRDIFF_MAX)
#   if defined(__LP64__) && __LP64__
#     define PTRDIFF_MAX 9223372036854775807LL
#   else
#     define PTRDIFF_MAX 2147483647
#   endif
#endif

struct stat;

namespace qi {

  /**
   * \brief OS abstraction layer.
   * \includename{qi/os.hpp}
   * \verbatim
   * This module provides some functions following POSIX convention to manipulate
   * the operating system layer in a cross-platform way.
   *
   * .. note::
   *
   *     Every path taken in parameter *must* be encoded in UTF-8. Every path
   *     returned is encoded in UTF-8.
   * \endverbatim
   */
  namespace os {
    /**
     * \brief Open a file and returns and handle on it.
     * \param filename Path to the file (in UTF-8).
     * \param mode The mode.
     * \return A FILE* handle if successful, 0 on error.
     *
     * \verbatim
     * Nothing special under POSIX systems, it's only useful for Windows,
     * where files should be open using a widestring.
     *
     * Refer to ``man 3 fopen`` for more informations on POSIX systems, and to the
     * documentation of _wfopen on MSDN to understand the Windows behaviors.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API FILE* fopen(const char *filename, const char *mode);
    /**
     * \brief Get file status.
     * \param filename Path to the file (in UTF-8).
     * \param pstat A pointer to a stat structure that will be filled by the function.
     * \return 0 on success, -1 on error
     *
     * \verbatim
     * Stats the file pointed to by filename and fills in pstat.
     * You need to include <sys/stat.h> to get access to struct.
     *
     * .. todo::
     *     explain how to use stat on windows !
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int stat(const char *filename, struct stat *pstat);
    /**
     * \brief Check if the current process is running into a debugger
     * \return -1 on error, 1 if debugger is present, 0 otherwise
     */
    QI_API int checkdbg();
    /**
     * \brief Return path to the current user's HOME.
     * \throws On Android, a `std::runtime_error` explaining that this function is not available.
     * \return String to user's HOME
     */
    QI_API std::string home();
    /**
     * \brief Return a writable temporary directory.
     * \param prefix Prefix of the tmp file (in UTF-8).
     * \return The path to the temporary directory.
     *
     * \verbatim
     * The caller is responsible for destroying the returned directory. This will
     * create a unique directory in the temporary directory returned by
     * :cpp:func:`qi::os::tmp()`.
     *
     * The specified prefix will be prepended to a uniquely generated name.
     *
     * .. versionadded:: 1.12.1
     * \endverbatim
     */
    QI_API std::string mktmpdir(const char *prefix = "");
    /**
     * \brief Return the system's temporary directory.
     * \return The path to the system's temporary directory.
     *
     * The directory is writable and exists. The caller is responsible for destroying
     * the temporary files it creates.
     */
    QI_API std::string tmp();
    /**
     * \brief Create a symlink from source to destination.
     */
    QI_API void symlink(const qi::Path& source, const qi::Path& destination);
    /**
     * \brief Get the system's hostname.
     * \throws On Android, a `std::runtime_error` explaining that this function is not available.
     * \return The system's hostname. An empty string is returned on failure.
     */
    QI_API std::string gethostname();
    /**
     * \brief Test if descriptor represents a terminal
     * \param fd File descriptor.
     * \return Returns a value of 1 if the given file descriptor is a terminal.
     * Otherwise returns a value of 0. By default test for stdout (1).
     *
     * \verbatim
     * .. versionadded:: 1.22
     * \endverbatim
     */
    QI_API int isatty(int fd = 1);

    /**
     * \brief Implement POSIX compliant fnmatch.
     */
    QI_API bool fnmatch(const std::string &pattern, const std::string &string);

    // lib C
    /**
     * \brief Implement POSIX compliant strdup.
     */
    QI_API char* strdup(const char *src);
    /**
     * \brief Implement POSIX compliant snprintf.
     *
     * \verbatim
     * .. seealso:: http://en.cppreference.com/w/cpp/io/c/fprintf
     * \endverbatim
     */
    QI_API int snprintf(char *str, size_t size, const char *format, ...);

    /**
     * \brief Get an environment variable.
     * \param var The environment variable to search for.
     * \return The value in the environment, or an empty string if
     *         there is no match.
     *
     * \verbatim
     * Searches the  environment list to find the environment variable var, and
     * returns a pointer to the corresponding value string.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API std::string getenv(const char *var);

    /**
     * \brief Get the path separator.
     * \return The separator of paths.
     *
     * \verbatim
     * It is the separator we can found in PATH env for example.
     * \endverbatim
     */
    QI_API std::string pathsep();

    /**
     * \brief Change or add an environment variable.
     * \param var The variable name.
     * \param value The value of the variable.
     * \return 0 on success, or another unspecified value on error, in which case `errno` is set to
     *   indicate the cause of the error.
     *
     * \verbatim
     * Adds the variable name to the environment with the value in argument if name
     * does not already exist. Else changes its value.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int setenv(const char *var, const char *value);

    /**
     * \brief Remove an environment variable.
     * \param var The variable name. If null, the behavior is unspecified.
     * \return 0 on success, or another unspecified value on error, in which case `errno` is set to
     *   indicate the cause of the error.
     *
     * \verbatim
     * Removes the variable name from the environment. If the variable did not already exist or was
     * unset, the environment is unchanged and the function returns with a success.
     * \endverbatim
     * \post `getenv(var).empty()`
     *
     */
    QI_API int unsetenv(const char *var);

    /**
     * \brief Return the timezone.
     * \return A string with the timezone.
     *
     * \verbatim
     * .. versionadded:: 1.22
     * \endverbatim
     */
    QI_API std::string timezone();

    // time
    /**
     * \brief Sleep for the specified number of seconds.
     * \param seconds Number of seconds to sleep.
     *
     * \verbatim
     * Under Linux/OS X it will not be disturbed by eventual signals. Makes the
     * calling thread sleep until seconds have elapsed or a signal arrives which
     * is not ignored.
     *
     * .. todo::
     *     Explain the behavior of signals on windows when sleeping.
     *
     * .. seealso:: :cpp:func:`qi::os::msleep(unsigned int)`
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API_DEPRECATED_MSG(please use std::this_thread::sleep_for instead)
    QI_API void sleep(unsigned int seconds);

    /**
     * \brief Sleep for the specified number of milliseconds.
     * \param milliseconds Number of milliseconds to sleep.
     *
     * \verbatim
     * Under Linux/OSX it will not be disturbed by eventual signals sent to process.
     * Makes the calling thread sleep until millliseconds have elapsed or a signal
     * which is not ignored arrives.
     *
     * .. seealso:: :cpp:func:`qi::os::sleep(unsigned int)`
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API_DEPRECATED_MSG(please use std::this_thread::sleep_for instead)
    QI_API void msleep(unsigned int milliseconds);

    /**
     * \brief struct similar to POSIX timeval
     */
    struct QI_API timeval {
      qi::int64_t tv_sec;  ///< seconds
      qi::int64_t tv_usec; ///< microseconds

      timeval() : tv_sec(0), tv_usec(0) {}
      timeval(int64_t sec, int64_t usec) : tv_sec(sec), tv_usec(usec) {}
      explicit timeval(int64_t usec);
      explicit timeval(const qi::Duration &);
      explicit timeval(const qi::SystemClockTimePoint &);
    };

    /**
     * \brief The gettimeofday() function shall obtain the current time.
     * \param tp The timeval structure used to return the current time.
     * \return 0 on success
     * \deprecated since 2.4. Use qi::SystemClock::now() instead.
     *
     * The gettimeofday() function shall obtain the current time, expressed as
     * seconds and microseconds since the Epoch, and store it in the timeval
     * structure pointed to by tp. The resolution of the system clock is
     * unspecified. This clock is subject to NTP adjustments.
     */
    QI_API QI_API_DEPRECATED int gettimeofday(qi::os::timeval *tp);
    /**
     * \brief Elapsed time since program started in microseconds.
     * \return Return qi::int64_t in microseconds.
     */
    QI_API qi::int64_t ustime();
    /**
     * \brief Return CPU time used by the calling thread as a pair (userTime, systemTime)
     *        in microseconds.
     * \return Return pair(userTime, systemTime) in microseconds.
     *
     * \warning under Linux, systemTime is always 0 and merged in userTime. Also value
     * might be inaccurate if CPU frequency scaling is used and the thread is not
     * constrained to a single core.
     */
    QI_API std::pair<int64_t, int64_t> cputime();

    /**
     * \brief Add two timeval together
     * \param lhs First timeval
     * \param rhs Second timeval
     */
    QI_API qi::os::timeval operator+(const qi::os::timeval &lhs,
                                     const qi::os::timeval &rhs);
    /**
     * \brief Add a delay to a timeval (in microsecond)
     * \param lhs Frist timeval
     * \param us Second timeval
     */
    QI_API qi::os::timeval operator+(const qi::os::timeval &lhs,
                                     long                   us);
    /**
     * \brief Substract two timeval together
     * \param lhs First timeval
     * \param rhs Second timeval
     */
    QI_API qi::os::timeval operator-(const qi::os::timeval &lhs,
                                     const qi::os::timeval &rhs);
    /**
     * \brief Substract a delay to a timeval (in microsecond)
     * \param lhs Frist timeval
     * \param us Second timeval
     */
    QI_API qi::os::timeval operator-(const qi::os::timeval &lhs,
                                     long                   us);

    /**
     * \brief Load a dynamic library.
     * \param filename Name of the dynamic library.
     * \param flag Flags used to load the dynamic library.
     * \return A handle to the library, or 0 on error.
     *
     * \verbatim
     * Loads the dynamic library file named by the null-terminated string filename
     * and returns an opaque "handle" for the dynamic library. If filename is NULL,
     * then the returned handle is for the main program.
     *
     * No flag is supported on windows platform. Otherwise, see ``man 0p dlfcn.h``
     * for more information on flags available. If not given, ``RTLD_NOW`` is used.
     *
     * .. seealso:: :cpp:func:`qi::os::dlerror()` for more details on the error.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API void *dlopen(const char *filename, int flag = -1);
    /**
     * \brief Decrements the reference count on the dynamic library.
     * \param handle The dynamic library handle.
     * \return This function returns 0 on success, and non-zero on error.
     *
     * \verbatim
     * Decrements the reference count on the dynamic library handle. If the
     * reference count drops to zero and no other loaded library uses symbols in
     * it, then the dynamic library is unloaded.
     *
     * If there is an error you can look which one with dlerror function provided in
     * this same module.
     *
     * .. seealso:: :cpp:func:`qi::os::dlerror()` for more details on the error.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int dlclose(void *handle);
    /**
     * \brief Get the address where the symbol is loaded into memory.
     * \param handle Handle on a dynamic library returned by dlopen().
     * \param symbol The null-terminated symbol name.
     * \return The address of the symbol, or 0 on error.
     *
     * \verbatim
     * If the symbol is not found in the specified library or any of the libraries
     * that were automatically loaded by :cpp:func:`qi::os::dlopen()` when that
     * library was loaded, :cpp:func:`qi::os::dlsym()` returns 0.
     *
     * .. seealso:: :cpp:func:`qi::os::dlerror()` for more details on the error.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API void *dlsym(void *handle, const char *symbol);
    /**
     * \brief Returns a human readable string of the error code.
     * \return NULL if no error has occurred since it was last called.
     *         An human readable string otherwise.
     *
     * .. warning::
     *      On windows, return value may be modified by another function
     *      unrelated to qi::os::dlopen familly.
     *      This function does not ensure that error value is 0 at initialisation.
     *      You may reset error value before a call to any qi::os::dl{open, sym, close}
     *      functions with a first call to this function.
     *
     * \verbatim
     * Returns a human readable string describing the most recent error that
     * occurred from :cpp:func:`qi::os::dlopen(const char*, int)`,
     * :cpp:func:`qi::os::dlsym(void*, const char*)` or
     * :cpp:func:`qi::os::dlclose(void*)` since the last call to
     * :cpp:func:`qi::os::dlerror()`.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API const char *dlerror();

    // process management
    /**
     * \brief Create and execute a new process.
     * \param argv The command line arguments of the new process as an array (NULL
     *             terminated).
     * \return -1 on error, child pid otherwise.
     *
     * \verbatim
     * Creates and executes a new process.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int spawnvp(char *const argv[]);
    /**
     * \brief Create and execute a new process.
     * \param argv Path to the file to be executed.
     * \param ... The command line arguments of the new process as var args.
     * \return -1 on error, child pid otherwise.
     *
     * \verbatim
     * Creates and executes a new process.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int spawnlp(const char* argv, ...);
    /**
     * \brief Execute a shell command
     * \param command Command to execute.
     * \return The value returned is -1 on error, and the return status of the
     *         command otherwise.
     *
     * \verbatim
     * Executes a command by calling ``/bin/sh -c command``, and returns when the
     * command has completed.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int system(const char *command);
    /**
     * \brief Get the process identifier.
     */
    QI_API int getpid();
    /**
     * \brief Get the thread identifier.
     */
    QI_API int gettid();
    /**
     * \brief Wait for process to change state.
     * \param pid Pid to wait.
     * \param status Status returned by the process.
     * \return See detailed description.
     *
     * \verbatim
     * Suspends execution of the calling process until a child specified by pid
     * argument changes of state.
     *
     * Return values (rc):
     *
     * - rc = 0 means that everything went well.
     * - rc > 0 means that an error occurred. (For instance, no process
     *   corresponding to the pid was found). The exact value is an errno code.
     * - rc < 0 means that the child was killed by a signal. The value of the signal
     *   is -rc.
     *
     * .. versionadded:: 1.12
     * \endverbatim
     */
    QI_API int waitpid(int pid, int* status);
    /**
     * \brief Send a signal to a process.
     * \param pid PID to kill.
     * \param sig Signal to deliver to the process.
     * \return See detailed description.
     *
     * \verbatim
     * The kill() function shall send a signal to a process or a group of processes
     * specified by pid.
     *
     * Return value (rc):
     *
     * - rc = 0 means that everything went well.
     * - rc != 0 means that an error occurred. (For instance, no process
     *   corresponding to the pid was found).
     *
     * .. versionadded:: 1.14
     * \endverbatim
     */
    QI_API int kill(int pid, int sig);

    /**
     * \brief Check whether a process is running, given its file name and pid.
     * \warning On Linux, since the command line of the process takes a little
     * time to be made available, the file name check may fail if the process
     * was spawned too recently.
     * \param pid The PID to check.
     * \param fileName The name of the process: the executable file name
     * with no .exe or _d.exe extension.
     * \return true if the process is running and has the expected name.
     */
    QI_API bool isProcessRunning(int pid, const std::string& fileName = std::string());

    /**
     * \brief Find the first available port starting at port number in parameter.
     * \param port First port tested, then each port following it is tested
     *             one by one until one available is found.
     * \return Available port or 0 on error
     *
     * \verbatim
     * .. warning::
     *
     *     This function is not thread safe and suffers from a race condition. You
     *     should avoid calling it and call listen on port 0 instead. This will pick
     *     a random port with no race condition.
     *
     * .. versionadded:: 1.14
     * \endverbatim
     */
    QI_API unsigned short findAvailablePort(unsigned short port);
    /**
     * \brief Find all network adapters and corresponding IPs.
     * \param ipv6Addr Look for IPv6 addresses instead of IPv4 ones.
     * \return A map of interfaces associated with the list of IPs of that interface.
     *
     * \verbatim
     * .. versionadded:: 1.14
     * \endverbatim
     */
    QI_API std::map<std::string, std::vector<std::string> > hostIPAddrs(bool ipv6Addr = false);

    /**
     * \brief Set the current thread name to the string in parameter.
     * \param name The new name of the current thread.
     *
     * Prefer using ScopedThreadName that will restore the thread name on exit.
     *
     * \warning this feature can be considered as slow and should only used when the task is long
     */
    QI_API void setCurrentThreadName(const std::string &name);
    /**
     * \brief returns the current thread name as a std::string
     * \return a std::string of at most 16 characters
     * \warning Not implemented on Windows, always returns an empty string
     */
    QI_API std::string currentThreadName();


    /**
     * \brief Set the current thread name and restore it after use.
     *
     * \warning this feature can be considered as slow and should only used when the task is long
     */
    class QI_API ScopedThreadName {
    public:
      ScopedThreadName(const std::string& newName) : _oldName(currentThreadName()) {
        setCurrentThreadName(newName);
      };
      ~ScopedThreadName() {
        setCurrentThreadName(_oldName);
      }
    private:
      std::string _oldName;
    };


    /**
     *  \brief Set the CPU affinity for the current thread.
     *  \param cpus a vector of CPU core ids
     *  \return true on success
     *  \warning This function has no effect under Android nor OSX.
     *
     * \verbatim
     * When CPU Affinity is set, the current thread will only be allowed
     * to run on the given set of cores.
     *
     * .. versionadded:: 1.22
     * \endverbatim
     */
    QI_API bool setCurrentThreadCPUAffinity(const std::vector<int> &cpus);
    /**
     *  \brief Get the number of CPUs on the local machin
     *  \return Number of CPUs
     */
    QI_API long numberOfCPUs();

    /**
     * \brief Returns an UUID identifying the machine, as a best effort.
     * \return The uuid of the machine as a string.
     *
     * When libsystemd is available, the UUID is generated from systemd's machine-id.
     *
     * On Android, as a best effort as we do not assume any shared file access between processes,
     * an UUID is generated for the process once and persists for its duration, so that each call
     * of this function will return the same result. This implies that different processes on a
     * same Android device will *NOT* share the same machine-id.
     *
     * Otherwise, the UUID is read from the machine-id file. If the file does
     * not exist, the UUID is randomly generated and stored in the file.
     *
     * The returned machine-id is never null (00000000-0000-0000-0000-000000000000).
     *
     * \throws On failure, throws a `std::exception`.
     */
    QI_API std::string getMachineId();

    /**
     * \brief Same as getMachineId but return a uuid and not its string representation.
     */
    QI_API const Uuid& getMachineIdAsUuid();
    /**
     * \brief Returns an unique uuid for the process.
     * \return The uuid of the process.
     */
    QI_API const Uuid& getProcessUuid();
    /**
     * \brief Generate a universally unique identifier.
     * \return The uuid.
     * .. versionadded:: 1.20
     */
    QI_API std::string generateUuid();
    /**
     * \param pid PID of a process
     * \brief Get the memory usage of a process in kB
     * \return Returns a value > 0 corresponding to the memory in RAM used by the process in kB.
     * Otherwise returns a value of 0, meaning that it was impossible to get the memory usage.
     */
    QI_API size_t memoryUsage(unsigned int pid);

    /** Constructs a PtrUid using process and machine ids provided by qi::os implementation.
    */
    QI_API PtrUid ptrUid(void* address);

    /**
     * \brief Returns the value of the environment variableif set, the defaultVal otherwise.
     * \param name Name of the environment variable
     * \param defaultVal DefaultVal to return if the environment variable isn't set.
     * \return Function obtains the current value of the environment variable, name if set.
     * Return defaultVal otherwise.
     * .. versionadded:: 2.4
     */
    template <typename T>
    inline T getEnvParam(const char* name, T defaultVal)
    {
      std::string sval = qi::os::getenv(name);
      if (sval.empty())
        return defaultVal;
      else
        return boost::lexical_cast<T>(sval);
    }

    /// (Arithmetic or Enum) N
    template<typename N>
    std::string to_string(N n)
    {
      // Prevent the Android version to behave differently than other platforms.
      // Enums are implicitly convertible to integral types, so we accept them.
      static_assert(std::is_arithmetic<N>::value || std::is_enum<N>::value,
        "to_string() accepts only arithmetic types (i.e. integral types and "
        "floating-point types) and enum types.");

#if ANDROID && BOOST_COMP_GNUC
      // workaround android gcc missing std::to_string on arm
      // http://stackoverflow.com/questions/17950814/how-to-use-stdstoul-and-stdstoull-in-android/18124627#18124627
      std::ostringstream stream;
      stream << n;
      return stream.str();
#else
      return std::to_string(n);
#endif
    }
  }
}

#endif  // _QI_OS_HPP_
