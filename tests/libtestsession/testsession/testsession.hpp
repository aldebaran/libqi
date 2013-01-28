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
 * \brief Provide usefull methods to manipulate libtestsession different modes.
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
#include <qimessaging/session.hpp>

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
    Mode_SSL,
    Mode_Default
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
  qi::Session* session();

private:
  TestSessionPrivate *_p;
};

extern TestMode::Mode testMode;
#endif // !_TESTS_LIBTESTSESSION_TESTSESSION_HPP_
