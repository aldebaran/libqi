/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

/*!
 * \class TestSessionPair
 * \brief Simpliest way to provide pair of qi::Session with different network settings.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSessionPair::TestSessionPair()
 * \brief TestSessionPair constructor. Allocate and initialize two qi::Session, one in client mode and the other in server mode.
 *        Test setting used depends on environment variable.
 * \throw TestSessionError on failure.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSessionPair::TestSessionPair(TestSession::Mode mode)
 * \brief TestSessionPair constructor. Allocate and initialize two qi::Session, one in client mode and the other in server mode.
 *        Test settings used depends on given mode.
 * \throw TestSessionError on failure.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSessionPair::TestSessionPair(TestSessionPair &other)
 * \brief TestSessionPair copy constructor. Allocate and initialize two qi::Session, one in client mode and the other in server mode.
 *        Use qi::ServiceDirectory of given TestSessionPair, both pair are therefore connected on the same service directory.
 *        Test setting used depends on environment variable.
 * \throw TestSessionError on failure.
 * \see qi::Session
 * \see qi::ServiceDirectory
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSessionPair::client()
 * \brief Getter for client session of pair.
 * \return Pointer to qi::Session.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSessionPair::client()
 * \brief Getter for server session of pair.
 * \return Pointer to qi::Session.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_TESTSESSIONPAIR_HPP_
#define _TESTS_LIBTESTSESSION_TESTSESSIONPAIR_HPP_

#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsession.hpp>

class TestSessionPair
{
public:
  TestSessionPair(TestMode::Mode mode = TestMode::Mode_Default,
                  const std::string url = "tcp://0.0.0.0:0");
  TestSessionPair(TestSessionPair &other);
  ~TestSessionPair();

public:
  qi::Session* client() const;
  qi::Session* server() const;
  std::vector<qi::Url> serviceDirectoryEndpoints() const;

private:
  TestMode::Mode       _mode;
  TestSession         *_client;
  TestSession         *_server;
  qi::ServiceDirectory _sd;
};

#endif // !_TESTS_LIBTESTSESSION_TESTSESSIONPAIR_HPP_
