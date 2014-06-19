\defgroup qipath qi::path
  \brief Provide access to application's path, this include configuration, data, executable and library paths. \n
         See \ref qipathguide for a global overview of path related functions \n


  Provide access to various path including:
  - executables, qi::path::findBin
  - libraries, qi::pathfindLib
  - application data qi::path::findData and qi::path::listData
  - application configuration qi::path::findConf
  - writable data and configuration paths qi::path::userWritableConfPath, qi::path::userWritableDataPath

  Since this library is cross-platform we need to take care of different charsets and localizations (UTF-8, UTF-16).
  - Apple OS X always use UTF-8 locale and path.
    OS X provide a Posix API that do not care about locale.
  - Windows (including Cygwin and MinGW) have both 8 bits functions using
    various charsets depending on the Windows localization setup, and 16
    bits functions using UTF-16. To support internationalization we need
    to always use UTF-16 functions. (_wfopen, _wopen, _wsystem, ...).
  - Linux distributions use UTF-8 locale and path by default.
    Linux provide a Posix API that do not care about locale.

  To support internationalization we will always consider path to be encoded in UTF-8 under Windows.
  We will convert them to UTF-16 to pass them to the native windows API.
  On Posix platform we have nothing to do.

  We recommand to use boost::filesystem::path with and imbued UTF-8 locale.
  you could use this code in your main to initialise boost::filesystem locale once:
  \code
  // create a locale with a unicode facet to convert between char(utf-8) and wchar(utf-16/utf-32)
  std::locale loc(std::locale(), &qi::unicodeFacet());
  // Make boost.filesystem always use the unicode facet
  boost::filesystem::path::imbue(loc);

  // it's although possible to set the locale as global.
  // This will enable UTF8 supportfor iostream.
  std::locale::global(loc);
  \endcode

\page qipathguide qi::path Developer Guide

go to the \ref qipath API reference.

.. _guide-cxx-path:

Overview
========

The SDK Layout
++++++++++++++

The main idea of the qibuild-based project is that you always end up
with the same layout.

For instance, right after having built you project, you end up with
a directory looking like this.

Here we assume you have a `foo` executable which:
- depends on a `bar` dynamic library
- need to read data from a file named `foo.data` and XML files in a `models` directory
- need to read configuration from a file named `foo.cfg`


::

  src
  |__ foo
      |__ data
      |   |__ foo.data
      |   |__ models
      |       |__ nao.xml
      |       |__ romeo.xml
      |__ etc
          |__ foo.cfg

  build
  |__ sdk
      |__ lib
      |    |__ libbar.so
      |__ bin
           |__ foo


When everything is installed, you have something like:

::

  prefix
  |__ lib
  |   |__ libbar.so
  |__ bin
  |   |__ foo
  |__ share
  |   |__ foo
  |       |__ foo.data
  |       |__ models
  |           |__ nao.xml
  |           |__ romeo.xml
  |__ etc
      |__ foo
          |__ foo.cfg


The problem
+++++++++++

Here is a list of common requirements:

- Find the files `foo/foo.cfg`, `foo/foo.data` and `foo/models/*.xml`
  in a clean, simple way,  while making sure the solution works whereas the
  project is run from the build directory or installed

- The executable `foo` may need to write or update its configuration
  files or data but we need to make sure nothing will be written inside
  the installed directory

- Since there will be several `foo.cfg` files, we need to be able to process
  then in a correct order.


The solution
++++++++++++

Here is how it works:

- First we introduce the concept of **prefix**. When something is built,
  the prefix is `/path/to/build/sdk`, when something is installed, the prefix
  is the `DESTDIR` plus the installation prefix.

- Then we make sure the layout in the build prefix and in the install prefix is
  always the same.
  For instance, we will have CMake rules to be sure that whenever the foo project
  is configured, a copy of `foo.cfg` is placed in `build/sdk/foo/foo.cfg`
  (same thing for data)


- At last, we provide an easy way to get the prefix anywhere from the c++ code.
  The idea is that it is easy to get the prefix from `argv0`.
  For instance, if `argv0` is `/path/to/build/sdk/bin/foo`, we can assume
  the prefix is `/path/to/build/sdk`.


Using Namespace path
====================

Notes & Requirement
+++++++++++++++++++

The `qi::path` always make sure that:

- returned path are absolute, native paths. (with "/" on UNIX and "\\" on windows)
- path always MUST be in UTF-8 encoding (every methods who need UTF-8
  charset is specified in `qi::path`),
- return path will be in UTF-8 charset.

For this to work, we must make sure that

- `qi::init` has been called.

Have a look on the `qi::path` for more details.

Reading and writing configuration files
+++++++++++++++++++++++++++++++++++++++

Writing a configuration file is very different from reading one.

Let's assume the `foo` executable want to make sure that
`SPAM=42` in `foo.cfg`.

Here is how it works:

- First, ask for a **list** of possible paths for `foo.cfg`

- Iterate through this list and stop when the first possible `foo.cfg` is found

- Read and update the `foo.cfg` file

- Write the `foo.cfg` file.


You can see that we ask for a **list** of paths when reading, but that we
always write to **one** file.


Let's go through these steps again, assuming `foo` is installed in
`/usr/bin/foo`, and `foo.cfg` in `/usr/share/foo/foo.cfg`, and that there
is nothing else on the machine where `foo` is running.

- First step: ask for a list of possible paths for `foo.cfg`
  using `qi::path::getConfigurationPaths`
  This gives a list looking like :
  `[~/.config/foo/foo.cfg, /usr/share/foo/foo.cfg]`

- Since `.config/foo/foo.cfg` does not exist, we read `/usr/share/foo/foo.cfg`

- Then we ask for a location to write: using qi::path::getConfigurationPaths
  In this case it's `~/.config/foo/foo.cfg`
  So we write `SPAM=42` to `~/.config/foo/foo.cfg`


Then each time a piece of code will ask for the `foo.cfg` path, it will
get a list starting with `~/.config/foo/foo.cfg`, so we are sure the setting
`SPAM=42` will be used.

Example
=======

.. literalinclude:: examples/qipath_example.cpp
   :language: cpp
