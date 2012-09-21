/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef LIBQI_APPLICATION_HPP_
#define LIBQI_APPLICATION_HPP_

#include <qi/config.hpp>
#include <boost/function.hpp>

namespace qi {

  class QI_API Application
  {
  public:
    Application(int argc, char** argv);
    ~Application();

    void run();

    static bool initialized();
    static int argc();
    static const char** argv();
    static const char* program();

    static bool atEnter(boost::function<void()> func);

    static bool atExit(boost::function<void()> func);
  };
}

#define QI_AT_ENTER(func) \
static bool _qi_ ## __LINE__ ## atenter = ::qi::Application::atEnter(func);

#define QI_AT_EXIT(func) \
static bool _qi_ ## __LINE__ ## atenter = ::qi::Application::atExit(func);

#endif
