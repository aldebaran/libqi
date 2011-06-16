#pragma once
/**
 * Author(s):
 *  - Herve CUCHE <hcuche@aldebaran-robotics.com>
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
 */

/** @file
 *  @brief os related cross platform functions
 */

#ifndef _QI_OS_HPP_
#define _QI_OS_HPP_

# include <string>
# include <qi/api.hpp>

struct stat;

/**
 * \namespace qi::os
 * \brief OS abstraction layer.
 *
 * Include posix compatibility support for OS not following posix.
 *
 * \warning Every string MUST be encoded in UTF8 and return UTF-8.
 *
 */
namespace qi {

  // namespaced because open/fopen/stat/... are available in the lib c.
  // it's mostly stuff to make windows works like a standard posix system.
  namespace os {


    /** \brief Stream open a file.
     *
     * Nothing special under posix systems, it's only useful for Windows,
     * where files should be open using a widestring.
     * Refer to 'man 3 fopen' for more informations, and to the documentation of
     * _wfopen on MSDN to understand the Windows behaviors.
     * \param filename Path to the file (in UTF-8).
     * \param mode The mode.
     * \return A FILE* handle, 0 on error.
     */
    QI_API FILE* fopen(const char *filename, const char *mode);

    /** \brief Get file status.
     *
     *  Stats the file pointed to by filename and fills in pstat.
     *  \param filename Path to the file (in UTF-8).
     *  \param pstat A pointer to a struct stat that will be filled
     *  by the function.
     *  You need to include <sys/stat.h> to get access to struct stat.
     *  \return 0 on success, -1 on error
     */
    // FIXME: explain how to use stat on windows !
    // (it's definitely NOT in <sys/stat.h> ...
    QI_API int stat(const char *filename, struct stat *pstat);

    /** \brief Get an environment variable.
     *
     *  Searches  the  environment
     *  list to find the environment variable var,
     *  and returns a pointer to the corresponding value string.
     *  \param var The environment variable to search for.
     *  \return A pointer to the value in the environment,
     *  or an empty string if there is no match.
     */
    QI_API std::string getenv(const char *var);

    /** \brief Change or add an environment variable.
     *
     *  Adds the variable name to the environment with the value
     *  var, if name does not already exist. If var does exist
     *  in the environment, then its value is changed to value
     *  \param var The variable name.
     *  \param value The value of the variable.
     *  \return 0 on success, or -1 on error.
     */
    QI_API int setenv(const char *var, const char *value);

    /** \brief Check if the current program is running under a debugger.
     *  \warning Not implement for windows.
     *  \return -1 on error, 1 if the program is currently being debugged, 0 otherwize.
     */
    QI_API int check_debugger();

    /** \brief Sleep for the specified number of seconds.
     *
     *  Under Linux/OSX it will not be disturbed by eventual signals.
     *  Makes the calling thread sleep until seconds have elapsed
     *  or a signal arrives which is not ignored.
     *  \param seconds Number of second to sleep for
     */
    QI_API void sleep(unsigned int seconds);

    /** \brief Sleep for the specified number of milliseconds.
     *
     *  Under Linux/OSX it will not be disturbed by eventual signals.
     *  Makes the calling thread sleep until millliseconds have elapsed
     *  or a signal arrives which is not ignored.
     *  \param milliseconds Number of milliseconds to sleep for
     */
    QI_API void msleep(unsigned int milliseconds);



    /** \brief Loads dynamic library.
     *
     *  Loads the dynamic library file named by the null-terminated
     *  string filename and returns an opaque "handle" for
     *  the dynamic library. If filename is NULL, then the returned
     *  handle is for the main program.
     *
     *  \param filename Name of the dynamic library.
     *  \param flag Flags to load the dynamic library.
     */
    QI_API void *dlopen(const char *filename, int flag = -1);

    /** \brief Decrements the reference count on the dynamic library.
     *
     *  Decrements the reference count on the dynamic library handle.
     *  If the reference count drops to zero and no other loaded libraries
     *  use symbols in it, then the dynamic library is unloaded.
     *
     *  \param handle The dynamic library handle.
     *  \return 0 on success, and nonzero on error.
     */
    QI_API int dlclose(void *handle);

    /** \brief Get the address where the symbol is loaded into memory.
     *
     *  If the symbol is not found, in the specified library or any
     *  of the libraries that were automatically loaded by dlopen()
     *  when that library was loaded, dlsym() returns NULL.
     *
     *  \param handle Handle of a dynamic library returned by dlopen().
     *  \param symbol The null-terminated symbol name.
     */
    QI_API void *dlsym(void *handle, const char *symbol);

    /** \brief Returns a human readable string.
     *
     *  Returns a human readable string describing the most recent error
     *  that occurred from dlopen(), dlsym() or dlclose() since the last
     *  call to dlerror().
     *
     *  \return It returns NULL if no errors have occurred since
     *  initialization or since it was last called or the human
     *  readable string.
     */
    QI_API const char *dlerror(void);



    /** \brief Create and execute a new process.
     *
     *  Creates and executes a new process.
     *
     *  \param argv The command line arguments of the new process
     *              as an array (NULL terminated).
     *  \return -1 on error, child pid otherwise.
     */
    QI_API int spawnvp(char *const argv[]);

    /** \brief Create and execute a new process.
     *
     *  Creates and executes a new process
     *
     *  \param argv Path of file to be executed.
     *  \param ... The command line arguments of the new process
     *             as var args.
     *  \return -1 on error, child pid otherwise.
     */
    QI_API int spawnlp(const char* argv, ...);

    /** \brief Execute a shell command
     *
     *  Executes a command specified in command by calling /bin/sh -c
     *  command, and returns after the command has been completed.
     *
     *  \param command Command to execute.
     *  \return The value returned is -1 on error, and the return status
     *   of the command otherwise.
     */
    QI_API int system(const char *command);

    /** \brief Wait for process to change state.
     *
     *  Suspends execution of the calling process until a
     *  child specified by pid argument has changed state.
     *
     *  \param pid Pid to wait
     *  \param status Pointer to a buffer where the return code
     *   of the specified process will be stored, or NULL.
     *  \return rc.
     *    rc = 0 means that everything went well.
     *    rc > 0 means that an error occurs. (For instance,
     *           no process corresponding to the pid was
     *           found). The exact value is an errno code.
     *    rc < 0 means that the child was killed by a signal.
     *           The value of the signal is -rc.
     */
    QI_API int waitpid(int pid, int* status);

    /**
     * \brief Return path to the current user's HOME.
     */
    QI_API std::string home();

    /**
     * \brief Return a temporary directory
     *        The directory is writeable and exits.
     *        The caller is responsible of destroying the temporary
     *        directory.
     * \param prefix Prefix of the tmp file (in UTF-8).
     * \return The path to the temporary directory.
     */
    QI_API std::string tmp(const char *prefix = "");


    /** \brief qi::os::timeval struct similar to POSIX timeval
     *
     */
    struct QI_API timeval {
      long tv_sec;
      long tv_usec;
    };

    QI_API int gettimeofday(qi::os::timeval *tp, void *tzp);

  };
};


#endif  // _QI_OS_HPP_
