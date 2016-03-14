#pragma once
/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef QI_MESSAGING_APPLICATIONSESSION_HPP_
#define QI_MESSAGING_APPLICATIONSESSION_HPP_
#include <boost/noncopyable.hpp>
#include <qi/application.hpp>
#include <qi/session.hpp>

namespace qi
{
class ApplicationSessionPrivate;

/**
 * \class qi::ApplicationSession
 * \includename{qi/applicationsession.hpp}
 *  ApplicationSession is an application with an embedded session.
 *  The constructor has to be the first method called of the class to initialize the class.
 *  Be careful with the scope of the object, once the destructor is called,
 *  the session is destroyed as well.
 */
class QI_API ApplicationSession : public Application, private boost::noncopyable
{
public:
  /** By default, ApplicationSession will automatically call qi::Application::stop()
   *  when the session is over. If you want a different behaviour you have to
   *  call the constructor with the desired option below.
   *
   *  Ex: qi::ApplicationSession app(argc, argv, qi::ApplicationSession::Option_NoAutoExit)
   */
  enum Option
  {
    Option_None = 0,       //!< No option, this is the default behavior.
    Option_NoAutoExit = 1, //!< With this option the application won't stop once the session is disconnected.
  };

  /**
   * This class is used to set configuration of the ApplicationSession
   */
  class QI_API Config
  {
  public:
    Config();
    ~Config();

    /**
     * Set default value of standAlone
     * if defaultUrl is set this function will throw.
     * If --qi-standalone is set the session will automatically be standalone.
     */
    Config& setDefaultStandAlone(bool standAlone);
    bool defaultStandAlone() const;

    Config& setOption(ApplicationSession::Option opt);
    ApplicationSession::Option option() const;

    /**
     * Set default url to connect to
     * If --qi-url is set the session will connect on the provided url.
     */
    Config& setDefaultUrl(const Url& url);
    const Url& defaultUrl() const;

    /**
     * Set default url to listen to
     * If --qi-listen-url is set the session will listen on the provided url.
     */
    Config& setDefaultListenUrl(const Url& listenUrl);
    const Url& defaultListenUrl() const;

    Config& setName(const std::string& name);
    const std::string& name() const;

  private:
    qi::uint32_t _stateMachine;
    Option _opt;
    Url _url;
    Url _listenUrl;
    std::string _name;
  };

  /** ApplicationSession will check first if there is a --qi-url given in argv,
   *  if not it will take the url in the constructor instead setting its url.
   *  If --qi-listen-url is set the session will listen on the provided url.
   *  @param argc The number of arguments.
   *  @param argv The array containing all the arguments given to the program.
   *  @param opt Either ApplicationSession::Option_None or
   *  ApplicationSession::Option_NoAutoExit. The default behavior of
   *  ApplicationSession is to call stop() once the session gets disconnected.
   *  @see qi::ApplicationSession::Option
   *  @param url The default url used if no --qi-url was found in the options.
   *  @param listenUrl The default url used if no --qi-listen-url was found in the options.
   */
  ApplicationSession(int& argc, char**& argv, int opt = 0, const Url& url = "tcp://127.0.0.1:9559");
  ApplicationSession(const std::string& name,
                     int& argc,
                     char**& argv,
                     int opt = 0,
                     const Url& url = "tcp://127.0.0.1:9559");
  ApplicationSession(int& argc, char**& argv, const Config& defaultConfig);
  virtual ~ApplicationSession();

  /**
   * @return The embedded session used by ApplicationSession.
   */
  SessionPtr session();

  /**
   *  @return The url used by ApplicationSession parsed on the command line by
   *  --qi-url if specified, otherwise the default url given in the constructor.
   *  @see qi::ApplicationSession::ApplicationSession for parsing information.
   */
  Url url();

  /**
   *  @return The url used by ApplicationSession parsed on the command line by
   *  --qi-listen-url, otherwise the default url given in the constructor.
   *  @see qi::ApplicationSession::ApplicationSession for parsing information.
   */
  Url listenUrl();

  /**
   * Establishes the session's connection and moreover starts listening if
   * --qi-listen-url was given.
   * @deprecated since 2.5, use startSession instead
   */
  QI_API_DEPRECATED_MSG(Use 'startSession' instead)
  void start();

  /**
   * Establishes the session's connection and moreover starts listening if
   * --qi-listen-url was given.
   */
  void startSession();

  /** Runs the application and automatically calls start() if it hasn't been done yet.
   */
  void run();

private:
  ApplicationSessionPrivate* _p;
};

using ApplicationSessionOptions = ApplicationSession::Option;
}

#endif // QIMESSAGING_APPLICATIONSESSION_HPP_
