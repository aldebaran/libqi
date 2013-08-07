#pragma once
/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef QIMESSAGING_APPLICATIONSESSION_HPP_
#define QIMESSAGING_APPLICATIONSESSION_HPP_
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

namespace qi
{
  /** By default, ApplicationSession will automatically connect the session and
   *  will automatically call Application::stop() when the session is over.
   *  If you want a different behaviour you have to call the constructor with
   *  the desired options below.
   *
   *  Ex : qi::ApplicationSession app(argc, argv, qi::ApplicationSession_NoAutoConnect | qi::ApplicationSession_NoAutoExit)
   */
  enum Options
  {
    ApplicationSession_None          = 0,
    ApplicationSession_NoAutoConnect = 1,
    ApplicationSession_NoAutoExit    = 1 << 1,
  };
  typedef qi::uint32_t ApplicationSessionOptions;

  /** ApplicationSession is an application with an embedded session.
   *  The constructor has to be the first method called of the class to initialize the class.
   *  Be careful with the scope of the object, once the destructor is called,
   *  the session is destroyed as well.
   */
  class QIMESSAGING_API ApplicationSession : public Application
  {
  public:
    /** ApplicationSession will check first if there is a --qi-url given in argv,
     *  if not it will take the url in the constructor instead.
     */
    ApplicationSession(int& argc, char**& argv, ApplicationSessionOptions opt = ApplicationSession_None, const Url& url = "tcp://127.0.0.1:9559");
    ApplicationSession(const std::string& name, int& argc, char**& argv, ApplicationSessionOptions opt = ApplicationSession_None, const Url& url = "tcp://127.0.0.1:9559");
    virtual ~ApplicationSession();

    static Session& session();
  private:
    QI_DISALLOW_COPY_AND_ASSIGN(ApplicationSession);
  };
}

#endif  // QIMESSAGING_APPLICATIONSESSION_HPP_
