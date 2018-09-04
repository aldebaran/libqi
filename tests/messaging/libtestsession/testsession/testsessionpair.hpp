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
 * \fn TestSessionPair::client()
 * \brief Getter for client session of pair.
 * \return Pointer to qi::Session.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \fn TestSessionPair::server()
 * \brief Getter for server session of pair.
 * \return Pointer to qi::Session.
 * \see qi::Session
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_TESTSESSIONPAIR_HPP_
#define _TESTS_LIBTESTSESSION_TESTSESSIONPAIR_HPP_

#include <qi/session.hpp>
#include <testsession/testsession.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/os.hpp>

const auto serviceWaitDefaultTimeout = qi::Seconds{ 1 };

class TestSessionPair
{
public:
  struct ShareServiceDirectory_tag {};
  static const ShareServiceDirectory_tag ShareServiceDirectory;

  /*!
   * Constructs a session pair that will instanciate a service directory and possibly a gateway if
   * mode is Mode_Gateway. The server and the client will connect
   * to the endpoint that was created (the service directory or the gateway if it was instanciated).
   * \note The newly created session will not instanciate a service directory or a gateway of its
   * own.
   */
  TestSessionPair(TestMode::Mode mode = TestMode::getTestMode(), std::string sdUrl = "tcp://0.0.0.0:0");

  /*!
   * Constructs a session pair that will connect the server and the client to the service directory (or
   * the gateway, depending on mode) of the other session pair, so that both sessions share the same.
   * \note The newly created session will not instanciate a service directory or a gateway of its own.
   */
  TestSessionPair(ShareServiceDirectory_tag,
                  const TestSessionPair& other,
                  TestMode::Mode mode = TestMode::getTestMode());

  /*!
   * Constructs a session pair that will connect the server and the client to sdEndpoint.
   * \note The newly created session will not instanciate a service directory or a gateway of its own.
   */
  TestSessionPair(const qi::Url& sdEndpoint, TestMode::Mode mode = TestMode::getTestMode());

  ~TestSessionPair() = default;

public:
  qi::SessionPtr client() const;
  qi::SessionPtr server() const;
  qi::SessionPtr sd() const;
  const qi::Gateway& gateway() const;
  std::vector<qi::Url> serviceDirectoryEndpoints() const;
  std::vector<qi::Url> gatewayEndpoints() const;
  TestMode::Mode mode() const
  {
    return _mode;
  }

  qi::Url endpointToServiceSource() const
  {
    switch (_mode)
    {
    case(TestMode::Mode_Gateway):
      return gatewayEndpoints().at(0);
    case(TestMode::Mode_SD):
    case(TestMode::Mode_SSL):
    case(TestMode::Mode_Direct):
      return serviceDirectoryEndpoints().at(0);

    default:
      throw std::runtime_error("Unmanaged Mode: " + qi::os::to_string(static_cast<int>(_mode)));
    }

  }

private:
  TestMode::Mode _mode;
  qi::SessionPtr _sd;
  std::unique_ptr<qi::Gateway> _gw;
  std::unique_ptr<TestSession> _server;
  std::unique_ptr<TestSession> _client;
};

#endif // !_TESTS_LIBTESTSESSION_TESTSESSIONPAIR_HPP_
