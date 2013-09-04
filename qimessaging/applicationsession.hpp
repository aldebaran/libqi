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
  class ApplicationSessionPrivate;
  /** By default, ApplicationSession will automatically will automatically
   *  call Application::stop() when the session is over.
   *  If you want a different behaviour you have to call the constructor with
   *  the desired option below.
   *
   *  Ex: qi::ApplicationSession app(argc, argv, qi::ApplicationSession_NoAutoExit)
   */
  enum Options
  {
    ApplicationSession_None          = 0,
    ApplicationSession_NoAutoExit    = 1,
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
     *  if not it will take the url in the constructor instead setting its url.
     *  If --qi-listen-url is set the session will listen on the provided url.
     */
    ApplicationSession(int& argc, char**& argv, ApplicationSessionOptions opt = ApplicationSession_None, const Url& url = "tcp://127.0.0.1:9559");
    ApplicationSession(const std::string& name, int& argc, char**& argv, ApplicationSessionOptions opt = ApplicationSession_None, const Url& url = "tcp://127.0.0.1:9559");
    virtual ~ApplicationSession();

    Session&   session();

    /** Returns the intern url used by ApplicationSession parsed on the
     *  command line by --qi-url or given in the constructor.
     */
    const Url& url();

    /** Returns the intern url used by ApplicationSession parsed on the
     *  command line by --qi-listen-url.
     */
    const Url& listenUrl();

    /** Establishes the session's connection and moreover starts listening if
     * --qi-listen-url was given.
     */
    void       start();

    /** Runs the application and automatically calls start() if it hasn't been done yet.
     */
    void       run();
  private:
    ApplicationSessionPrivate* _p;
  private:
    QI_DISALLOW_COPY_AND_ASSIGN(ApplicationSession);
  };
}

#endif  // QIMESSAGING_APPLICATIONSESSION_HPP_
