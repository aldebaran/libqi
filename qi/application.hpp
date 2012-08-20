/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef LIBQI_APPLICATION_HPP_
#define LIBQI_APPLICATION_HPP_

#include <vector>
#include <qi/config.hpp>
#include <boost/function.hpp>

namespace qi {

  class QI_API Application
  {
  public:
    Application(int& argc, char** &argv);
    ~Application();

    void run();

    static const std::vector<std::string>& arguments();
    static int argc();
    static const char** argv();
    static void setArguments(int argc, char** argv);
    static void setArguments(const std::vector<std::string>& arguments);

    static void* loadModule(const std::string& name, int flags=-1);
    static void unloadModule(void* handle);
    static bool initialized();

    static const char* program();

    static bool atEnter(boost::function<void()> func);
    static bool atExit(boost::function<void()> func);
  };
}

#define QI_AT_ENTER(func) \
static bool _qi_ ## __LINE__ ## atenter = ::qi::Application::atEnter(func);

#define QI_AT_EXIT(func) \
static bool _qi_ ## __LINE__ ## atenter = ::qi::Application::atExit(func);

#define QI_AT_PARSE_OPTIONS(func) \
static bool _qi_ ## __LINE__ ## atenter = ::qi::Application::atParseOptions(func);

#define QI_COMMAND_LINE_OPTIONS(opts)                                 \
static void _qi_## __LINE__## opt_func() {                            \
  namespace po = boost::program_options;                              \
  po::variables_map vm;                                               \
  po::command_line_parser p(::qi::Application::arguments());          \
  po::options_description options;                                    \
  {                                                                   \
    using namespace boost::program_options;                           \
    options.add_options() opts;                                       \
  }                                                                   \
  options.add_options()                                               \
    ("help", "Show command line options")                             \
    ("__positional", po::value<std::vector<std::string> >(),          \
      "Positional arguments");                                        \
  po::positional_options_description pos;                             \
  pos.add("__positional", -1);                                        \
  po::parsed_options res = p.options(options)                         \
    .allow_unregistered()                                             \
    .positional(pos)                                                  \
    .run();                                                           \
  po::store(res, vm);                                                 \
  /* Invoke notify callbacks*/                                        \
  po::notify(vm);                                                     \
  if (vm.count("help"))                                               \
    std::cout << options << std::endl;                                \
  std::vector<std::string> args                                       \
    = po::collect_unrecognized(res.options, po::include_positional);  \
  /* Keep --help for next option parser*/                             \
  if (vm.count("help"))                                               \
    args.push_back("--help");                                         \
  /* Set arguments to what was not used */                            \
  ::qi::Application::setArguments(args);                              \
}                                                                     \
QI_AT_ENTER(boost::bind(&_qi_## __LINE__## opt_func))


#endif
