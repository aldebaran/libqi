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
  // Regular:
    Config();
    ~Config();

  // Config:
    /**
     * Set default value of standAlone
     * if defaultUrl is set this function will throw.
     * If --qi-standalone is set the session will automatically be standalone.
     */
    QI_API_DEPRECATED_MSG(Use 'setStandAlone' instead)
    Config& setDefaultStandAlone(bool standAlone);

    QI_API_DEPRECATED_MSG(Use 'standAlone' instead)
    bool defaultStandAlone() const;

    Config& setStandalone(bool standalone);

    bool standalone() const;

    Config& setOption(ApplicationSession::Option opt);
    ApplicationSession::Option option() const;

    /**
     * Set default url to connect to
     * If --qi-url is set the session will connect on the provided url.
     */
    QI_API_DEPRECATED_MSG(Use 'setConnectUrl' instead)
    Config& setDefaultUrl(const Url& connectUrl);
    QI_API_DEPRECATED_MSG(Use 'connectUrl' instead)
    const Url& defaultUrl() const;

    Config& setConnectUrl(Url connectUrl);

    const boost::optional<Url>& connectUrl() const;

    /**
     * Set default url to listen to
     * If --qi-listen-url is set the session will listen on the provided url.
     */
    QI_API_DEPRECATED_MSG(Use 'setListenUrls' or 'addListenUrl' instead)
    Config& setDefaultListenUrl(const Url& listenUrl);
    QI_API_DEPRECATED_MSG(Use 'listenUrls' instead)
    const Url& defaultListenUrl() const;

    Config& addListenUrl(Url listenUrl);

    /// @note Any previous listen URLs that were added in this configuration are discarded.
    Config& setListenUrls(std::vector<Url> listenUrls);

    const std::vector<Url>& listenUrls() const;

    Config& setSessionConfig(SessionConfig sessConfig);
    const SessionConfig& sessionConfig() const;

    Config& setName(const std::string& name);
    const std::string& name() const;

  private:
    bool _standalone;
    Option _opt;
    SessionConfig _sessionConfig;
    std::string _name;
  };

  /**
   * @brief Constructors of the class.
   *
   * The session owned by the object can have two different modes: either standard or standalone.
   *
   * To make an application session standalone, you need to specify the "--qi-standalone" option on
   * the command line arguments or set the option in the configuration passed at the application
   * construction.
   *
   * A standard session must connect to a service directory, and therefore needs a URL to connect
   * to. This URL is chosen from available values (the ones that have been specified) according to
   * the following order:
   *   - First the URL that was specified as  "--qi-url <url>" in the command line arguments.
   *   - Then the URL specified in the "QI_URL" environment variable.
   *   - Finally the URL passed directly or as a member of the configuration to the constructor.
   *
   * A standalone session cannot connect to a service directory but must instead listen on some
   * URLs. A standard session can also listen, but will only do if the "--qi-listen-url" is given in
   * the command line arguments or if a service is registered on it. In both cases, the URLs to
   * listen on are chosen from available values (the ones that have been specified) according to the
   * following order:
   *   - First the URLs that were specified as  "--qi-listen-url <urls>" in the command line
   *    arguments.
   *   - Then the URLs specified in the "QI_LISTEN_URL" environment variable.
   *   - Then the URLs in the configuration passed to the constructor.
   *   - Finally the hardcoded default listen URL.
   *
   * Both the "--qi-listen-url" command line argument and the "QI_LISTEN_URL" environment variable
   * follow the same format "url1;url2;...", allowing the user to specify multiple values.
   *
   *  @param argc The number of arguments.
   *  @param argv The array containing all the arguments given to the program.
   *  @param opt Either ApplicationSession::Option_None or
   *  ApplicationSession::Option_NoAutoExit. The default behavior of
   *  ApplicationSession is to call stop() once the session gets disconnected.
   *  @see qi::ApplicationSession::Option
   *  @param defaultUrl The default url used if no --qi-url was found in the command line arguments
   * and no suitable QI_URL environment variable was defined.
   */
  ApplicationSession(int& argc,
                     char**& argv,
                     int opt = 0,
                     const Url& defaultUrl = SessionConfig::defaultConnectUrl());
  ApplicationSession(const std::string& name,
                     int& argc,
                     char**& argv,
                     int opt = 0,
                     const Url& defaultUrl = SessionConfig::defaultConnectUrl());
  ApplicationSession(int& argc, char**& argv, const Config& defaultConfig);
  virtual ~ApplicationSession();

  /**
   * @return The embedded session used by ApplicationSession.
   */
  SessionPtr session() const;

  const Config& config() const;

  /**
   *  @return The URL used by ApplicationSession to connect to a service directory.
   *  @see qi::ApplicationSession::ApplicationSession for parsing information.
   */
  Url url() const;

  /**
   *  @return The first URL used by ApplicationSession to listen on.
   *  @see qi::ApplicationSession::ApplicationSession for parsing information.
   */
  Url listenUrl() const;

  /**
   *  @return All the URLs used by ApplicationSession to listen on.
   *  @see qi::ApplicationSession::ApplicationSession for parsing information.
   */
  std::vector<Url> allListenUrl() const;

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

  /**
   * Returns whether the ApplicationSession runs in standalone mode.
   */
  bool standAlone();

  /**
   *  @return A concatenation of the ApplicationSession's --help with the Application's --help texts.
   */
  std::string helpText() const;

private:
  std::unique_ptr<ApplicationSessionPrivate> _p;
};

using ApplicationSessionOptions = ApplicationSession::Option;
}

#endif // QIMESSAGING_APPLICATIONSESSION_HPP_
