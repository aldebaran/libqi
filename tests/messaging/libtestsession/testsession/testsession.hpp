/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

/*!
 * \class TestSession
 * \brief Enhance QiMessaging unit test.
 *  Base class of Testsession Library. Provide a working session with network connections midified by environment variable
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSession::TestSession(const std::string &serviceDirectoryUrl, bool listen, TestMode::Mode mode = TestMode::getTestMode())
 * \brief TestSession constructor, initialize internal session and connect it to service directory. Internal qi::Session will listen
 * if boolean 'listen' is set to true.
 * \throw TestSessionError if ENVIRON_VARIABLE value is not supported.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSession::session()
 * \brief Return a valid session.
 *        Behavior of this function is modified with ENVIRON_VARIABLE environment variable.
 * \return Valid qi::Session.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \class TestMode
 * \brief Provide useful methods to manipulate libtestsession different modes.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \enum Mode
 * \brief Main enum used to set and manipulate test modes in tests.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestMode::getTestMode
 * \brief Return activated test mode for libtestsession.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestMode::initTestMode(int argc, char **argv)
 * \brief Parse command line arguments to find the '--mode' option and set it to environment.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestMode::forceTestMode(TestMode::Mode mode)
 * \brief Force libtestsession test mode to value.
 *  Value is set to environment.
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_TESTSESSION_HPP_
#define _TESTS_LIBTESTSESSION_TESTSESSION_HPP_

#include <stdexcept>
#include <memory>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <qi/session.hpp>
#include <qi/messaging/gateway.hpp>
#include <ka/typetraits.hpp>

#define ENVIRON_VARIABLE "TESTMODE"

class TestSessionPrivate;

class TestSessionError : public std::runtime_error
{
public:
  TestSessionError(const std::string& what_arg);
  ~TestSessionError() throw();
};

class TestMode
{
public:
  enum Mode
  {
    Mode_Direct,
    Mode_SD,
    Mode_Gateway,
    Mode_ReverseGateway,
    Mode_RemoteGateway,
    Mode_Random,
    Mode_Nightmare,
    Mode_NetworkMap,
    Mode_SSL
  };

public:
  static TestMode::Mode getTestMode();
  static void           initTestMode(int argc, char **argv);
  static void           forceTestMode(TestMode::Mode mode);

private:
  static void           help();
};

class TestSession
{
private:
  TestSession();

public:
  TestSession(const std::string &serviceDirectoryUrl, bool listen, TestMode::Mode mode = TestMode::getTestMode());
  ~TestSession();

public:
  qi::SessionPtr session();

private:
  std::unique_ptr<TestSessionPrivate> _p;
};

extern TestMode::Mode testMode;

namespace test
{
  inline std::string adaptScheme(std::string url, TestMode::Mode mode = TestMode::getTestMode())
  {
    if (mode == TestMode::Mode_SSL)
    {
      static const boost::string_ref tcpScheme{ "tcp://" };
      if (boost::starts_with(url, tcpScheme))
      {
        url.replace(0, tcpScheme.size(), "tcps://");
      }
    }
    return url;
  }

  inline qi::Url defaultListenUrl()
  {
    return "tcp://127.0.0.1:0";
  }

  inline qi::Url url(const qi::Session& sess)
  {
    const auto& endpoints = sess.endpoints();
    if (endpoints.empty())
      throw std::runtime_error("Session has no endpoint to connect to");
    return endpoints.front();
  }

  inline qi::Url url(const qi::Gateway& gw)
  {
    const auto& endpoints = gw.endpoints();
    if (endpoints.empty())
      throw std::runtime_error("Session has no endpoint to connect to");
    return endpoints.front();
  }
}

#endif // !_TESTS_LIBTESTSESSION_TESTSESSION_HPP_
